# Export selection to audio

Status: feature-complete working implementation; final regression and PR preparation in progress

## Goal

Export a rehearsal audio file directly from a range selection in the original
score. The export must preserve musical context computed from the master score,
including tempo changes, dynamics, articulations, instrument changes, repeats,
playback automation, effects, and the current sound profile.

This feature must not create a temporary score and must not persist temporary
playback or mixer settings in the project.

## Product decisions

- The feature initially accepts a contiguous range selection.
- The horizontal extent defines the exported time range.
- The vertical extent defines the focused parts.
- Existing audio formats and encoding settings are reused.
- Focus levels are relative to the existing mix, rather than replacements for
  the score's mixer levels.
- Tentative defaults:
  - one focused part: focused part at 100%, other parts muted;
  - two or more focused parts: focused parts at 100%, other parts at 50%.
- The user can change the background level, including setting it to 0%.
- Tempo is expressed as a percentage of the score tempo.
- Fade-in and fade-out add bounded musical context before and after the selected
  range and apply gain ramps in the final offline mix.
- Metronome export can be off or audible throughout the exported fragment.
- Enabling the metronome disables the pre-selection fade-in; fade-out remains
  available.

## Why Save selection is not the implementation basis

`Save selection` creates another score whose musical state begins at the copied
range. Prior dynamics, tempo information, playback techniques, and other context
may therefore be absent. Audio export should instead render the original
notation's existing playback data and seek to the selected start position.

## Existing architecture

The relevant path is:

1. `ExportDialogModel` collects export settings.
2. `ExportProjectScenario` resolves an `INotationWriter` and calls it.
3. `AbstractAudioWriter` prepares the notation and calls
   `IPlayback::saveSoundTrack()`.
4. `AudioContext` resets the player to time zero.
5. `SoundTrackWriter` renders the mixer's full configured duration.

Useful existing capabilities:

- `INotationSelectionRange` already exposes start/end ticks and selected parts.
- `PlaybackController` already maps range selections to instrument track IDs.
- `INotationPlayback` already maps raw score ticks to unfolded playback ticks
  and seconds.
- `PlaybackController` already supports a transient tempo multiplier.
- The playback graph already contains evaluated context from the original score.

The current audio-export path explicitly disables range-playback muting while
`m_isExportingAudio` is true. Selection export should be an explicit export mode,
not a removal of this guard, so normal full-score export remains unchanged.

## Proposed implementation

### 1. Represent an offline render range

Introduce an audio-engine save-options value distinct from `SoundTrackFormat`.
The format describes encoding; the save options describe which portion of the
playback graph to render.

The initial value needs only:

```cpp
struct SoundTrackSaveOptions {
    bool hasTimeRange = false;
    secs_t startTime = 0.0;
    secs_t endTime = 0.0;
    secs_t fadeInDuration = 0.0;
    secs_t fadeOutDuration = 0.0;
};
```

The values are seconds because the framework audio engine is notation-agnostic.
When `hasTimeRange` is false, behavior must remain compatible with full export.

The value must be propagated through the audio RPC boundary and covered by
packing tests.

### 2. Render from the requested position

For a valid range `[startTime, endTime)`:

- stop the offline player;
- seek all track sources to `startTime`;
- render `endTime - startTime` seconds;
- reset the engine to the same post-export state used by full export.

Seeking the existing synthesizers is important: their event streams already
contain the dynamics, articulations, instrument setup, and automation computed
from the full score.

### 3. Resolve the score selection

Snapshot the following before opening or closing asynchronous UI:

- raw start tick;
- raw end tick;
- selected part IDs.

After applying the requested tempo multiplier, convert the raw ticks through
`INotationPlayback::playPositionTickByRawTick()` and `playedTickToSec()`. This
order matters because changing the tempo multiplier changes the time mapping.

The first slice should reject an empty or non-range selection with a clear error.

### 4. Add a selection-export mode

Add an `Export selection…` command enabled for a range selection. It should reuse
the existing export dialog in selection mode rather than duplicate format and
encoder settings.

For the first slice, selection mode may expose only audio formats. The normal
`Export…` command and full-score behavior remain unchanged.

The selection dialog uses a compact layout: tempo and metronome share one row;
fade-in and fade-out share another; global background mute precedes the
background-level slider; and format, sample rate/sample format, and bit rate
share a final settings row when the selected codec exposes them. Individual
instrument controls retain bounded internal scrolling.

### 5. Apply rehearsal mix overrides

Selected part IDs map to the instrument track IDs already maintained by
`PlaybackController`. A percentage is a gain applied on top of each track's
current mixer/automation value:

- 100% = no change;
- 50% = approximately -6.02 dB;
- 0% = mute.

Overrides must be scoped to one export and restored on success, cancellation,
or error. They must not call persistent `ProjectAudioSettings` setters unless the
original values are unconditionally restored and no project-dirty notification
can escape. Prefer transient engine control parameters or an export-only gain
stage.

### 6. Metronome and fades

The metronome uses the score's native main-stream click events and is enabled
transiently only for the offline export. The export start is aligned down to the
preceding audio-sample boundary so an event exactly on the selected first beat
cannot be skipped by time-to-sample rounding.

Fades are applied in `SoundTrackWriter` immediately before encoding. Their
durations are clamped to available musical context. The dialog treats
metronome and pre-selection fade-in as mutually exclusive, preventing the
first click from being attenuated.

A one-measure count-in was prototyped but deferred from the initial PR. The
native count-in uses the interactive off-stream: the real-time driver can
consume a click before the offline operation owns the graph. A future version
should transport count-in events into the atomic offline save operation rather
than reuse the interactive event stream.

## Repeats and boundaries

The raw-to-played tick conversion uses the expanded repeat timeline. Initial
tests must cover a range outside repeats and a range within one repeated region.
Ranges that are visually contiguous but occur more than once in unfolded
playback need an explicit behavior decision before claiming full repeat support.

The end boundary is exclusive. Reverb and release tails need a later explicit
policy; the first slice should render the exact selected duration so its behavior
is deterministic.

## Delivery slices

1. Engine range contract and tests.
2. Writer propagation and a programmatic range-export test.
3. `Export selection…` action and minimal audio-only UI.
4. Tempo percentage with unconditional state restoration.
5. Focus/background mix controls.
6. Metronome, fades, release-tail policy, and extended repeat tests.

Each slice should leave full-score export unchanged and independently testable.

## Acceptance criteria for the first working slice

- A range starting after the first measure exports without earlier music.
- Export stops at the exclusive end of the selection.
- A prior dynamic marking still affects notes at the selection start.
- Full-score export still starts at zero and keeps its previous duration.
- WAV and at least one compressed format use the same selected duration.
- Cancel and error paths leave playback usable and do not dirty the score.

## Test strategy

- Unit tests for save-options validation and RPC serialization.
- Audio-engine tests verifying seek position and requested frame count.
- Playback conversion tests for tempo multiplier and simple repeats.
- Project-level tests for action availability and option propagation.
- A small manual score containing a prior dynamic, a tempo change, multiple
  instruments, an instrument change, and a repeat.

Avoid waveform golden files where possible. Prefer deterministic frame counts,
event boundaries, state restoration, and non-silent/silent interval assertions.
