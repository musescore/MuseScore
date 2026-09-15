# Export selection to audio — validation plan and execution record

Status: active validation document for pull-request evidence

## Purpose

This document defines the functional, regression, usability, and resilience
tests for **Export selection…**. It is both a repeatable test plan and the place
where results are recorded before marking the draft pull request ready for review.

The feature is accepted only if it exports the requested musical range from the
original score, applies the temporary rehearsal settings, and restores all
playback and mixer state afterwards.

## Result legend

- **PASS** — observed result matches the expected result.
- **PARTIAL** — a named subcase passed, but other expected checks remain unverified.
- **FAIL** — result differs from the expectation; link the issue or add notes.
- **BLOCKED** — the test cannot currently be executed; explain why.
- **READY** — implemented or specified, but not yet executed for this build.
- **N/A** — intentionally not applicable to the tested platform or build.

## Execution record

| Field | Value |
|---|---|
| Test date | 2026-09-14 manual validation; 2026-09-15 refreshed build and extended regression checks |
| Tester | User manual confirmations and development build; remaining boundary, accessibility, and failure cases tracked below |
| Platform | Windows, x64 |
| Build type | RelWithDebInfo |
| Qt | 6.10.2, MSVC 2022 x64 |
| MuseScore base revision | `87e6a2cc1973` |
| Muse Framework base revision | `34ecec524d1b` |
| Feature revision | Published MuseScore draft `876d398de8` and framework draft `4ffe3392b94c`; unsuccessful local cleanup candidate reverted, test results documented here |
| Executable | `C:\Users\afortun8\b\m\install\bin\MuseScoreStudio5.exe` |
| Build result | **PASS** — complete compile, link, and install after upstream refresh on 2026-09-15; one transient `LNK1104` at final link resolved by retry |
| PR style-only revision rebuild | **PASS** — compile, link, and install on 2026-09-15 after closing MuseScore; linked and installed executable SHA-256 hashes agree |
| Cancellation-fix rebuild | **PASS** — compile, link, install, and linked/installed SHA-256 equality on 2026-09-15; user confirmed that in-progress abort no longer crashes standard or selection audio export, while canceled MP3 cleanup remains a known limitation |
| Unpublished cleanup experiment | **FAIL** — compile, link, and install passed, but user testing found the partial MP3 still remained and a new program runtime regression; the candidate was reverted and the published non-crashing code was rebuilt and reinstalled |
| Code-style check | **PASS** — Uncrustify 0.74.0 checks all 13 framework and 35 application C++ files touched by the feature |
| Audio unit-test build | **PASS** — isolated `muse_audio_tests` target compiled and linked |
| Audio unit-test run | **BLOCKED** — Windows runner exits with `0xC0000005` before publishing GoogleTest results |
| MuseScore PR unit-test CI | **FAIL** — [current `run_tests` job](https://github.com/musescore/MuseScore/actions/runs/34966230344/job/104371224603) built successfully, but its **Run tests** step failed; the exact failing assertion or process exit has not yet been verified from the job log |

### Launching the local test build on Windows

Always launch the installed executable:

```text
C:\Users\afortun8\b\m\install\bin\MuseScoreStudio5.exe
```

Do not launch `C:\Users\afortun8\b\m\MuseScoreStudio5.exe`. That is the
intermediate linker output and its directory does not contain runtime libraries
such as `FLAC.dll`. A Windows “FLAC.dll was not found” dialog when using that
path is a launch-path error, not an application build or feature failure.

The MSVC `C4702` warnings emitted from Qt 6.10.2 headers are pre-existing
third-party warnings and do not fail compilation. They should not be reported as
feature defects unless accompanied by a runtime failure.

The unit-test runner issue was reproduced in both the existing build folder and
a fresh, isolated build folder. A pre-existing alignment-buffer test and the new
save-options tests terminate with the same access violation inside
`MSVCP140.dll`, before GoogleTest writes console or XML results. Therefore no
unit test is marked PASS from this local run. Re-run the compiled tests in CI or
on a second supported environment; track the Windows runner failure separately
if it reproduces there.

The manual PASS results in the tables below were observed on the
2026-09-14 installed build. Both feature commits were subsequently rebased
onto newer upstream `main` revisions, and the 2026-09-15 installed build
compiled, linked, and installed successfully. The post-rebase manual smoke
test (BUILD-02) passed on the refreshed build: two MP3 selections exported
the correct ranges, the enabled metronome sounded its first downbeat, tempo
and background levels were applied, and subsequent normal playback retained
its original settings. More scenarios were user-tested on 2026-09-15 and are
marked individually below. Repeat-specific fixtures, extreme fades, keyboard
navigation, and abort/error recovery still need attention; individual manual
results from 2026-09-14 should not be presented as a full retest of every
scenario on the refreshed build. After the generic audio-writer cancellation
fix, the user canceled both a standard full-score export and a selection export
in progress without a process crash. On the selection path, original tempo and
instrument volumes were retained, but the partial MP3 remained and played only
the portion rendered before cancellation. Exact mute/solo state after abort was
not separately recorded.

One test-score fragment exhibits the same sound-rendering anomaly during live
playback and selection export. Because the exported result matches MuseScore's
own live playback for the identical fragment, it is recorded as an upstream
playback/sound-profile observation rather than an export-selection defect. It
does not affect the range, tempo, mix, fade, metronome, or restoration results.

## Reference scores

Use small, inspectable scores so failures can be diagnosed without relying only
on waveform comparison.

### F1 — Core chamber score

- At least four independent parts: piano, strings, wind, and percussion.
- Eight or more measures in 4/4.
- A dynamic before the selected range that remains active inside it.
- A tempo marking before the range and a tempo change inside it.
- Clearly different rhythmic material in every part.
- Notes sounding exactly at both selection boundaries.

### F2 — Meter and metronome score

- Consecutive sections in 4/4, 3/4, 6/8, and 5/8.
- An anacrusis/pickup measure.
- Selections beginning both on a barline and partway through a measure.
- Reuse `src/engraving/tests/playback/playbackeventsrenderer_data/count_in.mscx`
  for anacrusis, 4/4, 3/8, and mid-measure alignment.
- Reuse `src/importexport/midi/tests/midiexport_data/testMetronomeCompound.mscx`
  for a compound-meter manual check.

### F3 — Playback-context score

- A repeat, first/second endings, an instrument change, articulations, playing
  techniques, and sustained notes crossing a selection boundary.
- Useful existing repeat fixtures include
  `src/engraving/tests/playback/playbackmodel_data/repeat_range/` and
  `repeat_tempo_changes_and_tie/`.

### F4 — Large score

- Between 20 and 30 parts, with unique short rhythmic identifiers.
- Long enough to observe export time and UI responsiveness.

The existing `TestExportacio.mscz` may be used as F1 if it contains the required
markers. F2–F4 can be purpose-built or adapted from repository test data.

## Evidence to retain

For each executed case, record the result and short notes in the tables below.
For representative audio cases, retain:

- output filename and format;
- selected measure/staff range;
- dialog settings;
- observed duration, with a tolerance of one encoded audio frame;
- a waveform or spectrogram screenshot when the boundary, fade, or metronome is
  the subject of the test;
- the score file and final feature commit SHA.

Do not attach every generated audio file to the pull request. The completed
tables plus a compact set of representative WAV/MP3 files and screenshots are
sufficient.

## A. Build, command availability, and regressions

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| BUILD-01 | Configure, compile, link, and install the desktop application. | Build and install complete with exit code 0. | **PASS** | RelWithDebInfo refreshed build installed 2026-09-15 after retrying a transient final-link file lock. |
| BUILD-02 | Launch the refreshed installed build and export one middle-score range with metronome Off, then another with metronome On, changed tempo and background level. | Both exports contain the requested range and settings; the first downbeat clicks when enabled; normal playback state remains unchanged. | **PASS** | User-confirmed on 2026-09-15: two correct MP3 fragments; first metronome tick, tempo and levels correct; normal playback settings restored. |
| ACT-01 | Create a contiguous range selection and open the main File menu. | **Export selection…** is enabled and opens the selection export dialog. | **PASS** | Manually confirmed during development. |
| ACT-02 | Right-click a contiguous range selection. | The context menu contains **Export selection…** and opens the same dialog. | **PASS** | Manually confirmed during development. |
| ACT-03 | Test with no selection, a single note, and a list selection. | The command is unavailable; no invalid export starts. | **PASS** | User confirmed the unavailable-command behaviour with no valid range on 2026-09-15. |
| ACT-04 | Open normal **Export…** without using selection export. | Existing formats, options, duration, and full-score rendering are unchanged. | **PASS** | Normal full-score export and dialog remain unchanged, user-confirmed 2026-09-15. |
| ACT-05 | Open selection export and inspect the format list. | Only supported audio formats are offered. | **PASS** | Manually confirmed during development. |
| ACT-06 | Inspect the normal and selection dialogs. | Normal-export-only text such as per-part export guidance is absent from selection mode. | **PASS** | Normal-export guidance remains only in the normal dialog, user-confirmed 2026-09-15. |

### Automated-test status

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| AUTO-01 | Compile the audio unit-test target in a fresh tests-enabled build. | New save-option validation tests and RPC packing test compile and link. | **PASS** | Isolated 34-step target build completed. |
| AUTO-02 | Run `Audio_SoundTrackSaveOptionsTests.*`. | Seven validation tests pass. | **BLOCKED** | Runner exits `0xC0000005` before test reporting. |
| AUTO-03 | Run `Audio_RpcPackerTests.SoundTrackSaveOptions`. | Range and fade fields round-trip exactly. | **BLOCKED** | Same runner failure. |
| AUTO-04 | Run a pre-existing audio baseline test. | Baseline test passes independently of feature code. | **BLOCKED** | `Audio_AlignmentBufferTests.*` has the identical runner failure. |

## B. Range and musical-context accuracy

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| RNG-01 | In F1, select complete measures in the middle of the score and export at 100%, no fades. | Audio begins at the selected start and stops at the exclusive end; earlier/later measures are absent. | **PASS** | Range capture manually confirmed. |
| RNG-02 | Select different horizontal ranges while keeping the same parts. | Each file contains exactly its own selected range. | **PASS** | Manually confirmed repeatedly with metronome off. |
| RNG-03 | Export a range beginning at score time zero. | No negative seek or unexpected leading silence occurs. | **PASS** | Export beginning at the score start behaved correctly, user-confirmed 2026-09-15. |
| RNG-04 | Export a range ending at the score end. | Export completes normally with no overrun or hang. | **PASS** | Export through the final measure behaved correctly; no post-score fade-out margin was added. |
| RNG-05 | In F1, place `p` or `f` before the selection and no dynamic inside it. | The selected notes retain the prior dynamic context. | READY | User heard correct dynamics throughout the score on 2026-09-15; the isolated prior-marker/no-inside-marker subcase remains to be checked. |
| RNG-06 | Use a tempo marking before the selection and a tempo change inside it. | Both affect the export at the correct musical positions. | **PASS** | Prior and internal tempo markings followed the expected playback, user-confirmed 2026-09-15. |
| RNG-07 | In F3, select across articulations, techniques, automation, and an instrument change. | The original score’s evaluated playback context is preserved. | READY | General playback appeared correct, but an F3 fixture with all named events was not identified or checked individually. |
| RNG-08 | Select a region outside repeats in F3. | Exported audio maps to the correct unfolded playback position. | READY | General behaviour appeared correct; repeat-specific F3 comparison remains. |
| RNG-09 | Select within a repeated region in F3. | Behaviour is deterministic and matches the documented repeat policy. | READY | General behaviour appeared correct; record the rendered occurrence in a dedicated repeat fixture. |
| RNG-10 | Export a sustained note crossing the start or end boundary. | Result follows the documented exact-range/fade policy without earlier unrelated music. | READY | |
| RNG-11 | With MuseSounds active, export a selection beginning well after the score start, then compare it with live playback of the same range. | Audio begins at the selected music, not at the beginning of the score, and retains the requested duration. | **PASS** | User confirmed on 2026-09-15 that ranged MuseSounds exports match normal playback and start at the requested music. Exact score/measures and MuseSampler version remain to be logged as supporting evidence against [#24369](https://github.com/musescore/MuseScore/pull/24369) and [#32564](https://github.com/musescore/MuseScore/pull/32564). |

## C. Formats and duration consistency

Run the same F1 selection with identical settings. Formats unavailable in a
particular distribution may be marked N/A.

| ID | Format | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| FMT-01 | WAV | Opens successfully; correct range and duration. | **PASS** | Manually confirmed on the compact-dialog build. |
| FMT-02 | MP3 | Opens successfully; same musical content and expected encoder-padding tolerance. | **PASS** | Multiple exports manually confirmed. |
| FMT-03 | FLAC | Opens successfully; duration agrees with WAV. | **PASS** | Manually confirmed on the compact-dialog build. |
| FMT-04 | OGG | Opens successfully; duration agrees within codec tolerance. | **PASS** | Manually confirmed on the compact-dialog build. |
| FMT-05 | AAC | Opens successfully; duration agrees within codec tolerance. | **PASS** | AAC export succeeded, user-confirmed 2026-09-15. |
| FMT-06 | Repeat WAV then MP3 export without restarting MuseScore. | Both exports succeed and neither inherits stale state. | **PASS** | MP3, WAV, and another MP3 succeeded consecutively without stale options or errors. |

## D. Tempo controls

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| TMP-01 | Export at 100%. | Duration and tempo match normal score playback. | **PASS** | Manually confirmed. |
| TMP-02 | Export at 75%. | Musical duration is approximately `100/75` of the 100% version. | **PASS** | Manually confirmed. |
| TMP-03 | Export at 50%. | Musical duration is approximately twice the 100% version. | **PASS** | User confirmed correct behaviour at 50% and other tempo percentages on 2026-09-15. |
| TMP-04 | Export at the minimum value (10%). | Export completes; no zero/negative tempo or hang. | **PASS** | Minimum-tempo export completed normally, user-confirmed 2026-09-15. |
| TMP-05 | Export at the maximum value (300%). | Export completes; no truncation or seek error. | **PASS** | Maximum-tempo export completed normally, user-confirmed 2026-09-15. |
| TMP-06 | Drag the tempo thumb continuously and click several rail positions. | Value tracks smoothly and the final displayed value is exported. | **PASS** | Manually confirmed. |
| TMP-07 | After a non-100% export, play the score normally. | Playback returns to the user’s pre-export tempo multiplier. | **PASS** | Manually confirmed; real-time playback retains its original parameters. |

## E. Instrument focus and volume mix

All percentages are relative to each track’s existing mix, not replacements for
the project mixer setting.

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| MIX-01 | Select one part. | Selected part defaults to 100%; other parts default to muted. | **PASS** | Manually confirmed. |
| MIX-02 | Select two or more parts. | Selected parts default to 100%; other parts default to 50%. | **PASS** | Manually confirmed. |
| MIX-03 | Move the global **Other instruments** slider. | Every background-part slider follows it and export uses the shown values. | **PASS** | Manually confirmed. |
| MIX-04 | Enable **Mute other instruments**. | All background parts are silent; selected parts remain audible. | **PASS** | Manually confirmed. |
| MIX-05 | Disable mute after setting different individual background levels. | The prior individual values are retained/restored in the dialog. | **PASS** | Individual background values remained available after toggling mute, user-confirmed 2026-09-15. |
| MIX-06 | Drag each individual slider continuously and click the rail. | Thumb movement is smooth; no release/re-grab is needed. | **PASS** | Regression for the former 5%-step drag defect. |
| MIX-07 | Export individual values 0%, 25%, 50%, 100%, 150%, and 200%. | Audible levels follow the requested relative gains, including values above 100%. | **PASS** | User confirmed the individual gain values, including boosted levels, on 2026-09-15. |
| MIX-08 | Give different values to three background instruments. | Each instrument uses its own value in the rendered file. | **PASS** | Manually confirmed. |
| MIX-09 | Use a score whose mixer channels already have different fader levels. | 100% preserves the existing mix; other values scale it relatively. | **PASS** | Existing mixer levels were used as the starting values, user-confirmed 2026-09-15. |
| MIX-10 | Open F4 with 20–30 instruments. | Background details start collapsed, expand on demand, and scroll internally without growing beyond the dialog. | **PASS** | Collapsing/scrolling behaviour manually confirmed; repeat with F4 scale. |
| MIX-11 | Export, cancel a second export, then inspect/play the mixer. | Original mute, solo, and volume state is unchanged. | READY | Earlier normal cancellation left the mixer unchanged; in-progress cancellation now avoids a crash in both export paths, but exact post-abort mute, solo, and volume values were not recorded. |

## F. Fade-in and fade-out

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| FAD-01 | Export with both fades disabled. | Exact selected range is rendered, with no added pre/post musical margin. | **PASS** | Manually confirmed. |
| FAD-02 | Enable a 2.0 s fade-in only. | Up to 2.0 s before the selection is included and rises smoothly to full level at the selection start. | **PASS** | Manually confirmed. |
| FAD-03 | Enable a 2.0 s fade-out only. | Up to 2.0 s after the selection is included and falls smoothly to silence. | **PASS** | Manually confirmed. |
| FAD-04 | Enable both fades with different durations. | Both margins and ramps are independent and correctly timed. | **PASS** | Manually confirmed. |
| FAD-05 | Request fade-in longer than available audio before score start. | Fade is safely shortened to the available duration. | **PASS** | At score start there is no preceding music; the export safely starts there without an invented fade margin. |
| FAD-06 | Request fade-out longer than available audio after score end. | Fade is safely shortened to the available duration. | **PASS** | At score end there is no following music; the requested fade-out safely contributes no post-score margin. |
| FAD-07 | Use minimum (0.1 s) and maximum (30 s) UI durations. | Export remains valid, finite, and bounded by score duration. | READY | Ordinary 1 s and 5 s durations worked; the 0.1 s and 30 s boundary values were not tested. |
| FAD-08 | Inspect representative WAV waveforms at both joins. | Gain ramps are monotonic and no abrupt full-scale cut/click is introduced. | READY | User heard no extra clicks or unexpected volume changes; actual waveform/ramp inspection and screenshot remain pending. |

## G. Metronome

The dialog offers **Off** or **Throughout exported fragment**. Choosing the
active mode automatically disables and unchecks the pre-selection fade-in;
fade-out remains available. A one-measure count-in is explicitly deferred from
this PR because its interactive event stream is not atomic with offline export.

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| MET-01 | Choose **Off** while the toolbar metronome is enabled. | Export contains no click. | **PASS** | Manually confirmed; export choice overrides, but does not persist over, toolbar state. |
| MET-02 | Export several different ranges with **Throughout exported fragment**. | Every file contains the requested musical range; enabling the metronome does not change range mapping. | **PASS** | Manually confirmed; the former displaced-range defect occurred only in the now-deferred count-in prototype. |
| MET-03 | Export from a barline with **Throughout exported fragment**. | The downbeat click is audible on the first selected beat and remains synchronized throughout. | **PASS** | Manually confirmed after aligning the export start down to the preceding audio-sample boundary. |
| MET-04 | Run Off and Throughout at 50%, 75%, and 150% tempo. | Click timing follows the export tempo exactly. | **PASS** | User confirmed metronome tempo alignment across the requested values on 2026-09-15. |
| MET-05 | Enable fade-in, then choose **Throughout exported fragment**. | Fade-in is automatically unchecked and disabled; selecting Off makes it available again. | **PASS** | Manually confirmed. |
| MET-06 | Export throughout-click with fade-out both disabled and enabled. | Range and click remain correct in both files; fade-out affects the final mixed signal. | **PASS** | Manually confirmed, including the first click covered by MET-03. |
| MET-07 | Test with toolbar metronome initially off, then initially on. | After every export, the toolbar setting and normal playback behaviour are exactly restored. | **PASS** | Real-time playback state manually confirmed unchanged after export. |
| MET-08 | Export different nonzero ranges successively with the metronome active. | Every file starts at its own selected range and no isolated click sounds through the live interface. | **PASS** | Manually confirmed after removing the interactive count-in path. |
| MET-09 | Export active metronome to WAV and one compressed format. | Both contain the same click pattern and musical start time within codec tolerance. | **PASS** | WAV and compressed exports contained the expected metronome pattern, user-confirmed 2026-09-15. |
| MET-10 | Start playback, invoke selection export, and export with active metronome. | Playback stops safely; export succeeds; subsequent playback remains usable. | READY | Related subcase passed: toolbar metronome initially on, selection export set Off, and no exported click; exporting while playback runs with metronome On remains untested. |

## H. Cancellation, failure, and state restoration

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| STATE-01 | Export with changed tempo, volumes, fades, and metronome, then play normally. | Original tempo, mixer, metronome, and playback state are restored. | **PASS** | Manually confirmed across development iterations. |
| STATE-02 | Change settings and cancel from the dialog before choosing a path. | No state changes escape the dialog; score is not dirty. | **PASS** | Canceling the dialog before export leaves the score and playback unchanged, user-confirmed 2026-09-15. |
| STATE-03 | Cancel/abort while an export is in progress, if supported by the progress dialog. | Temporary state is restored and no unusable partial file is presented as successful. | **FAIL** | Pre-fix build crashed in both selection and standard full-score audio export (Windows 0xC0000005 in MuseSamplerCoreLib.dll 0.105.7.1189). The first generic abort correction stopped both crashes; the user confirmed original tempo and volumes are restored. However, a canceled MP3 remains on disk and plays only the rendered portion. A local cancellation-result follow-up failed to clean it and caused a runtime regression, so it was reverted before publication. |
| STATE-04 | Export to a read-only/invalid destination. | A clear error is shown; temporary playback and mixer state is restored. | READY | Not attempted; identify a safe temporary destination for this test. |
| STATE-05 | Export twice with opposite settings without restarting. | Second file reflects only the second settings; no stale range, gain, fade, or click remains. | **PASS** | Opposite settings were applied independently in consecutive exports, user-confirmed 2026-09-15. |
| STATE-06 | Compare the project dirty indicator and save prompt before/after export. | Export does not modify or dirty the score. | **PASS** | Manually confirmed: temporary tempo, level, and metronome settings do not modify the score. |
| STATE-07 | Close the selection dialog and run normal full-score export. | Full export still starts at zero and uses the normal mixer/metronome policy. | **PASS** | Normal full-score export retained its usual behaviour after using selection export, user-confirmed 2026-09-15. |

## I. UI, accessibility, and scale

| ID | Procedure | Expected result | Status | Notes / evidence |
|---|---|---|---|---|
| UI-01 | Resize/open the dialog at common 100%, 125%, and 150% Windows display scaling. | Controls fit, labels are readable, and no essential control is clipped. | READY | Dialog is not user-resizable, as with normal Export; the 100%/125%/150% scaling subcases were not executed. |
| UI-02 | Traverse controls using keyboard navigation only. | Focus order follows tempo, metronome, fades, background controls, then individual instruments. | **FAIL** | Tab cycles only Export and audio format. Arrow navigation can reach tempo, metronome, fades, codec fields, mute and instrument controls, but its order is unintuitive; horizontal arrows inside numeric fields edit the cursor instead of moving focus. User-confirmed 2026-09-15. |
| UI-03 | Operate sliders with mouse drag, rail click, and keyboard. | All methods update predictably and accessible names report purpose/value. | **FAIL** | Mouse drag, rail click, and wheel worked. Up/Down on the first instrument percentage changes the value by 5% then jumps focus to Open folder on export; tempo Up/Down continues editing tempo. Accessible-name verification remains pending. User-confirmed 2026-09-15. |
| UI-04 | Expand/collapse background instruments repeatedly. | State changes reliably; internal scrolling does not move unrelated dialog content unexpectedly. | **PASS** | Core interaction manually confirmed. |
| UI-05 | Test long and duplicate instrument names. | Labels remain distinguishable or expose sufficient identifying context; layout stays usable. | READY | Not yet tested with long or duplicate part names. |
| UI-06 | Run the translatable-string extraction/check used by MuseScore. | All new user-facing strings are discoverable through the project translation context. | **PASS** | Qt `lupdate` found all 17 expected feature strings, including the menu mnemonic and accessible names. Locale catalogues remain managed by Transifex. |
| UI-07 | Inspect the compact dialog with one instrument and with several instruments, including formats with different codec controls. | Tempo/metronome, fade controls, background mute/level, and format controls use their intended compact rows; the instrument area remains usable. | **PASS** | Manually confirmed on 2026-09-14; layout is substantially more compact and clear. Display-scaling coverage remains tracked separately in UI-01. |
| PERF-01 | Export a short F4 range with 20–30 parts. | Dialog remains responsive and export completes without excessive memory growth. | READY | Record elapsed time. |
| PERF-02 | Export a longer orchestral range, then immediately export again. | Both complete; no crash, hang, or progressive slowdown is observed. | READY | |

## Pull-request completion gate

Before marking either draft PR ready for review:

1. Record the exact application and framework commit SHAs in the PR descriptions.
2. Execute every high-priority case: BUILD-01, RNG-01, RNG-05, FMT-01,
   FMT-02, TMP-02, MIX-01, MIX-02, MIX-07, FAD-04, MET-01, MET-03,
   MET-07, MET-08, STATE-01, STATE-04, STATE-06, and ACT-04.
3. Disclose STATE-03's known canceled-MP3 partial-file limitation and the
   UI-02/UI-03 keyboard-navigation limitations to maintainers. The candidate
   cleanup fix that regressed at runtime is excluded from the PR; its original
   non-crashing cancellation path was rebuilt and reinstalled.
4. Run available automated unit tests for audio range validation and RPC
   packing.
5. Attach the completed result summary plus representative boundary/fade/
   metronome evidence.

## PR result-summary template

```text
Validation environment:
- OS:
- MuseScore commit:
- Muse Framework commit:
- Build type / Qt:

Results:
- Build and install: PASS/FAIL
- Range and context: __ passed, __ failed, __ N/A
- Formats: __ passed, __ failed, __ N/A
- Tempo and mix: __ passed, __ failed, __ N/A
- Fades: __ passed, __ failed, __ N/A
- Metronome: __ passed, __ failed, __ N/A
- State restoration/regression: __ passed, __ failed, __ N/A
- UI/accessibility/performance: __ passed, __ failed, __ N/A

Known limitations:
-

Representative evidence:
-
```
