#!/usr/bin/env bash
# Start Metro for Expo dev client on a fixed port (8081) so Xcode / Simulator
# always matches the packager. Run this BEFORE pressing Run in Xcode.
#
# Usage: from repo root: ./scripts/start-dev-client-metro.sh
#    or: bash scripts/start-dev-client-metro.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CLIENT_DIR="${REPO_ROOT}/client"
METRO_PORT="${METRO_PORT:-8081}"

if [[ ! -d "${CLIENT_DIR}" ]] || [[ ! -f "${CLIENT_DIR}/package.json" ]]; then
  echo "error: expected Expo app at ${CLIENT_DIR}" >&2
  exit 1
fi

# Prefer Homebrew Node 20 (Expo/Metro is flaky on very new Node versions).
if [[ -x "/opt/homebrew/opt/node@20/bin/node" ]]; then
  export PATH="/opt/homebrew/opt/node@20/bin:${PATH}"
elif [[ -x "/usr/local/opt/node@20/bin/node" ]]; then
  export PATH="/usr/local/opt/node@20/bin:${PATH}"
fi

cd "${CLIENT_DIR}"

# Some shells/CI set CI=true, which makes Metro run without file watching.
unset CI

echo "Metro will listen on http://localhost:${METRO_PORT}"
echo "Wait until this says it's ready, then Run (⌘R) in Xcode."
echo ""

exec npx expo start --dev-client --port "${METRO_PORT}"
