# Drawing Mode Preparation Record

## P1 — Preparation baseline

This branch starts from the official MuseScore `main` baseline at commit `f019f4079d0ba74c481fa93396b3486a77f22635`. The preparation review keeps the source tree ready for a future Drawing Mode implementation while avoiding speculative scaffolding and visual changes.

### Source files modified

None in this revision. No source scaffolding was added because the reviewed official interfaces do not establish a necessary standalone preparation change; drawing behavior and its data model require implementation-specific design and validation.

### Existing integration points reviewed (unchanged)

| File | Location | Relevance to future implementation |
| --- | --- | --- |
| `src/notationscene/qml/MuseScore/NotationScene/abstractnotationpaintview.h` and `.cpp` | Class declaration and `load()` lifecycle | Existing notation view lifecycle and input handling. Any drawing gesture routing must preserve normal notation interaction when Drawing Mode is inactive. |
| `src/notation/inotationinteraction.h` | `INotationInteraction` declaration | Existing notation interaction boundary for score-related operations. |
| `src/notationscene/internal/notationsceneconfiguration.h` | `NotationSceneConfiguration` declaration | Existing scene configuration boundary for session-scoped state, if required by the final design. |
| `src/notationscene/internal/notationcommandsregister.cpp` and `notationactioncontroller.cpp` | Command list and controller registration | Existing command and action route for menu-accessible mode changes. |
| `src/notationscene/qml/MuseScore/NotationScene/notationtoolbarmodel.cpp` | `NotationToolBarModel::load()` | Existing toolbar model integration point. No toolbar or visual changes are part of this preparation revision. |
| `src/notationscene/qml/MuseScore/NotationScene/CMakeLists.txt` | QML resource list | Existing registration point for future QML components. |
| `src/appshell/qml/MuseScore/AppShell/NotationPage/NotationPage.qml` | Notation page composition | Existing application-level notation view composition. |

## P2 — Preparation review

The official `origin/main` snapshot was refreshed and resolves to the same commit as this branch. The drawing feature branch shares this baseline. A merge-tree review of the two branches completed without conflicts.

The source comparison showed that the drawing branch changes implement drawing behavior itself. No independent source change was identified as a prerequisite for that implementation. Adding abstractions before the score-data model and interaction design are established would be speculative, so this revision keeps preparation limited to the documented integration points.

### Scope and validation

- The branch and the refreshed official `main` share commit `f019f4079d0ba74c481fa93396b3486a77f22635`; the feature branch shares that base as well.
- This preparation revision changes no existing source files and introduces no visual changes.
- The branch comparison reported no merge conflicts. Compilation, automated tests, and runtime behavior were not evaluated.

## P3 — Windows build validation

The initial Windows builds failed while creating compiler databases and file-tracking output. The affected database paths were 260–267 characters long; tracking paths also exceeded 260 characters. Reducing parallelism and adding PDB synchronization did not resolve those failures.

A fresh build was configured in a short top-level output directory using the same source revision, Qt 6.12.0, Visual Studio 2022, and the default RelWithDebInfo compiler flags. Three previously failing targets completed successfully: `muse_legacytreeview_qml_resources_3`, `instrumentsscene_qml_resources_3`, and `muse_graphicaleffects_qmlplugin_init`. This isolates the reproduced failures to the output-path length.

The full Qt 6.12.0 build subsequently failed with an ambiguous `Type` name in generated QML code when compiled together with extension sources. The official Windows workflow for this source revision specifies Qt 6.11.2. A fresh short-path build using that version completed successfully, including the extensions module, complete RelWithDebInfo build, and automatic installation. No existing source files were changed for these build-environment corrections. The portable deployment correction and validation are recorded in P4.

## P4 — Windows Qt deployment from source paths containing spaces

The portable executable compiled successfully, but Qt deployment split the source directory at a space and treated the remainder as another argument. Quoting the source directory in the deployment options preserves it as one path when Qt generates and runs the installation script.

### Files added

None.

### Files modified

| File | Modified line | Reason |
| --- | ---: | --- |
| `src/app/CMakeLists.txt` | 363 | Quotes the source path supplied to the Qt deployment tool so installation works from directories containing spaces. |

### Validation

The generated-argument check reproduced the original path splitting and confirmed that the corrected path remains one argument. The portable build, installation, Qt deployment, and official PortableApps launcher generation completed successfully with Qt 6.11.2. The installed executable matches the compiled executable by SHA-256. A version-reporting startup check completed with exit code 0 after removing development Qt paths from the process environment. The portable package is approximately 462.9 MiB. Manual application startup and normal initial operation were confirmed. Detailed score-editing behavior has not been assessed.
