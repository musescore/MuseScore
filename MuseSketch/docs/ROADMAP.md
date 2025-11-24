Here’s a **technical roadmap** broken into PRs, each with **numbered subtasks** so an engineer can execute step-by-step. I’ll assume:

* **Tech stack:** C++17 + Qt 6 + QML for UI
* **Engraving:** MuseScore core as a linked library or submodule
* **Platforms:** iOS + Android (desktop builds are allowed for dev/debug, but mobile is the product)

You can tweak stack later, but this gives a very concrete plan.

---

## PR-01 — Repo Bootstrap & Project Skeleton

**Goal:** Create the MuseSketch repo with a compilable “Hello world” Qt/QML app and basic structure.

**Subtasks**

1. **Initialize repo**
   1.1. Create `musesketch/` repo with standard layout:
   `app/`, `core/`, `third_party/`, `cmake/`, `docs/`, `scripts/`, `tests/`.
   1.2. Add `.gitignore` for CMake/Qt/build artifacts.
   1.3. Add `LICENSE` (GPL-3 or GPL-compatible).

2. **CMake + Qt project setup**
   2.1. Add top-level `CMakeLists.txt` with Qt 6 find_package and options for `iOS`, `Android`, `Desktop`.
   2.2. Add `app/CMakeLists.txt` to build `musesketch` executable.
   2.3. Configure minimum viable cross-compile flags for Android/iOS (no store packaging yet).

3. **QML entry point**
   3.1. Create `app/main.cpp` that initializes `QGuiApplication`/`QQmlApplicationEngine`.
   3.2. Create `app/qml/Main.qml` with a simple “MuseSketch” title screen.
   3.3. Wire up CMake install rules for QML/asset deployment.

4. **CI basics**
   4.1. Add GitHub Actions or other CI to:
   - Configure & build on Linux/macOS.
   4.2. Run at least a dummy test (e.g., `ctest` placeholder).

5. **Docs**
   5.1. Add `docs/README-dev.md` with build instructions (Desktop debug build).
   5.2. Document required tools: CMake version, Qt version, compilers, Android/iOS SDKs.

---

## PR-02 — Integrate MuseScore Core (Read-Only Engraving Demo)

**Goal:** Pull in MuseScore’s engraving engine & fonts and prove we can render *any* score in the MuseSketch UI.

**Subtasks**

1. **Bring in MuseScore core**
   1.1. Add MuseScore as a git submodule in `third_party/musescore/` **or** set up CMake `FetchContent`.
   1.2. Identify minimal modules needed: engraving, project, notation, fonts.
   1.3. Add `core/CMakeLists.txt` to build a `libmusescore_core` static or shared library.

2. **Thin wrapper API**
   2.1. In `core/include/`, create `ScoreEngine.h` with class `ScoreEngine` that can:
   - Load a `.mscz` or `.mscx` file.
   - Create a simple new score with one staff.
   2.2. Implement in `core/src/ScoreEngine.cpp` by delegating to MuseScore APIs.
   2.3. Add basic error handling (file not found, invalid score).

3. **Rendering to QImage**
   3.1. Extend `ScoreEngine` to render first page to `QImage`.
   3.2. In QML, create a `ScoreView` component backed by a `QQuickPaintedItem` or similar that draws that `QImage`.
   3.3. Add zoom/pan support (simple pinch/drag).

4. **Demo screen**
   4.1. Add a dev-only “Load sample score” button in `Main.qml`.
   4.2. Bundle a sample `.mscz` in `app/assets/`.
   4.3. Verify engraved output appears on both desktop and mobile emulator.

5. **Tests**
   5.1. Add a unit test to load a `.mscz` and assert no exceptions / null pointers.
   5.2. Add a render smoke test (e.g., check `QImage` non-zero size).

---

## PR-03 — Domain Models: Sketch, Motif, Section

**Goal:** Define data models independent of the MuseScore score for **Sketch**, **Motif**, and **Section**, with local persistence.

**Subtasks**

1. **Core data structures**
   1.1. In `core/include/`, create:
   - `Sketch.h`
   - `Motif.h`
   - `Section.h`
   1.2. Implement in `core/src/`:
   - `Sketch` = `{ id, name, key, tempo, timeSig, motifs[], sections[] }`
   - `Motif`  = `{ id, name, lengthBars, pitchContour[], rhythmicPattern[], keyRef }`
   - `Section` = `{ id, name, lengthBars, placements[] }` where placements reference motif IDs + targets (voice/staff & bar index).

2. **Serialization**
   2.1. Implement JSON serialization / deserialization (e.g., via Qt’s `QJsonObject` / `QJsonDocument`).
   2.2. Add `core/SketchRepository` to load/save sketches to disk (e.g., app data directory).

3. **Repository APIs**
   3.1. Define `SketchRepository::createSketch`, `loadSketch`, `saveSketch`, `listSketches`.
   3.2. Add caching in memory (simple map of `sketchId → Sketch`).

4. **QML exposure**
   4.1. Expose a `SketchManager` QObject to QML:
   - `sketchList` model
   - `createNewSketch(name)`
   - `openSketch(id)`
   4.2. Use QQmlContext to inject into QML root.

5. **UI: Sketch list**
   5.1. Implement `SketchListView.qml`:
   - Basic list of sketches with name + last modified.
   - Fab/button “New Sketch”.

6. **Tests**
   6.1. Unit tests for serialization round-trip of Sketch/Motif/Section.
   6.2. Unit tests for repository create/load/save cycle.

---

## PR-04 — Motif Shape Mode (Pitch Contour + Rhythm Grid)

**Goal:** Implement the **touch-native motif creation interface** (Shape Mode) and map it into the Motif data model.

**Subtasks**

1. **MotifEditor controller**
   1.1. Add `MotifEditorController` QObject in `core` with methods:
   - `startNewMotif(sketchId)`
   - `setKey(key)` / `setTimeSignature(...)`
   - APIs to mutate contour & rhythm.
   1.2. Expose as `MotifEditor` singleton to QML.

2. **Pitch contour representation**
   2.1. Define internal representation: list of scale-degree offsets and durations.
   2.2. Add helper functions for:
   - Quantizing drawn path to scale degrees.
   - Converting pad taps to stepwise intervals or leaps.

3. **Rhythm grid representation**
   3.1. Create a `RhythmGrid` data structure in `Motif`: array of `RhythmCell { duration, tie, isRest }`.
   3.2. Implement quantization from touch events (tap, drag, pinch) to cells.

4. **UI: Shape Mode QML**
   4.1. Create `MotifShapeView.qml` with:
   - `ContourCanvas` (custom QQuickItem or Canvas) to capture draw gestures.
   - Scale-degree pad row (buttons 1–7).
   - Rhythm strip (grid of touchable cells).
   - Playback controls.
   4.2. Wire gestures:
   - Draw on contour → call `MotifEditor::applyContourPath(pathPoints)`.
   - Tap pad → add note at current time position.
   - Rhythm strip taps → toggle note/rest, change durations.

5. **Preview playback**
   5.1. Rough MIDI/Qt audio playback: just map scale degrees to pitches in current key.
   5.2. Add simple metronome click aligned with rhythm grid.

6. **Persist motif**
   6.1. On “Save Motif”, call `MotifEditorController::commit()` to inject motif into current `Sketch`.
   6.2. Update `SketchRepository` to save motif data.

7. **Tests**
   7.1. Unit tests for contour quantization (path → scale degrees).
   7.2. Unit tests for rhythm strip interactions → expected `RhythmGrid` sequence.

---

## PR-05 — Motif → Notation Mode via MuseScore Engine

**Goal:** Convert a Motif into an internal MuseScore score fragment and display/edit it in staff notation (Notation Mode).

**Subtasks**

1. **Motif → ScoreEngine translation**
   1.1. Add `ScoreEngine::createScoreFromMotif(const Motif&, const Sketch&)`.
   1.2. Create a single-staff score, add measures, and map motif pitch/rhythm into MuseScore notes.
   1.3. Handle accidentals based on key and scale degrees.

2. **Notation Mode view**
   2.1. Create `MotifNotationView.qml` using same `ScoreView` from PR-02 but focused on 1 staff.
   2.2. Allow tap to select note, long-press to open an edit menu (change pitch, duration, accidental).
   2.3. Add toolbar with:
   - Pitch up/down
   - Duration modifications
   - Slur toggle
   - Undo/redo

3. **Round-trip updates**
   3.1. Implement `ScoreEngine::applyEditToMotif(...)` or re-derive motif from score on save.
   3.2. On exit Notation Mode, regenerate `Motif` from the MuseScore score fragment and update the Sketch.

4. **Navigation**
   4.1. From Motif Library → tap motif → open Notation Mode.
   4.2. From Shape Mode → “Commit to notation & refine” button → Notation Mode.

5. **Tests**
   5.1. Unit test: motif → score → motif round-trip with tolerance (e.g., same pitches/durations).
   5.2. UI test (manual/automated) to ensure taps select right note.

---

## PR-06 — Motif Library UI & CRUD

**Goal:** Manage motifs per Sketch: list, rename, duplicate, delete, and open for edit.

**Subtasks**

1. **Motif list model**
   1.1. Add `MotifListModel` QAbstractListModel to provide motifs to QML.
   1.2. Expose fields: name, length, usage count, last modified.

2. **UI: Motif Library**
   2.1. Create `MotifLibraryView.qml` with list/grid of motif cards.
   2.2. Each card shows name + 1–2 bar preview (optional tiny rendered image or text).

3. **Actions**
   3.1. Add rename action (inline edit).
   3.2. Add duplicate action.
   3.3. Add delete action with confirm.
   3.4. Tap motif → open Notation Mode.
   3.5. FAB / button: “New Motif” → Shape Mode.

4. **Persistence**
   4.1. Ensure Sketch saves motif changes on mutation.

5. **Tests**
   5.1. Unit tests for Motif CRUD operations on Sketch.
   5.2. Minimal UI test to ensure deletion updates the view.

---

## PR-07 — Section Editor: Single-Staff Arrangement of Motifs

**Goal:** Implement **Sections** and the ability to place motifs along a single staff timeline as a precursor to SATB.

**Subtasks**

1. **Section placement logic**
   1.1. Extend `Section` to keep `MotifPlacement { motifId, startBar, targetVoice }`, starting with single “melody voice”.
   1.2. Add methods to compute a flattened melodic timeline from placements.

2. **Section → MuseScore score**
   2.1. Implement `ScoreEngine::createScoreFromSection(section, sketch)` for a single staff.
   2.2. Concatenate motifs in order, respecting `startBar`.

3. **Section Editor UI**
   3.1. Create `SectionEditorView.qml`:
   - Top: section name & length
   - Middle: horizontal timeline lane where motif “blocks” can be placed/dragged.
   - Bottom: motif library mini-strip (for drag/drop or tap-to-place).

4. **Editing interactions**
   4.1. Tap on timeline → create an empty placement slot.
   4.2. Drag motif from library to timeline slot.
   4.3. Drag edges to extend/shrink number of repetitions (repeat motif).
   4.4. Tap placement → contextual menu (duplicate, remove).

5. **Playback**
   5.1. Hook Section playback to ScoreEngine’s rendering + audio.
   5.2. Add simple play/pause, loop section.

6. **Tests**
   6.1. Unit tests for flattening motif placements → note timeline.
   6.2. Validate that overlapping placements are either disallowed or resolved deterministically.

---

## PR-08 — Partwriting Engine v1: SATB Chorale Texture

**Goal:** Introduce the first **texture preset**: generate a simple SATB chorale from a melody motif.

**Subtasks**

1. **Texture engine API**
   1.1. In `core/PartwritingEngine.h`, define:
   - `generateChoraleTexture(const Motif&, const Sketch&) -> ChoraleVoices`
   where `ChoraleVoices` holds four lines (S,A,T,B) as simple pitch/duration sequences.
   1.2. Implement naive algorithm:
   - Use motif as soprano.
   - Harmonize with simple diatonic triads based on scale degrees.
   - Voice-lead inner voices by minimal pitch distance.

2. **Section + texture integration**
   2.1. Allow `Section` to specify `textureType` and store 4 independent voices.
   2.2. Implement `ScoreEngine::createSATBScoreFromSection(...)`.

3. **UI: texture preset**
   3.1. In `SectionEditorView.qml`, add “Texture” control:
   - Options: `Melody Only`, `SATB Chorale`.
   3.2. When switching to `SATB Chorale`:
   - Prompt user to choose which motif is soprano base.
   - Call `PartwritingEngine` to generate lines and update Section.

4. **SATB rendering**
   4.1. Extend `ScoreView` to optionally show 4 staves.
   4.2. On tablet: show full stack; on phone: show soprano only initially (later PR adds voice switching UI).

5. **Tests**
   5.1. Unit tests for `generateChoraleTexture` producing 4 aligned voices with consistent rhythm.
   5.2. Validate no illegal array access on edge cases (short motifs, repeated sections, etc.).

---

## PR-09 — SATB UI: Voices, Phone vs Tablet Layout

**Goal:** Build the **SATB editing UI**, including voice selection and responsive layouts for phone/tablet.

**Subtasks**

1. **Voice selection model**
   1.1. Add `VoiceSelectionController` with state: activeVoice (S/A/T/B), visibleVoices.
   1.2. Expose as QObject to QML.

2. **Phone layout**
   2.1. In `SectionEditorView.qml`, detect form factor via screen size/aspect ratio.
   2.2. On phone:
   - Display only activeVoice staff in main view.
   - Show “voice switcher” bar with S, A, T, B tabs.
   - Add small “mini stack” thumbnail showing approximate SATB alignment.

3. **Tablet layout**
   3.1. Display full SATB stack simultaneously.
   3.2. Still allow activeVoice highlighting (dim others).

4. **Voice editing**
   4.1. Route taps in score view to the currently activeVoice notes.
   4.2. Allow editing only activeVoice in Notation Mode for section (change pitch, duration).

5. **Tests**
   5.1. Manual verification with emulators & real devices (if available).
   5.2. Small UI test to ensure voice switching changes which notes are editable.

---

## PR-10 — Constraint Warnings (Parallel 5ths/8ves, Voice Crossing)

**Goal:** Add basic **voice-leading analysis** that surfaces warnings without blocking the user.

**Subtasks**

1. **Analysis engine**
   1.1. In `PartwritingEngine`, implement `analyzeVoices(const ChoraleVoices&) -> WarningList`.
   1.2. Detect:
   - Parallel 5ths/8ves between S–A, A–T, T–B, and outer voices S–B.
   - Voice crossing (e.g., Alto above Soprano).
   - Giant leaps (> octave) as soft warnings.

2. **Warning types & severities**
   2.1. Define `WarningType` enum (Parallel5th, Parallel8ve, Crossing, LargeLeap, etc.).
   2.2. Define severity (Info, Warning, Severe).

3. **UI indicators**
   3.1. Add a non-intrusive badge in the section editor: “Voice-leading warnings (3)”.
   3.2. Tap badge → open bottom sheet listing warnings:
   - “Parallel 5th between S and A at bar 4”
   - “Crossing between T and B at bar 7”

4. **Score highlighting**
   4.1. When user taps a warning, highlight offending notes in the score (color overlay).
   4.2. Support toggling display of warning overlays.

5. **Tests**
   5.1. Unit tests with known SATB lines that intentionally include parallels to confirm detection.
   5.2. Confirm no performance regressions on analysis for typical lengths (e.g., ≤32 bars).

---

## PR-11 — Playback Enhancements

**Goal:** Improve playback to make motif and section auditioning pleasant.

**Subtasks**

1. **Audio engine abstraction**
   1.1. Abstract audio playback behind `AudioEngine` interface (could be QtMultimedia, etc.).
   1.2. Support simple sample-based instruments, starting with piano.

2. **Motif playback**
   2.1. Ensure Shape Mode playback uses same audio backend as section playback.
   2.2. Add loop toggle for motifs.

3. **Section playback**
   3.1. Add loop section toggle.
   3.2. Add tempo slider (affects all playback).
   3.3. Add simple balance control: melody vs inner voices vs bass.

4. **Playback UI polish**
   4.1. Add playhead cursor on notation view.
   4.2. Snap scroll/zoom to keep playhead visible.

5. **Tests**
   5.1. Manual QA to ensure playback remains sync’d with rhythm for motifs and sections.
   5.2. Stress test: repeated play/stop/loop toggling.

---

## PR-12 — Export: MuseScore (.mscz), MusicXML, MIDI

**Goal:** Allow MuseSketch users to move work into **MuseScore Studio** and other tools.

**Subtasks**

1. **Score export**
   1.1. Implement `ScoreEngine::buildFullScoreFromSketch(sketch)` (monolithic score using sections in order).
   1.2. Implement `exportToMscz(sketch, filePath)` using MuseScore project APIs.
   1.3. Implement `exportToMusicXML(sketch, filePath)`.
   1.4. Implement `exportToMIDI(sketch, filePath)`.

2. **Export UI**
   2.1. Add “Export” screen:
   - Choose format: `.mscz`, `MusicXML`, `MIDI`.
   - Choose destination: local storage (Downloads / Files).
   2.2. On mobile, integrate with platform share sheet where possible.

3. **Validation**
   3.1. Manual test exported `.mscz` in MuseScore Studio.
   3.2. Manual test MusicXML in other notation apps.
   3.3. Manual test MIDI in DAW.

4. **Tests**
   4.1. Unit tests for export functions returning non-empty files.
   4.2. Round-trip check: export, re-import into MuseSketch (if feasible) and compare structures.

---

## PR-13 — Settings, Onboarding & Robustness

**Goal:** Wire up the “finish work”: settings, onboarding experience, crash logging.

**Subtasks**

1. **Settings**
   1.1. Add a `Settings` model with:
   - default key/time sig for new sketches
   - default texture preset
   - theme (light/dark/system)
   1.2. Simple settings UI screen in QML.

2. **Onboarding**
   2.1. First-run onboarding that:
   - Explains motifs vs sections vs textures in 3–4 screens.
   - Offers to create a sample sketch with a pre-made motif & SATB section.

3. **Error handling & logging**
   3.1. Add centralized error logger.
   3.2. On non-fatal engine exceptions, show user-friendly toast/snackbar.
   3.3. Optionally integrate with a crash-reporting SDK (if allowed).

4. **Polish & performance**
   4.1. Profile rendering & playback on typical mobile devices.
   4.2. Optimize QML bindings for motif/section lists.

5. **Tests**
   5.1. Sanity pass across all flows: new sketch → new motif → section → texture → playback → export.
   5.2. Confirm onboarding only appears once (or via “Replay tutorial”).


