# Third-party notices

Traka includes Tracktion Engine (Copyright Tracktion Software Corporation) and JUCE (Copyright Raw Material Software Limited). The complete corresponding upstream trees, copyright notices and license files remain available in the pinned submodules. Application code is AGPL-3.0-or-later; engine code retains GPL-3.0-or-later. See `LICENSE` and `vendor/tracktion_engine/LICENSE.md`, `vendor/tracktion_engine/modules/juce/LICENSE.md`.

Tracktion's dependency inventory is in its README. Included libraries include rpmalloc (public domain), ConcurrentQueue (BSD/BSL), CHOC (ISC), crill (BSL), expected (CC0), libsamplerate (BSD), MPMCQueue (MIT), magic_enum (MIT), farbot (MIT), doctest (MIT), signalsmith-stretch (MIT), SoundTouch (LGPL-2.1-or-later), and Airwindows (MIT). Preserve source headers and bundled notices when redistributing.

JUCE's license file lists its embedded dependencies and license paths, including FLAC, Ogg Vorbis, JPEG, PNG, zlib, HarfBuzz, SheenBidi and SDK headers. Traka does not enable external plugin hosting, web browser or curl.

This file is an index, not a replacement for the license texts and copyright notices in the corresponding source trees. A distribution must include the corresponding source and applicable notices; the complete source archive includes these exact source trees. The app also bundles full AGPL/GPL/LGPL texts and the collected upstream notices under `Contents/Resources/OpenSource/`.
