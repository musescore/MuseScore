# Export selection — pull-request package

## Proposed title

Add rehearsal-oriented audio export for score selections

## Delivery structure

This change crosses the `muse_framework` submodule boundary and should be
reviewed as two linked pull requests:

1. **musescore/muse_framework** — add bounded soundtrack save options, RPC
   serialization, sample-aligned range seeking, fades, and focused unit tests.
2. **musescore/MuseScore** — consume the framework revision and add selection
   capture, transient tempo/mix/metronome handling, commands, UI, and project
   tests/mocks.

The linked review proposals are
[muse_framework #292](https://github.com/musescore/muse_framework/pull/292) and
[MuseScore #34920](https://github.com/musescore/MuseScore/pull/34920). The
MuseScore proposal depends on the framework change. Once the framework PR is
merged, update the MuseScore submodule pointer to the upstream commit before
merge.

The existing feature request is
[MuseScore #19345](https://github.com/musescore/MuseScore/issues/19345);
do not open a duplicate issue. Earlier attempts
[#24369](https://github.com/musescore/MuseScore/pull/24369) and
[#32564](https://github.com/musescore/MuseScore/pull/32564) were closed
unmerged. The first used a MuseSampler offline seek workaround that a
maintainer considered unsuitable; the second was closed when the framework
changed substantially and the feature needed reimplementation. This proposal
uses the current `muse_framework` audio export interfaces and does not patch
`MuseSamplerWrapper` or discard offline-rendered samples to seek. A
nonzero-range export with MuseSounds is a mandatory targeted regression test
before claiming that the earlier sampler problem is solved.

Do not commit local fixtures or generated exports named `TestExportacio*`.

## Draft pull-request description

### Summary

This change adds **Export selection…** for contiguous score range selections.
It renders the range directly from the original score instead of creating a
temporary score, preserving prior tempo, dynamics, articulations, playback
techniques, instrument changes, effects, and the active sound profile.

The command is available from the File menu and the range-selection context
menu. The compact export dialog supports:

- the existing audio formats and encoder options;
- tempo from 10% to 300%;
- selected-instrument and background-instrument gain controls;
- muting all background instruments;
- optional pre-selection fade-in and post-selection fade-out;
- metronome off or audible throughout the exported fragment.

All rehearsal settings are transient. Export completion, cancellation, and
failure restore normal playback state and do not dirty the score.

### Implementation

- Register the command through the current project command registry and state
  provider, including the File and range-selection context menus. The legacy
  `ProjectUiActions` registration removed upstream is not retained. The command
  state follows the active notation when switching between the score and parts.
- Adapt MuseScore's enum contract declarations and three percussion command
  call sites to the new contract API in the framework submodule. Upstream
  MuseScore `main` still uses the older API because it pins an earlier
  framework revision; these compatibility edits can be dropped if upstream
  updates them first.
- Implement the newer framework shortcut-resolver registration method in
  MuseScore's resolver, preserving existing scope priorities and titles.
- Snapshot the raw selection ticks and selected part IDs before export.
- Apply the temporary tempo multiplier before mapping raw score ticks onto the
  unfolded playback timeline.
- Pass a validated `SoundTrackSaveOptions` range through the audio RPC layer.
- Seek the existing playback graph to the range start and render only the
  requested duration.
- Align the start down to the preceding audio-sample boundary so events exactly
  on the first selected beat are not skipped.
- Apply fade ramps to the final offline mix before encoding.
- Apply per-part gains relative to the existing mixer values whenever track
  controls are built. This also preserves the export-only mix when automation
  envelopes are resent after a tempo change or a track finishes loading.
  Restore all tempo, metronome, solo/mute, and volume state afterwards.

### Validation

- RelWithDebInfo build, link, and install on Windows x64 with Qt 6.10.2 and
  MSVC 2022: **PASS** on the November 2026 upstream integration; built and
  installed executable hashes match. User audio smoke test is pending.
- Manual range, tempo, focused/background mix, fade, metronome, MP3, WAV, OGG,
  and post-export state-restoration checks: see
  `docs/export-selection-test-plan.md`.
- `SoundTrackSaveOptions` validation and RPC round-trip tests are included in
  the framework change.
- A playback-controller regression test covers an export-muted instrument when
  a tempo change resends its volume automation. It and the other 19 controller
  tests passed on the refreshed local test build.
- The November 2026 upstream integration has passed conflict, code-style,
  full application build, and install checks locally. The audio and playback
  test targets compile and link. User playback checks are still pending.
- The framework audio test executable exits before GoogleTest output with a
  local Windows `0xC0000005`, as in earlier local runs. Its test cases are not
  claimed as passed here.
- MuseScore CI `run_tests` passed, including `muse_audio_tests`. The local
  Windows runner did not report results, so no local test pass is claimed for
  that earlier revision. The upstream refresh requires a new local build.

### Scope and known limitations

- The initial command requires a contiguous range selection.
- The vertical selection resolves focused instruments/parts, not individual
  voices within one staff.
- A one-measure count-in is deferred. The prototype used the real-time
  interactive click stream, which could race the offline renderer. Count-in
  should later be transported atomically as part of the offline save request.
- Complex repeated passages remain explicit validation coverage before claiming
  complete repeat-scenario support.
- **FAIL (STATE-03):** Canceling an in-progress MP3 export leaves a playable
  partial file on disk. Cancellation no longer crashes and restores playback
  settings.
- **FAIL (UI-02):** Keyboard focus order in the dialog is unintuitive; Tab
  reaches only the Export button and audio format directly.
- **FAIL (UI-03):** Mouse control of the sliders works, but Up/Down on an
  individual instrument percentage changes it by 5% and then moves focus to
  another control. Accessible-name verification is still pending.

### Localization

All new strings use `qsTrc` or `TranslatableString`. The English source
catalogue is generated by the normal `lupdate` workflow, and localized
catalogues remain managed through Transifex. Translation seeds for Spanish,
Catalan, German, Italian, and French are in
`docs/export-selection-i18n.md`.

## Evidence to attach

- Screenshot of a compact dialog with one focused instrument and background
  instruments muted.
- Screenshot with two focused instruments, background level at 50%, and the
  background-instrument section expanded.
- One waveform showing exact range boundaries and independent fades.
- One waveform showing a synchronized first-beat metronome click.
- The completed result summary from `docs/export-selection-test-plan.md`.

## Submission checklist

- [x] Link the existing MuseScore feature request #19345 and describe the
      rehearsal and teaching use case without creating a duplicate.
- [x] Confirm both repositories include upstream `main` as fetched on
      2026-11-13.
- [x] Run the official Uncrustify check on touched C++ files: 14/14 framework
      and 43/43 application sources pass.
- [x] Run `git diff --check` in both repositories.
- [x] Build and install the complete application on Windows x64.
- [ ] Execute all high-priority cases in the test plan.
- [ ] Run translation extraction and placeholder-language UI checks.
- [x] Commit the framework change on its own branch and open its PR first.
- [ ] Commit MuseScore application changes separately, including the resulting
      framework submodule pointer.
- [x] Exclude `TestExportacio*`, build directories, logs, and generated audio.
- [ ] Sign the MuseScore contributor license agreement when prompted.
- [x] Link both PRs and the feature issue in each description.

## Recommended commit grouping

### muse_framework

```text
Add ranged soundtrack export options
```

Keep the audio value type, RPC transport, engine seeking, fade processing, and
their tests in one reviewable framework commit.

### MuseScore

```text
Add audio export for score selections
```

Keep command registration, selection capture, transient playback controls,
dialog UI, mocks/tests, documentation, and the framework submodule update in
one application commit unless maintainers request a split.
