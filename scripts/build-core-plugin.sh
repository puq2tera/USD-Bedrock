#!/bin/bash
# Rebuild the Core Bedrock plugin inside the Multipass VM and restart Bedrock.
# Intended to be run from the host machine.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

VM_NAME="bedrock-starter"

print_header "Rebuilding Core plugin in VM (${VM_NAME})"

# Ensure Multipass is available on the host
require_multipass

# Make sure the VM is running
info "Ensuring VM '${VM_NAME}' is running..."
if ! multipass info "${VM_NAME}" &>/dev/null; then
    error "VM '${VM_NAME}' not found. Run ./scripts/launch.sh first."
    exit 1
fi
multipass start "${VM_NAME}" >/dev/null 2>&1 || true

info "Building Core plugin and restarting Bedrock inside the VM..."

# Run the build inside the VM as root so we can write to /opt/bedrock
multipass exec "${VM_NAME}" -- sudo bash << 'EOF'
set -euo pipefail

export BEDROCK_DIR="/opt/bedrock/Bedrock"
CORE_DIR="/opt/bedrock/server/core"
BUILD_DIR="${CORE_DIR}/.build"

mkdir -p "${BUILD_DIR}"

# Always generate build files into ${BUILD_DIR}, even if CMake was previously
# invoked from a different directory.
cmake -S "${CORE_DIR}" -B "${BUILD_DIR}" -G Ninja

cd "${BUILD_DIR}"
ninja -j "$(nproc)"

systemctl restart bedrock
EOF

success "Core plugin rebuilt and Bedrock restarted."


