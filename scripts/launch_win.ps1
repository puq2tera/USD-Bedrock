<# 
Bedrock Starter - Multipass Launcher (Windows PowerShell)

Usage:
  - Save as scripts\launch.ps1 in the repo (same place as launch.sh).
  - Run from a PowerShell prompt:
      cd <repo-root>
      .\scripts\launch.ps1
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# -----------------------------
# Utility functions
# -----------------------------

function Write-Header($msg) {
    Write-Host ""
    Write-Host "==============================================" -ForegroundColor Cyan
    Write-Host $msg -ForegroundColor Cyan
    Write-Host "==============================================" -ForegroundColor Cyan
    Write-Host ""
}
function Write-Info($msg)  { Write-Host "[INFO ] $msg"  -ForegroundColor White  }
function Write-Warn($msg)  { Write-Host "[WARN ] $msg"  -ForegroundColor Yellow }
function Write-Ok($msg)    { Write-Host "[ OK  ] $msg"  -ForegroundColor Green  }
function Write-Err($msg)   { Write-Host "[FAIL] $msg"   -ForegroundColor Red    }

function Require-Multipass {
    Write-Info "Checking for Multipass..."
    try {
        $null = multipass --version 2>$null
    } catch {
        Write-Err "Multipass not found. Install Multipass for Windows and ensure it is in PATH."
        throw
    }
    Write-Ok "Multipass found."
}

function Get-ProjectDir {
    # When running as a script, $PSScriptRoot is the directory of this .ps1
    if ($null -ne $PSScriptRoot -and $PSScriptRoot -ne "") {
        $scriptDir = $PSScriptRoot
    } else {
        # Fallback for weird hosts / dot-sourcing:
        $scriptPath = $MyInvocation.MyCommand.Definition
        $scriptDir  = Split-Path -Path $scriptPath -Parent
    }

    # Project root is the parent of the scripts\ directory
    $projectDir = Split-Path -Path $scriptDir -Parent
    return (Resolve-Path $projectDir).Path
}



# -----------------------------
# Main
# -----------------------------

$VM_NAME = "bedrock-starter"
$PROJECT_DIR = Get-ProjectDir

Write-Header "Bedrock Starter - Multipass Launcher (Windows)"

# 1) Ensure Bedrock submodule exists
$bedrockPath = Join-Path $PROJECT_DIR "Bedrock"
if (-not (Test-Path $bedrockPath)) {
    Write-Warn "Bedrock submodule not found. Initializing..."
    Push-Location $PROJECT_DIR
    try {
        git submodule update --init --recursive
    } catch {
        Write-Err "Failed to initialize Bedrock submodule."
        Write-Warn "Please run: git submodule update --init --recursive"
        throw
    } finally {
        Pop-Location
    }
}

# 2) Generate .clangd from .clangd.example if needed
$clangdPath        = Join-Path $PROJECT_DIR ".clangd"
$clangdTemplate    = Join-Path $PROJECT_DIR ".clangd.example"

if (-not (Test-Path $clangdPath) -and (Test-Path $clangdTemplate)) {
    Write-Info "Generating .clangd from template..."

    $FMT_INCLUDE = ""

    # Windows: typical vcpkg location
    if (Test-Path "C:\vcpkg\installed\x64-windows\include\fmt") {
        $FMT_INCLUDE = "C:/vcpkg/installed/x64-windows/include/fmt"
    }

    if ([string]::IsNullOrWhiteSpace($FMT_INCLUDE)) {
        Write-Warn "fmt library not found in standard locations; IDE may not find fmt headers."
    }

    $content = Get-Content -LiteralPath $clangdTemplate -Raw
    $content = $content.Replace("{{PROJECT_ROOT}}", $PROJECT_DIR)
    $content = $content.Replace("{{FMT_INCLUDE}}",  $FMT_INCLUDE)

    Set-Content -LiteralPath $clangdPath -Value $content -Encoding UTF8

    if (-not [string]::IsNullOrWhiteSpace($FMT_INCLUDE)) {
        Write-Ok "Created .clangd with project-specific paths (fmt: $FMT_INCLUDE)"
    } else {
        Write-Ok "Created .clangd (fmt paths will be resolved from compile_commands.json)"
    }
}

# 3) Check Multipass
Require-Multipass

# 4) Detect architecture (mainly informational; Multipass auto-selects image arch)
$arch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture
$image = "24.04"   # Ubuntu 24.04 LTS

if ($arch -eq "Arm64") {
    Write-Ok "Detected ARM64 architecture - Multipass will use ARM Ubuntu (native performance)."
} else {
    Write-Ok "Detected $arch architecture - Multipass will use x86_64 Ubuntu."
}

# Windows: no special network args by default (Default Switch works fine)
$networkArgs = @()

# 5) Check if VM exists
$vmExists = $false
try {
    $list = multipass list
    if ($list -match ("^" + [regex]::Escape($VM_NAME) + "\s")) {
        $vmExists = $true
    }
} catch {
    # multipass list already validated above; just propagate
    throw
}

if ($vmExists) {
    Write-Warn "VM '$VM_NAME' already exists."
    $reply = Read-Host "Delete and recreate? (y/n)"
    if ($reply -match '^[Yy]') {
        Write-Info "Deleting existing VM..."
        multipass delete $VM_NAME --purge 2>$null | Out-Null
    } else {
        Write-Ok "Using existing VM."
        Write-Ok "To start it: multipass start $VM_NAME"
        Write-Ok "To shell into it: multipass shell $VM_NAME"
        return
    }
}

# 6) Launch VM with cloud-init
Write-Info "Launching Ubuntu VM (this may take a few minutes)..."

$launchArgs = @(
    "launch",
    "--name", $VM_NAME,
    "--memory", "4G",
    "--cpus", "4",
    "--disk", "20G",
    "--cloud-init", (Join-Path $PROJECT_DIR "multipass.yaml"),
    "--timeout", "600",
    $image
)

# Insert any network args before image if present
if ($networkArgs.Count -gt 0) {
    # rebuild args with network inserted before image
    $launchArgs = @(
        "launch",
        "--name", $VM_NAME,
        "--memory", "4G",
        "--cpus", "4",
        "--disk", "20G",
        "--cloud-init", (Join-Path $PROJECT_DIR "multipass.yaml"),
        "--timeout", "600"
    ) + $networkArgs + @($image)
}

try {
    multipass @launchArgs
} catch {
    Write-Err "Failed to launch VM."
    throw
}

# 7) Wait for cloud-init
Write-Info "Waiting for cloud-init to complete..."
multipass exec $VM_NAME -- cloud-init status --wait | Out-Null
Start-Sleep -Seconds 5

# 8) Set as primary if no primary exists
$primaryName = ""
try {
    $primaryName = multipass get client.primary-name 2>$null
} catch {
    $primaryName = ""
}

if ([string]::IsNullOrWhiteSpace($primaryName) -or
    $primaryName -eq "None" -or
    $primaryName -eq "primary") {

    Write-Info "Setting $VM_NAME as primary instance..."
    try {
        multipass set "client.primary-name=$VM_NAME"
        Write-Ok "You can now use 'multipass shell' without specifying the VM name."
    } catch {
        Write-Err "Failed to set primary instance. Run 'multipass set client.primary-name=$VM_NAME' manually to investigate."
        throw
    }
}

# 9) Install multipass-sshfs in VM and mount project
$projectMount = "/home/ubuntu/Expensify_windows"   # canonical mount in this VM
$legacyMount  = "/bedrock-starter"                 # keep old path working via symlink

Write-Info "Installing multipass-sshfs inside the VM (required for mounts)..."
try {
    multipass exec $VM_NAME -- sudo snap install multipass-sshfs
    Write-Ok "multipass-sshfs installed."
} catch {
    Write-Warn "snap install failed, attempting snap refresh..."
    multipass exec $VM_NAME -- sudo snap refresh multipass-sshfs
    Write-Ok "multipass-sshfs refreshed."
}

Write-Info "Mounting project directory for real-time sync..."
$hostPath    = (Resolve-Path $PROJECT_DIR).Path
$mountTarget = "${VM_NAME}:${projectMount}"

# clear stale mounts (ignore failures)
multipass umount "${VM_NAME}:${legacyMount}"  2>$null | Out-Null
multipass umount "${VM_NAME}:${projectMount}" 2>$null | Out-Null

# ensure target exists
multipass exec $VM_NAME -- sudo mkdir -p $projectMount | Out-Null
multipass exec $VM_NAME -- sudo chown ubuntu:ubuntu $projectMount | Out-Null

# mount
multipass mount $hostPath $mountTarget
Write-Ok "Mount successful - files will sync in real-time."

# provide legacy path expected by older docs/scripts
multipass exec $VM_NAME -- sudo rm -rf $legacyMount | Out-Null
multipass exec $VM_NAME -- sudo ln -s $projectMount $legacyMount | Out-Null
Write-Ok "Legacy path available at $legacyMount -> $projectMount"

# 10) Run setup script inside VM
Write-Info "Running setup script (this will take several minutes)..."
Write-Info "Monitor progress in another terminal with:"
Write-Info "  multipass exec $VM_NAME -- tail -f /var/log/cloud-init-output.log"

multipass exec $VM_NAME -- sudo bash "$projectMount/scripts/setup.sh"

# 11) Show VM info and service status
Write-Info "Gathering VM IP address(es)..."
$vmInfo  = multipass info $VM_NAME
$vmIPs   = ($vmInfo | Select-String "IPv4" | ForEach-Object {
    ($_ -split ":",2)[1].Trim()
}) -join " "

Write-Ok "VM IP address(es): $vmIPs"

Write-Info "Checking service status..."
Start-Sleep -Seconds 10

$bedrockActive = $false
$phpActive     = $false
$nginxActive   = $false

try {
    multipass exec $VM_NAME -- systemctl is-active bedrock | Select-String "^active" -Quiet | Out-Null
    $bedrockActive = $True
} catch {}

try {
    multipass exec $VM_NAME -- systemctl is-active php8.4-fpm | Select-String "^active" -Quiet | Out-Null
    $phpActive = $True
} catch {}

try {
    multipass exec $VM_NAME -- systemctl is-active nginx | Select-String "^active" -Quiet | Out-Null
    $nginxActive = $True
} catch {}

if ($bedrockActive) { Write-Ok "Bedrock service is running." } else { Write-Warn "Bedrock service may still be starting..." }
if ($phpActive)     { Write-Ok "PHP-FPM service is running." } else { Write-Warn "PHP-FPM service may still be starting..." }
if ($nginxActive)   { Write-Ok "Nginx service is running."   } else { Write-Warn "Nginx service may still be starting..." }

Write-Host ""
Write-Ok "=========================================="
Write-Ok "Setup Complete!"
Write-Ok "=========================================="
Write-Host ""

Write-Info "Quick Start:"
Write-Host "  multipass shell $VM_NAME"
Write-Host "  multipass exec $VM_NAME -- systemctl status bedrock"
Write-Host ""

Write-Info "Access Services (from Windows host):"
Write-Host "  API:     curl.exe http://<VM_IP>/api/status"
Write-Host "  Bedrock: nc <VM_IP> 8888 (or equivalent client)"
Write-Host ""

Write-Info "Optional Port Forwarding:"
Write-Host "  multipass port-forward $VM_NAME 8888:8888  # Bedrock"
Write-Host "  multipass port-forward $VM_NAME 80:8080   # API (host:guest)"
Write-Host ""

Write-Info "Development:"
Write-Host "  Project is mounted at /bedrock-starter in the VM."
Write-Host "  After editing C++ code, rebuild:"
Write-Host "    ./scripts/build-core-plugin.sh"
Write-Host ""

Write-Info "Service Management:"
Write-Host "  multipass exec $VM_NAME -- sudo systemctl restart bedrock"
Write-Host "  multipass exec $VM_NAME -- sudo systemctl restart nginx"
Write-Host ""

Write-Info "View Logs:"
Write-Host "  multipass exec $VM_NAME -- sudo journalctl -u bedrock -f"
Write-Host ""
