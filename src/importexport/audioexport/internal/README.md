# Audio export: exporting all parts at the same time

This directory's `AbstractAudioWriter` (and its per-format subclasses —
`WaveWriter`, `Mp3Writer`, `OggWriter`, `FlacWriter`, `AacWriter`) normally
export one notation (score or part) at a time via `write()`, each of which
drives one full offline render of the audio engine
(`IPlayback::saveSoundTrack`, see `muse/framework/audio/engine/internal/export/README.md`
for the engine side).

For "Export parts" with several parts, that means one full render per part,
one after another. `writeParts()` / `IPlayback::saveSoundTracks` instead hand
all parts to the engine at once, and the engine renders them at the same time
on several threads (one file per part). Each file contains the same audio as
the per-part export: the part's own tracks plus their aux sends (e.g. reverb).

## The "Multi-stem render" option

`IAudioExportConfiguration::multiStemRender()` (setting
`export/audio/multiStemRender`, on by default) is the checkbox of the same name
on the audio format pages of the export dialog
(`MultiStemRenderSettings.qml`, which also has the "Idle instruments until
their first note" option). When it's off,
`AbstractAudioWriter::supportsBatchPartExport()` returns false and every part
is exported the original way, one after another. It's there as a fallback in
case a plugin doesn't behave the same when the engine copies it (each render
thread gets its own copy of the aux effects).

## The three pieces

- **`INotationWriter::supportsBatchPartExport()` / `writeParts()`**
  (`src/project/inotationwriter.h`) — an opt-in capability on top of the
  existing `write()`/`writeList()` interface. Default is unsupported
  (`writeParts()` returns `NotSupported`), so every other writer (image
  export, PDF, MusicXML, MIDI, …) is untouched. `AbstractAudioWriter` is
  currently the only implementer.

- **`AbstractAudioWriter::writeParts()`** (this directory) — given the
  *master* notation and a list of `{part notation, destination device}`
  pairs, resolves each part's `engraving::InstrumentTrackId`s (via
  `Part::instrumentTrackIdList()`) to the master mixer's `TrackId`s (via
  `IPlaybackController::instrumentTrackIdMap()`, which is always built
  from the master score). This is why the export goes through the *master*
  notation: the mixer only has tracks for the master. Part excerpts still work
  as the lookup key because `Excerpt::createExcerpt` copies each part's
  id/instrument from the master, so a part's `InstrumentTrackId`s are always
  present in the master's map too.

  Each part becomes **one** `muse::audio::SoundTrackTarget`: one destination
  file with all of the part's tracks (several when the part has instrument
  changes). The whole batch goes to the engine in one call:
  `IPlayback::saveSoundTracks(format, targets)`.

  If a part's tracks can't be resolved, that part is skipped with a logged
  error rather than failing the whole batch — mirroring how the per-part
  loop would independently succeed or fail per file.

- **`ExportProjectScenario::exportPartsInOnePass()`**
  (`src/project/internal/exportprojectscenario.cpp`) — decides *when* to take
  this path and does the file bookkeeping. It's used only when all of these
  hold (see `useBatchPartExport`):
  - unit type is `PER_PART` (one file per part),
  - the writer opts in (`supportsBatchPartExport()`, i.e. the option is on),
  - there's more than one notation to export.

  The full score and parts that share an instrument (e.g. a combined
  percussion part plus the individual ones) take this path too: the engine
  renders every track only once and mixes the files that share tracks from
  the rendered output (see the engine README).

  Otherwise the original per-notation loop runs unchanged.

  On this path it asks about replacing existing files first (skipping any the
  user declines, same prompt as the per-notation path; if all are skipped
  there's nothing to do), then opens one output file per part, builds the
  `PartExportTargetList`, and makes a single `writer->writeParts()` call. If
  that fails, the incomplete files are removed and the user can retry, as in
  the per-notation path; a cancelled export removes them as well. This is also
  why `fileCount`/progress is set to `1` "file" for this path — it's one
  export operation with one progress cycle, not N (the per-file progress is
  shown in its own dialog).

## What did *not* change

- Exporting a single notation (one part, or only the full score), and every
  non-audio format, still go through the original `write()`/`writeList()` path
  exactly as before.
- `AbstractAudioWriter::write()` (single notation) is untouched; `writeParts()`
  is new code alongside it, sharing only the small helpers
  (`soundTrackFormat()`, leading/trailing silence option parsing) so both paths
  build the same `SoundTrackFormat`.
