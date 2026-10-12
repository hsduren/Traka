# Upstream

- Tracktion Engine: https://github.com/Tracktion/tracktion_engine
  - Commit: `e42d82f06e553771477eabd1dd0d561fe2cbe573`
  - License: GPL-3.0-or-later / commercial dual license; this project uses the open-source path.
- JUCE (upstream submodule): https://github.com/juce-framework/JUCE
  - Commit: `37c894f83d379179b2070d437ccd0f1cd9af9576`
  - License: AGPLv3 / commercial dual license; this project uses the open-source path.

No upstream source modifications. The local JUCE submodule URL uses HTTPS instead of upstream's SSH URL. Tracktion is linked as JUCE modules. Traka's UI, session management and demo generator are authored here. Upstream examples serve as API references; none of their audio assets are included in the application.

| P0 capability | Implementation |
|---|---|
| Multitrack playback / audio import | Tracktion Edit, AudioTrack and WaveAudioClip |
| Waveforms | JUCE AudioThumbnail, independent timeline |
| Recording / input routing | Tracktion input devices and TransportControl |
| Move / trim / split / delete / undo | Tracktion clip and UndoManager APIs |
| Volume / pan / mute / solo | Tracktion track and VolumeAndPanPlugin |
| Save / reopen / portable media | Tracktion EditFileOperations with Traka media copying |
| WAV export | Tracktion EditRenderer, 48 kHz 24-bit stereo |

All other upstream capabilities are outside the explicitly narrowed P0 scope.
