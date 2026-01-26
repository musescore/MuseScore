# Test Data Directory

This directory contains all data files used for testing MNX import/export functionality for MuseScore. The contents are intentionally split by origin and purpose.

## Directory Structure

### `*.mnx`
These are MNX test cases developed specifically for this project.

- File format: JSON
- Purpose: Exercise and validate importer/exporter behavior beyond the official MNX examples.
- Notes:
  - The `.mnx` extension is used by convention, pending a standardized MNX container format.
  - Some test cases are hand-edited versions from the exported version of the source. Their filenames indicate how they have been edited. (See below for a list.)

### `finale_sources/`
This directory contains source files exported from Finale and used to generate MNX test cases.

- File format: `.musx`
- Origin: Finale
- Conversion:
  - These files may be converted to MNX using the **denigma** command-line tool as an aid when updating test cases, particularly across significant schema changes.
  - Denigma may be downloaded or built from its [GitHub repository](https://github.com/rpatters1/denigma).
  - Over time, some or all of these may be supplemented or replaced with MuseScore source files, although maintaining source material from multiple platforms is also valuable for cross-tool validation.

### `mnx_reference_examples/`
This directory contains reference examples from the official [MNX W3 project](https://github.com/w3c/mnx).

- File format: `.json`
- Origin: MNX Working Group (W3)
- Purpose:
  - Serve as canonical reference material for schema validation and behavioral comparison.
- Maintenance:
  - These files are expected to be refreshed whenever the MNX schema is updated.

### `mscx_reference_examples/`
Reference MuseScore files corresponding to the official MNX W3 examples.

- File format: MuseScore uncompressed score (`.mscx`).
- Purpose:
  - Serve as expected-output reference files for MNX import into MuseScore.
  - Enable visual and behavioral comparison between imported MNX content and
    known-good MuseScore scores.
- Maintenance:
  - These files are expected to be refreshed alongside `mnx_reference_examples/`.
  - Coverage may be partial and is expected to grow over time.

## Update Policy

- Custom `.mnx` test cases evolve alongside project development.
- `mnx_reference_examples/` should be replaced wholesale when the MNX schema or official examples are updated.
- `mscx_reference_examples/` should be refreshed alongside `mnx_reference_examples/`.
- `finale_sources/` are retained to ensure reproducibility of generated MNX files.

## Schema Stability Warning

MNX is an evolving specification with no finalized release date.

As a result:
- Some test files in this directory may not validate against the latest MNX schema.
- Reference examples and project-specific test cases may temporarily diverge in structure or semantics.
- Corresponding MSCX reference files may lag behind MNX changes or reflect importer/exporter limitations rather than schema intent.
- Failing tests may reflect schema evolution rather than regressions in importer/exporter logic.

When updating the MNX schema, the contents of this directory—especially
`mnx_reference_examples/` and `mscx_reference_examples/`—should be reviewed and refreshed as needed.

## Notes

- File extensions are intentional:
  - `.mnx` — project-specific MNX test data
  - `.json` — official MNX reference examples
  - `.mscx` — MuseScore reference files
  - `.musx` — Finale source files
- All MNX files in this directory are JSON-based, regardless of file extension.
- MSCX files, when present, serve as authoritative expected-output references for MNX import into MuseScore.


### Hand-Edited Test-Case Files

- `altoFluteTremMissingKey.mnx`: Removed `key` node to test that importer correctly imports transposed key signatures when no first key signature is present.
- `barlineTypesWithShort.mnx`: Modified a dashed barline to "dotted" and a tick barline to "short" to test full range of barline type in mnx.
- `clarinet38MissingTime.mnx`: Removed `time` node to test that importer handles missing time signature gracefully.
- `key56Wrapped56Edited.mnx` : Hand-edited to test keyFifthsWrapAt values +/-5 and +/-6.
- `layoutBrackets.mnx`: Minimal layout with nested group brackets to exercise layout import/export bracket handling.
- `percussionKit.mnx`: Hand-authored percussion kit/kitNotes example (with sounds + ties) to exercise MNX percussion round-trip.
