# Test Data Directory

This directory contains all data files used for testing MNX import/export functionality for MuseScore. The contents are intentionally split by origin and purpose.

## Directory Structure

### `project_examples/`
Project-authored MNX test cases and their expected MuseScore outputs.

- Contents:
  - `*.mnx` — project-specific MNX inputs (JSON format)
  - `*_ref.mscx` — expected MuseScore results for import/round-trip comparison
- Purpose: Exercise and validate importer/exporter behavior beyond the official MNX examples.
- Notes:
  - The `.mnx` extension is used by convention, pending a standardized MNX container format.
  - Some test cases are hand-edited versions from their exported counterparts; filenames indicate edits. (See the list of hand-edited files below.)

### `project_sources/`
This directory contains source files used to regenerate MNX project test cases when the schema changes.

- File formats:
  - `.enigmaxml` — exported by **denigma** from Finale sources
  - `.mscx` — MuseScore scores created within MuseScore
- Origin: Finale (via denigma) and MuseScore
- Conversion:
  - These files may be converted to MNX using the **denigma** command-line tool as an aid when updating test cases, particularly across significant schema changes.
  - Denigma may be downloaded or built from its [GitHub repository](https://github.com/rpatters1/denigma).
  - Maintaining source material from multiple platforms remains valuable for cross-tool validation.
  - Planned denigma support: repackaging `.enigmaxml` back into a Finale-readable bundle to allow source edits in Finale before re-exporting.

### Official MNX W3 examples (external)
Official reference examples from the [MNX W3 project](https://github.com/w3c/mnx)
are no longer vendored in this directory.

- File format: `.json`
- Origin: MNX Working Group (W3)
- Source in tests:
  - Loaded at runtime via `MNX_W3C_EXAMPLES_PATH` exported by the `mnxdom` CMake target.
- Maintenance:
  - Update by refreshing the fetched W3C source used by `mnxdom`.

### `mscx_reference_examples/`
Reference MuseScore files corresponding to the official MNX W3 examples.

- File format: MuseScore uncompressed score (`.mscx`).
- Purpose:
  - Serve as expected-output reference files for MNX import into MuseScore.
  - Enable visual and behavioral comparison between imported MNX content and
    known-good MuseScore scores.
- Maintenance:
  - These files are expected to be refreshed alongside updates to the W3C examples loaded via `MNX_W3C_EXAMPLES_PATH`.
  - Coverage may be partial and is expected to grow over time.

## Update Policy

- Custom `.mnx` test cases evolve alongside project development.
- The fetched W3C examples used by `mnxdom` should be updated when the MNX schema or official examples change.
- `mscx_reference_examples/` should be refreshed alongside those W3C example updates.
- `project_examples/` should be refreshed when schema changes require regenerating project MNX/MSCX pairs.
- `project_sources/` are retained to ensure reproducibility of MNX files used for project tests.

## Schema Stability Warning

MNX is an evolving specification with no finalized release date.

As a result:
- Some test files in this directory may not validate against the latest MNX schema.
- Reference examples and project-specific test cases may temporarily diverge in structure or semantics.
- Corresponding MSCX reference files may lag behind MNX changes or reflect importer/exporter limitations rather than schema intent.
- Failing tests may reflect schema evolution rather than regressions in importer/exporter logic.

When updating the MNX schema, the fetched W3C examples (via `MNX_W3C_EXAMPLES_PATH`)
and `mscx_reference_examples/` should be reviewed and refreshed as needed.

## Notes

- File extensions are intentional:
  - `.mnx` — project-specific MNX test data
  - `.json` — official MNX reference examples loaded externally via `MNX_W3C_EXAMPLES_PATH`
  - `.mscx` — MuseScore reference files
  - `.enigmaxml` — Finale-derived sources exported by denigma
- All `.mnx` files in this directory are JSON-based.
- MSCX files, when present, serve as authoritative expected-output references for MNX import into MuseScore.


### Hand-Edited Test-Case Files
All located in `project_examples/`.

- `altoFluteTremMissingKey.mnx`: Removed `key` node to test that importer correctly imports transposed key signatures when no first key signature is present.
- `barlineTypesWithShort.mnx`: Modified a dashed barline to "dotted" and a tick barline to "short" to test full range of barline type in mnx.
- `clarinet38MissingTime.mnx`: Removed `time` node to test that importer handles missing time signature gracefully.
- `key56Wrapped56Edited.mnx` : Hand-edited to test keyFifthsWrapAt values +/-5 and +/-6.
- `layoutBrackets.mnx`: Minimal layout with nested group brackets to exercise layout import/export bracket handling.
- `layoutBarlineStylesInstrument.mnx`: Hand-authored SATB mensurstrich choir layout (no bracket symbol) with separate piano accompaniment, to verify mensurstrich handling without bracket coupling.
- `layoutBarlineStylesNested.mnx`: Hand-authored nested layout with conflicting group barline styles (`individual` outer, `unified` middle, `mensurstrich` inner) to verify nested export behavior.
- `percussionKit.mnx`: Hand-authored percussion kit/kitNotes example (with sounds + ties) to exercise MNX percussion round-trip.
- `restPosition.mnx`: export from MuseScore with "Export rest positions" enabled.
