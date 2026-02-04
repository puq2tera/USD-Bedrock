#!/bin/bash
# This script configures the VM with all the packages it needs, builds bedrock and the core plugin, and sets up the systemd services.
# It should be run from the VM.

set -e

# Source common utilities
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

print_header "Bedrock Starter Setup Script"

# Check if running as root
check_root

# Get the project directory
PROJECT_DIR=$(get_project_dir)
BEDROCK_DIR="$PROJECT_DIR/Bedrock"
INSTALL_DIR="/opt/bedrock"
DATA_DIR="/var/lib/bedrock"

success "Project directory: $PROJECT_DIR"
success "Install directory: $INSTALL_DIR"

# Update package lists
info "[1/10] Updating package lists..."
apt-get update

# Install apt-fast for faster package downloads
info "[2/10] Installing apt-fast..."
if ! command -v apt-fast &> /dev/null; then
    apt-get install -y software-properties-common
    add-apt-repository ppa:apt-fast/stable -y
    apt-get update
    echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
    echo 'apt-fast apt-fast/maxdownloads string 10' | debconf-set-selections
    echo 'apt-fast apt-fast/dlflag boolean true' | debconf-set-selections
    echo 'apt-fast apt-fast/aptmanager string apt-get' | debconf-set-selections
    apt-get install -y apt-fast
else
    success "apt-fast already installed"
fi

# Install Bedrock dependencies
info "[3/10] Installing Bedrock dependencies..."
chmod +x "${PROJECT_DIR}/scripts/"*.sh 2>/dev/null || true
bash "${PROJECT_DIR}/scripts/install-cpp-deps.sh" --vm

# Set up Clang as default compiler
info "[4/10] Configuring Clang compiler..."
update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 || true
update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100 || true

# Configure ccache
info "[5/10] Configuring ccache..."
ccache --set-config=max_size=2G || true
ccache --set-config=compression=true || true
ccache --set-config=cache_dir=/var/cache/ccache || true
mkdir -p /var/cache/ccache
chmod 777 /var/cache/ccache

# Set up ccache wrapper symlinks
mkdir -p /usr/lib/ccache
ln -sf /usr/bin/ccache /usr/lib/ccache/clang || true
ln -sf /usr/bin/ccache /usr/lib/ccache/clang++ || true
ln -sf /usr/bin/ccache /usr/lib/ccache/gcc || true
ln -sf /usr/bin/ccache /usr/lib/ccache/g++ || true

# Install PHP and nginx
info "[6/10] Installing PHP 8.4 and nginx..."
add-apt-repository ppa:ondrej/php -y
apt-get update
apt-fast install -y \
    php8.4-fpm \
    php8.4-cli \
    php8.4-curl \
    php8.4-mbstring \
    php8.4-apcu \
    php8.4-zip \
    nginx \
    curl \
    unzip \
    p7zip-full \
    7zip

# Restart PHP-FPM (if present) and verify zip extension
info "Verifying PHP zip support..."
systemctl restart php8.4-fpm || true
if php -m | grep -qi '^zip$'; then
    success "✓ PHP zip extension enabled"
else
    info "PHP zip extension not listed in php -m; checking phpinfo..."
    php -i | grep -qi 'zip support' && success "✓ PHP zip support detected" || { echo "ERROR: PHP zip extension missing"; exit 1; }
fi

# Install Composer
info "[7/10] Installing Composer..."
if ! command -v composer &> /dev/null; then
    curl -sS https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer
else
    success "Composer already installed"
fi

# Clone/build Bedrock
info "[8/10] Building Bedrock..."
if [[ ! -d "$BEDROCK_DIR" ]]; then
    error "Bedrock directory not found at $BEDROCK_DIR"
    warn "Please ensure Bedrock is cloned as a git submodule:"
    echo "  git submodule update --init --recursive"
    exit 1
fi

cd "$BEDROCK_DIR"
export CC=clang
export CXX=clang++
export PATH="/usr/lib/ccache:$PATH"
make clean || true
make bedrock --jobs "$(nproc)"

# Verify sqlite3 CLI tool (used for manual maintenance tasks like VACUUM)
info "Verifying sqlite3 CLI tool..."
if command -v sqlite3 &> /dev/null; then
    SQLITE_VERSION=$(sqlite3 --version 2>/dev/null | awk '{print $1}' || echo "")
    success "✓ sqlite3 CLI available (version ${SQLITE_VERSION})"
else
    error "✗ sqlite3 not found even after installation. Please install sqlite3 manually and re-run setup."
    exit 1
fi

# Create installation directory structure
info "[9/10] Setting up installation directories..."
mkdir -p "$INSTALL_DIR"
cp -r "$BEDROCK_DIR" "$INSTALL_DIR/"
cp -r "$PROJECT_DIR/server" "$INSTALL_DIR/"

# Ensure Bedrock binary exists at the systemd ExecStart path and is executable
# (Fixes systemd status=203/EXEC when the binary is missing or not +x)
info "Verifying Bedrock binary..."
BEDROCK_BIN_SRC="$(find "$BEDROCK_DIR" -maxdepth 4 -type f -name bedrock | head -n1 || true)"
if [[ -z "$BEDROCK_BIN_SRC" ]]; then
  error "Could not find built 'bedrock' executable under $BEDROCK_DIR. Build may have failed."
  exit 1
fi
install -m 0755 "$BEDROCK_BIN_SRC" "$INSTALL_DIR/Bedrock/bedrock"


# Build Core plugin
info "[10/10] Building Core plugin..."
export BEDROCK_DIR="$INSTALL_DIR/Bedrock"
mkdir -p "$INSTALL_DIR/server/core/.build"
cd "$INSTALL_DIR/server/core/.build"
cmake -G Ninja ..
ninja -j "$(nproc)"

# Create bedrock user
info "Creating bedrock user..."
if ! id "bedrock" &>/dev/null; then
    useradd -r -s /bin/false -d /opt/bedrock bedrock
fi

# Set ownership
chown -R bedrock:bedrock "$INSTALL_DIR"
chown -R bedrock:bedrock /var/cache/ccache

# Create data directory
mkdir -p "$DATA_DIR"
chown bedrock:bedrock "$DATA_DIR"
chmod 755 "$DATA_DIR"

# Install systemd service
info "Installing systemd service..."
cp "$PROJECT_DIR/server/config/bedrock.service" /etc/systemd/system/
systemctl daemon-reload
systemctl enable bedrock.service

# Configure nginx
info "Configuring nginx..."
cp "$PROJECT_DIR/server/config/nginx.conf" /etc/nginx/sites-available/bedrock-api
ln -sf /etc/nginx/sites-available/bedrock-api /etc/nginx/sites-enabled/bedrock-api
rm -f /etc/nginx/sites-enabled/default
systemctl enable nginx.service

# Install PHP dependencies (run as bedrock user to avoid root-owned vendor files)
info "Installing PHP dependencies via Composer..."
API_DIR="$INSTALL_DIR/server/api"

# Ensure bedrock user owns the API directory (especially vendor/) before running composer
chown -R bedrock:bedrock "$API_DIR"

sudo -u bedrock -H bash -lc "
  set -e
  export COMPOSER_HOME=/tmp/composer
  mkdir -p \"$COMPOSER_HOME\"
  cd \"$API_DIR\"
  composer install --no-interaction --prefer-dist --no-dev --optimize-autoloader
"

# Permissions: keep code owned by bedrock; allow webserver group access; ensure runtime dirs writable
chgrp -R www-data "$API_DIR" || true
find "$API_DIR" -type d -exec chmod 2755 {} \;
find "$API_DIR" -type f -exec chmod 0644 {} \;
for d in "$API_DIR/storage" "$API_DIR/bootstrap/cache"; do
  if [[ -d "$d" ]]; then
    chown -R bedrock:www-data "$d"
    chmod -R 2775 "$d"
  fi
done

echo
success "=========================================="
success "Setup complete!"
success "=========================================="
echo
echo "To start services:"
echo "  sudo systemctl start bedrock"
echo "  sudo systemctl start php8.4-fpm"
echo "  sudo systemctl start nginx"
echo
echo "To check status:"
echo "  sudo systemctl status bedrock"
echo "  sudo systemctl status php8.4-fpm"
echo "  sudo systemctl status nginx"
echo
echo "To view logs:"
echo "  sudo journalctl -u bedrock -f"
echo "  sudo tail -f /var/log/nginx/api_error.log"
echo

