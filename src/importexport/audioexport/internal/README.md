# Audio export: exporting all parts in one render pass

This directory's `AbstractAudioWriter` (and its per-format subclasses —
`WaveWriter`, `Mp3Writer`, `OggWriter`, `FlacWriter`, `AacWriter`) normally
export one notation (score or part) at a time via `write()`, each of which
drives one full offline render of the audio engine
(`IPlayback::saveSoundTrack`, see `muse/framework/audio/engine/internal/export/README.md`
for the engine side).

For "Export parts" with several parts and no full score in the selection,
that means one full render per part — for a large orchestral score, most of
that render time is spent re-computing the same mix over and over, once per
instrument. `writeParts()` / `IPlayback::saveSoundTracks` instead render the
graph **once** and capture every requested part's stem from that single
pass.

## The three pieces

- **`INotationWriter::supportsBatchPartExport()` / `writeParts()`**
  (`src/project/inotationwriter.h`) — an opt-in capability on top of the
  existing `write()`/`writeList()` interface. Default is unsupported
  (`writeParts()` returns `NotSupported`), so every other writer (image
  export, PDF, MusicXML, MIDI, …) is untouched. `AbstractAudioWriter` is
  currently the only implementer, so effectively only the audio writers use
  this path.

- **`AbstractAudioWriter::writeParts()`** (this directory) — given the
  *master* notation and a list of `{part notation, destination device}`
  pairs, resolves each part's `engraving::InstrumentTrackId`s (via
  `Part::instrumentTrackIdList()`) to the master mixer's `TrackId`s (via
  `IPlaybackController::instrumentTrackIdMap()`, which is always built
  from the master score — see the map's own doc comment). This is why the
  export must go through the *master* notation, not each part's own excerpt:
  the mixer only has tracks for the master. Part excerpts still work as the
  lookup key because `Excerpt::createExcerpt` copies each part's id/instrument
  from the master when the excerpt is created, so a part's own
  `InstrumentTrackId`s are always present in the master's map too. Each
  resolved `{TrackId, device}` pair becomes one `muse::audio::SoundTrackTarget`,
  and the whole batch is handed to the engine as one call:
  `IPlayback::saveSoundTracks(format, targets)`.

  If a part's tracks can't be resolved (e.g. an empty or muted part with no
  live mixer track), that part is skipped with a logged error rather than
  failing the whole batch — mirroring how the old per-notation loop would
  independently succeed or fail per file.

- **`ExportProjectScenario::exportPartsInOnePass()`**
  (`src/project/internal/exportprojectscenario.cpp`) — decides *when* to take
  this path and does the file bookkeeping. It's used only when all of these
  hold (see `useBatchPartExport` where `PER_PART` unit type is handled):
  - unit type is `PER_PART` (one file per part; not a combined score or a
    per-page format),
  - the writer opts in (`supportsBatchPartExport()`),
  - there's more than one notation to export, and
  - none of the selected notations is the main/full score.

  The full-score exclusion matters: `writeParts()` maps *parts* to master
  mixer tracks, but the main score's own "part" is the whole score — mixing
  that in would need every track, defeating the point, and the main score
  already has its own single-file `write()` path. When the full score is
  part of the selection, the writer falls back to the original per-notation
  loop unchanged, same as when the writer doesn't opt in at all.

  Otherwise, it opens one output file per part up front (skipping any the
  user declines to overwrite, same prompt as the per-notation path), builds
  the `PartExportTargetList`, and makes a single `writer->writeParts()` call
  instead of looping `write()` once per notation. This is also why
  `fileCount`/progress is set to `1` "file" for this path — it's one export
  operation with one progress cycle, not N.

## What did *not* change

- Single-part export, full-score export, and every non-audio format still go
  through the original `write()`/`writeList()` path exactly as before.
- `AbstractAudioWriter::write()` (single notation) is untouched; `writeParts()`
  is new code alongside it, sharing only the small helpers
  (`soundTrackFormat()`, leading/trailing silence option parsing) that were
  factored out so both paths build the same `SoundTrackFormat`.

## Known tradeoff carried over from the engine side

Exported stems from this path don't include shared reverb/aux sends that a
part would have in the full mix, or when soloed via the mixer panel — see
the "Known tradeoff, not a bug" section of the muse_framework README linked
above. This is a property of where the engine captures each stem, not of
anything in this directory.
