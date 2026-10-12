#!/bin/bash
# Run Traka's own UI/component workflow inside the user's macOS 2 VM.
# No host app launch, host pointer input, or global screenshot capture.
set -euo pipefail
cd "$(dirname "$0")/.."
vm_host="${1:-macos2}"
ssh -o BatchMode=yes "$vm_host" 'mkdir -p ~/Applications/Traka-P0-QA'
scp -r build/Traka_artefacts/Debug/Traka.app "$vm_host":Applications/Traka-P0-QA/
ssh "$vm_host" '~/Applications/Traka-P0-QA/Traka.app/Contents/MacOS/Traka --self-test > ~/Applications/Traka-P0-QA/audio-test.log 2>&1; result=$?; cat ~/Applications/Traka-P0-QA/audio-test.log; exit "$result"'
ssh "$vm_host" '~/Applications/Traka-P0-QA/Traka.app/Contents/MacOS/Traka --ui-smoke > ~/Applications/Traka-P0-QA/ui-runtime.log 2>&1; result=$?; cat ~/Documents/Traka-P0-QA/ui-smoke.txt; cat ~/Applications/Traka-P0-QA/ui-runtime.log; exit "$result"'
mkdir -p docs/qa
scp "$vm_host":Documents/Traka-P0-QA/studio-final.png docs/qa/studio-vm.png
scp "$vm_host":Documents/Traka-P0-QA/ui-smoke.txt docs/qa/ui-smoke.txt
scp "$vm_host":Applications/Traka-P0-QA/audio-test.log docs/qa/audio-test.log

scp "$vm_host":Documents/Traka-P0-QA/license-vm.png docs/qa/license-vm.png
