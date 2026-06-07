# Piano Keyboard v2 — Implementation Status

## Status: ✅ COMPLETE — Builds and passes 746 tests

## v2 Architecture
`PlaybackModel::activeNotesAtTimestamp(micros)` → `std::lower_bound` on `originEvents`
Replaces v1's `rebuildNoteTimeline()` + `m_noteTimeline` DOM-traversal cache.

## Changes
| File | Change |
|------|--------|
| `src/engraving/playback/playbackmodel.h` | Removed `NoteTimelineEvent`, `m_noteTimeline`. Added `ActiveNoteInfo`, `activeNotesAtTimestamp()` |
| `src/engraving/playback/playbackmodel.cpp` | Implemented `activeNotesAtTimestamp()` with `std::lower_bound` + `std::get_if<NoteEvent>` scan |
| `src/notation/inotationplayback.h` | Added `ActiveNoteInfo`, `activeNotesAtTimestamp()`, `collectPlaybackTracks()` |
| `src/notation/internal/notationplayback.h` | Added overrides for both new methods |
| `src/notation/internal/notationplayback.cpp` | Delegates both to `m_playbackModel` |
| `src/notationscene/.../pianokeyboardcontroller.cpp` | Uses `pos*1e6` → `activeNotesAtTimestamp` (was `secToPlayedTick` → `activeNotesAtTick`) |
| `src/notationscene/.../pianokeyboardview.cpp` | Per-instrument coloring via `INSTRUMENT_PALETTE`, unchanged from v1 |

## Per-instrument Colors
`playingKeyColor(key)` blends up to 10 palette colors. Works identically in v1 and v2 — the controller `setPlayingInstruments()` / query path is shared.

## Key Formula
```cpp
// pitch_level_t → MIDI note (empirical, in activeNotesAtTimestamp)
int midi = static_cast<int>(std::round(nominalPitchLevel / 50.0)) + 12;
```
