# Release policy

Traka's application source is AGPL-3.0-or-later. Dependencies retain their own licenses.

Every distributed application version must have a matching public source commit/tag and complete source archive, including its pinned Tracktion Engine and JUCE source trees, license notices and build scripts. Do not publish an application update first and postpone its corresponding source update.

For a new version:

1. Change the single version in `CMakeLists.txt` and update release notes.
2. Run `python3 scripts/collect_notices.py` if dependency revisions change. Preserve upstream notices and recheck dependency licenses.
3. Build with `./scripts/build.sh`; validate inside the macOS 2 VM with `./scripts/test-vm.sh macos2`.
4. Review the publishable diff for secrets, personal files and unrelated source. Commit it, then tag that exact commit `v<version>`.
5. Run `python3 scripts/package_source.py` from a clean checkout. It includes all three pinned repository trees and writes `SOURCE_MANIFEST.json` inside the source archive.
6. Upload the complete source archive to the matching GitHub Release. If distributing a binary, link its corresponding source archive on the same download page and keep the bundled license/source information current.
7. Verify the public tag and archive are accessible. Retain previous tags and source archives so older installed versions can obtain their corresponding source.

`build/` and `release/` are local outputs, excluded from Git. Signing keys, provisioning files, credentials, microphone recordings and user projects must never be committed. App Store distribution is a separate unresolved release path; this source release does not submit to any store.

## v0.1.0 — 2026-10-12

First P0 source release: audio import, multitrack playback, single-track recording implementation, basic move/trim/split/delete with undo/redo, track volume/pan/mute/solo, local project persistence with copied media, stereo 48 kHz / 24-bit WAV export, English/Simplified Chinese UI and bundled open-source notices. The recording implementation has not been tested against a real microphone; the VM has no input channels.

## v0.1.1 — 2026-10-18

Adds the user-approved front-facing silver mixing-console app icon, integrated as an Icon Composer `.icon` source and compiled into `Assets.car` plus `AppIcon.icns`. Requires Xcode 26+ to build. User authorized host-side launch and icon checks for this update. Application audio behavior is unchanged. License display now paginates the full text to avoid long layout stalls discovered during host validation.

## v0.1.2 — 2026-10-18

Replaces the detailed console icon with the user-approved simplified Dock preview: three thick charcoal tracks, smooth oversized caps, one orange center cap and two off-white caps on a full-bleed light panel. Removes metal grain, grip grooves, fine divider lines and glow to improve small-size legibility. Uses the same Icon Composer and ICNS integration; application behavior is unchanged.
