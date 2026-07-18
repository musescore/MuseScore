# Encore (.enc) binary format

Binary format reference for Encore `.enc` files, independent of any implementation.
First documented by Felipe Castro (enc2ly) and Leon Vinken (Enc2MusicXML, GPL v3+),
extended by reverse engineering across a large corpus of files spanning every version.

This document describes only the bytes in a `.enc` file: what is stored, where, and what it
means. How a parser turns those bytes into a score (gap filling, tuplet reconstruction, voice
remapping, spanner-endpoint resolution, and similar heuristics) is documented separately in
[ENCORE_IMPORTER.md](ENCORE_IMPORTER.md).

## Provenance tags

Every non-trivial fact carries one of three tags:

| Tag          | Meaning                                                                       |
|--------------|-------------------------------------------------------------------------------|
| `[verified]` | Round-tripped against Encore itself, or confirmed by a binary diff of two saves that differ in one attribute. |
| `[observed]` | Consistent across many corpus files but not proven against Encore.            |
| `[assumed]`  | Plausible and consistent with the data, but neither round-tripped nor diffed. |

Untagged statements are structural facts read directly from the byte stream (block framing,
element headers, field offsets a parser depends on to advance).

---

## Overview

### File anatomy

A file is a fixed header followed by a sequence of length-prefixed blocks. Each block begins with
a 4-byte ASCII magic and a 4-byte size (`varsize`), then `varsize` bytes of content.

| Block         | Count      | Contents                                             |
|---------------|------------|------------------------------------------------------|
| header        | 1          | version, counts, global staff size (fixed layout, no magic) |
| `TK00`..`TKnn`| 0..n       | one per instrument: name, MIDI program, Key transposition |
| `PAGE`        | 0..n       | page geometry (not decoded)                          |
| `LINE`        | 1 per system | staves, clef/key/size per staff                    |
| `MEAS`        | 1 per measure | notes, rests, ornaments, and other elements       |
| `PREC`        | 0..1       | printer / page-setup state                           |
| `WINI`        | 0..1       | page margins                                         |
| `TITL`        | 1 per page | title, subtitle, author, copyright, header, footer   |
| `TEXT`        | 0..n       | text payloads referenced by staff-text ornaments     |

Blocks appear roughly in the order above, but a parser locates each one by scanning for its magic
rather than assuming a fixed order; unknown bytes between known magics are skipped.

### Magics and byte order

The first four bytes are the file magic. It selects the byte order used for every multi-byte
integer in the file (with one exception, the TK size field, noted under [Instrument block](#instrument-block)).

| Magic  | Storage   | Byte order    | Notes                        |
|--------|-----------|---------------|------------------------------|
| `SCOW` | plaintext | little-endian | Windows Encore, all versions |
| `SCO5` | plaintext | big-endian    | macOS Encore 5               |
| `SCOX` | plaintext | ,             | rare variant `[observed]`    |
| `SCOR` | plaintext | ,             | rare variant `[observed]`    |
| `SCOS` | plaintext | ,             | rare variant `[observed]`    |
| `ZBOT` | encrypted | ,             | Encore 4.x default `[observed]` |
| `ZBOP` | encrypted | ,             | encrypted variant `[observed]` |
| `ZBO6` | encrypted | ,             | encrypted variant `[observed]` |

**Encryption.** In a `ZBOT` / `ZBOP` / `ZBO6` container only the first 42 bytes decrypt with a
known XOR key; beyond that the stream is algorithmically generated and unbroken `[observed]`. The
plaintext `SCOW` form is structurally different, so these files cannot be read without re-saving
from Encore as `SCOW`.

### Version byte and release mapping

The byte at file offset `0x04` (present only in the plaintext containers) is the format version.

| Byte   | Format | Encore release (app version at header `0x28`)                  | Tag          |
|--------|--------|----------------------------------------------------------------|--------------|
| `0xA6` | v0xA6  | Encore 2.x only (app 592)                                      | `[verified]` |
| `0xC2` | v0xC2  | Encore 3.x (app 773) and Encore 4.0-4.2 (app 775)              | `[verified]` |
| `0xC4` | v0xC4  | Encore 4.5 through 5.x (app 1056)                              | `[verified]` |

Notes on the mapping:

- A genuine Windows Encore 3.0 save already uses `SCOW` with version byte `0xC2` and app version
  773, so `0xA6` is Encore 2.x only `[verified]`. Encore 4.5 opens a v0xA6 file with a font /
  conversion prompt and re-saves it as v0xC4, so no 4.5 or later build produces v0xA6.
- v0xC4 spans Encore 4.5 through 5.x: both write version byte `0xC4` and app version 1056, so
  neither field alone identifies the release. The byte at header offset `0x3E` is a
  format-revision counter that does, constant for a given Encore build regardless of score
  content: `0` on early pre-4.5 files, `1` on Encore 4.5, `4` on Encore 5.0 `[verified]`. Encore
  4.5 refuses a file whose `0x3E` revision is newer than it supports, which is why a 5.0 file
  fails to open in 4.5. Dated saves corroborate the split: `0x3E=1` files run 1999-2008, `0x3E=4`
  files never appear before 2009 `[observed]`.

`SCO5` files carry no version byte at `0x04` (the byte order is big-endian and `chuMagio` is not
`0xC4`); they are the macOS variant of the v0xC4 format and are recognised by the `SCO5` magic
alone.

### Conventions

- **Element offsets.** Every element field offset in this document is measured from the element
  start: `+0` is the first tick byte, so the type/voice byte is `+2`, the size byte `+3`, the
  staff byte `+4`, and the element body begins at `+5`. This holds for all versions and all
  element types, including the v0xA6 compact ornament (whose fields are given as element-relative
  offsets even where the code computes them from the type/voice byte).
- **Ticks.** 240 ticks per quarter note; see [Rhythm encoding](#rhythm-encoding).
- **Text.** Text-bearing fields are Latin-1 or UTF-16 LE, chosen per field by a probe defined
  once under [Encoding probe](#encoding-probe).
- **varsize.** The 4-byte size after each block magic gives the content length, excluding the
  8-byte magic + size header (except in the TK "total-size" variant, noted in place).

---

## Header

Fixed-layout region at the start of the file (no magic of its own). Ends at offset `0xC2` (194
bytes) in v0xC2 / v0xC4 / SCO5, and at offset `0xA6` (166 bytes) in v0xA6 `[verified]`; the first
block begins immediately after.

| Offset | Size | Description                                                                |
|--------|------|----------------------------------------------------------------------------|
| `0x00` | 4    | magic (`SCOW`, `SCO5`, ...)                                                 |
| `0x04` | 1    | format version byte (see [Version byte](#version-byte-and-release-mapping); absent in `SCO5`) |
| `0x28` | 2    | Encore app version: 592 (v0xA6), 773 / 775 (v0xC2), 1056 (v0xC4)           |
| `0x2A` | 2    | purpose unconfirmed (possibly total LINE-staff-entry count) `[assumed]`    |
| `0x2C` | 2    | default beat ticks (240 = quarter-note grid); matches MEAS `+0x04`         |
| `0x2E` | 2    | number of system (LINE) blocks                                             |
| `0x30` | 2    | number of pages                                                            |
| `0x32` | 1    | number of instrument (TK) blocks                                           |
| `0x33` | 1    | staves per system                                                          |
| `0x34` | 2    | rendered measure count (see below)                                         |
| `0x3E` | 1    | v0xC4 format revision: 0 = pre-4.5, 1 = 4.5, 4 = 5.0 `[verified]`          |
| `0x52` | 1    | global staff-size selector (v0xC2 / v0xC4), 1-4                            |
| `0x8D` | 1    | global staff-size selector (v0xA6), 1-4                                    |

All other bytes up to the header end are padding.

**Rendered measure count (`0x34`).** The number of measures Encore displays. A file can contain
extra "ghost" MEAS blocks left over from prior edits; only the first `0x34` MEAS blocks are real
(one observed file rendered 36 measures but held 56 MEAS blocks) `[observed]`.

**Global staff-size selector.** A value 1-4 selecting one of four staff scales `[observed]`:

| Selector | Scale |
|----------|-------|
| 1        | 60%   |
| 2        | 75%   |
| 3        | 100%  |
| 4        | 130%  |

In v0xC2 / v0xC4 the selector lives at `0x52` and is only a global fallback; the authoritative
per-staff size is the LINE staff-entry byte `+13` (see [System block](#system-block-line)). In
Encore 4.x the `0x52` field is unrelated to size. In v0xA6 the selector lives at `0x8D` (byte
`0x52` is unrelated there) and applies to every staff, because v0xA6 has no per-staff LINE size.

---

## Instrument block

Magic `TK00`, `TK01`, ... one per instrument. Carries the instrument name, MIDI program, and Key
transposition.

**Name.** Latin-1 or UTF-16 LE, chosen by the [Encoding probe](#encoding-probe), null-terminated
within the block.

**Size-field byte order.** The 4-byte size field of a TK block is little-endian even in `SCO5`
files, whose other multi-byte fields are big-endian. A TK size of 112 is stored as `70 00 00 00`;
read big-endian that is `0x70000000`, whose low 16 bits are 0, which would bound every name length
to zero. Read this field little-endian (low 16 bits) regardless of the file byte order.

### MIDI program and Key by layout

The layout of the MIDI-program and Key-transposition fields depends on the TK block size
(`varsize`). `Key` is a signed `int8` in semitones matching Encore's Staff Sheet "Key" dropdown
(0 = sounds as written, -12 = octave lower, range about -33..+24); Encore shifts every note pitch
by this value at playback. MIDI program is a 1-indexed General MIDI number.

| Layout                     | Detection                                | MIDI program offset            | Key offset                    |
|----------------------------|------------------------------------------|--------------------------------|-------------------------------|
| Large-TK                   | `varsize > 250`                          | `2278 + n*2158` (fixed table)  | `2278 - 23 + n*2158`          |
| Small-TK, Encore 5.x       | `0 < varsize <= 250`, stride = `varsize+8` | `contentStart + varsize + 76` | `contentStart + varsize + 53` |
| Small-TK, Encore 4.x total | `0 < varsize <= 250`, stride = `varsize`   | `contentStart + 60`           | `contentStart + 42`           |
| v0xA6                      | version byte `0xA6`, `varsize = 64`      | `contentStart + 52`            | `contentStart + 42`           |

Where `contentStart = TK_block_start + 8` (past the 8-byte magic + size) and `n` is the
instrument's sequential index.

- In the **large-TK** layout the fixed table sits after the TK blocks (`base = 2278`,
  `step = 2158`).
- The **small-TK Encore 4.x total-size** variant stores the total block size (including the
  8-byte header) in `varsize`, so the block stride equals `varsize` rather than `varsize + 8`;
  actual content is `varsize - 8` bytes. Its MIDI/Key offsets match the v0xA6 layout `[verified]`.
- In **v0xA6** each TK block is 64 bytes total (56-byte content). The MIDI byte is at content
  `+52`, NOT `+60` as in the 4.x total-size variant `[verified]`. The Key byte at content `+42`
  is octave-only in practice (0, plus or minus 12/24).

### No-TK-block files

Some v0xC4 files and many v0xC2 files carry no TK blocks. The instrument metadata then lives in a
positional table; three sub-layouts occur, chosen by where the first PAGE/LINE/MEAS block starts
and whether a `~~~~` (`0x7E7E7E7E`) compact-table marker is present in the first 1 KiB.

| Sub-layout             | Detection                          | Name at         | MIDI at          | Key at         |
|------------------------|------------------------------------|-----------------|------------------|----------------|
| Compact v0xC4          | first block <= 2278, no `~~~~`     | `202 + n*112`   | `390 + n*276`    | `367` (1 instr)|
| Large-TK (no TK blocks)| first block > 2278                 | `202 + n*2158`  | `2278 + n*2158`  | `2255 + n*2158`|
| Compact v0xC2 variant A| `~~~~` present                     | `314 + k*112`   | `374 + k*112`    | ,              |
| Compact v0xC2 variant B| no `~~~~`, first block <= 2278     | `202 + n*112`   | `262 + n*112`    | ,              |

- In **variant A** (`~~~~` present) the entry table starts at 281 (name field `+33`, MIDI field
  `+93`). Some instruments also have an explicit "primary block" at `202 + n*2158` (printable
  ASCII at that offset marks it); such an instrument's MIDI is at `202 + n*2158 + 60`.
- In **variant B** (no `~~~~`) the entry table starts at 176 (name field `+26` giving base 202,
  MIDI field `+86` giving base 262). All instruments are in this linear table.
- The name offset for instrument 0 is 202 in every compact layout; the step between consecutive
  names is 2158 when the first block is beyond 2278, otherwise 112.

An instrument that has its own TK block whose name field is empty is genuinely unnamed; the
positional offsets must not be probed for it (the formula offset `202 + n*2158` would land on
unrelated music bytes).

**Percussion program.** Percussion tracks always report MIDI program 1 (General MIDI Grand
Piano) `[observed]`; the actual kit is not stored and can only be inferred from the track name. A
MIDI value of 113 or above (the General MIDI percussive range) indicates a drum instrument.

---

## System block (LINE)

Magic `LINE`, one per system. A 21-byte header followed by N staff entries of 30 bytes each, where
N is the header's staves-per-system.

### LINE header (21 bytes)

| Offset | Size | Description                                        |
|--------|------|----------------------------------------------------|
| `+10`  | 2    | 0-based index of the first measure in this system  |
| `+12`  | 1    | number of measures in this system                  |

In `SCO5` (big-endian) the single-byte measure count reads as 0 because the meaningful byte sits
in the other half of a wider big-endian field, but the first-measure index stays correct.

### LINE staff entry (30 bytes)

| Offset | Size | Description                                                             |
|--------|------|-------------------------------------------------------------------------|
| `+13`  | 1    | staff display size, 0-indexed selector (see below)                      |
| `+14`  | 1    | clef type (see [Clef types](#clef-element))                             |
| `+15`  | 1    | key signature (see [Key encoding](#key-encoding))                       |
| `+16`  | 1    | page-row counter for this system (NOT the page number)                  |
| `+19`  | 1    | visibility: `0x00` = hidden, non-zero = visible                         |
| `+20`  | 1    | staff type: 0 = melody, 1 = tablature, 2 = single-line percussion       |
| `+21`  | 1    | packed staff index: bits 0-5 = instrument, bits 6-7 = staff within instrument |

The display-size selector at `+13` is 0-indexed (0 = 60%, 1 = 70%, 2 = 75%, 3 = 100%)
`[observed]`; it is populated in both 4.x and 5.x files and is the authoritative per-staff size,
with header `0x52` as a global fallback. The packed staff index at `+21` uses the same encoding as
the element [Staff byte](#staff-byte-encoding). The staff type at `+20` is constant across all
LINE blocks for the same staff position.

### v0xA6 staff size and clef

v0xA6 reports staves-per-system as 0 and uses a 22-byte staff entry (not the 30-byte v0xC2/C4
entry). Each entry carries a `0x0E 0xFC` marker at offset 16 that bounds the run of entries.
Consequences:

- **Key signature** is at offset `+14` of each 22-byte entry (same index encoding as the v0xC2/C4
  key field) `[observed]`. It is not where v0xC2/C4 keep it, so it must be read from these entries
  directly.
- **Staff size** is the single global value at header `0x8D`, applied to every staff.
- **Clef** has no located per-staff field in v0xA6 `[assumed]`. Encore renders the correct clefs
  from these files, so the information is available to Encore, but the byte was not found in TK
  content, the 22-byte LINE entries, MEAS blocks (no initial clef elements), or PAGE blocks. The
  nearest candidate, TK content `+49`, reads 7 on a snare staff but 0 on a bass staff Encore still
  shows in F, so it is more likely a staff-type field than a clef. Whether the clef is stored
  elsewhere or derived from the instrument default is unproven; one file whose "Bajo" staff (MIDI
  program 59) opens as Tuba and shows bass clef (the Tuba default) is consistent with the
  instrument-derived hypothesis but does not prove it. A definitive test needs an Encore build
  that writes v0xA6 (Encore 2.x), which was unavailable.

---

## PAGE block

Magic `PAGE`. Page geometry. Not decoded; a parser skips it. Page count comes from header `0x30`,
and page size / orientation / scale from the [PREC block](#prec-block).

### Tab tuning (per track, at the end of each TK block)

Each track carries its own tab tuning in the eight bytes at the end of its `TK` block's content,
immediately before the next block's eight-byte header (the block's declared size counts those eight
trailing header bytes, so the tuning sits sixteen bytes before the block's nominal end). For the last
track this places the tuning in the eight bytes just before the first `PAGE` block. Those eight slots
hold the open-string MIDI pitches from lowest string to highest, followed by pad bytes: `0x7F` when
the tuning has been customised and `0x58` for the untouched 6-string guitar default. The string count
is the number of leading non-pad slots, so a 4-string mandolin reads `37 3E 45 4C 7F 7F 7F 7F`
(G3 D4 A4 E5) and a 6-string guitar reads `34 39 3E 43 47 4C 7F 7F` (the tab-display pitches, an octave
above concert).

Because the tuning is per track, a file that mixes differently tuned tablature staves (for example a
bandurria tab and a guitar tab) carries a distinct tuning in each track's block; each tab staff must
use the tuning from its own `TK` block. A near-identical block also appears once in the SCO5 header
(around offset `0x1A1`, always the 6-string guitar default padded with `0x58`); that copy is a global
default and is not a per-staff tuning.

Encore stores no per-note string or fret, only this tuning; the fingering is computed from the note
pitches.

A tablature staff is normally a derived view: its notes live on the paired notation staff and the
tab staff's own element stream carries only rests. The exception is a tab-only score (no notation
staff at all): there Encore materializes the tab staff's notes as pitch-bearing REST elements. Such
an element uses the REST byte layout (type nibble `8`) but sets bit `0x8` of the voice nibble (byte
`0x88`) and stores the MIDI pitch at element offset `+15`, the same slot a NOTE uses. It has no face
value; the duration is implied by the tick gaps to the next element. A genuine rest has a voice
nibble below `4` and no pitch, so the `0x8` voice bit distinguishes the two.

---

## Measure block (MEAS)

Magic `MEAS`. A fixed header followed by a variable element stream terminated by a `0xFFFF` tick.
The header is 54 bytes (`0x36`) in v0xC2 / v0xC4 / SCO5 and 26 bytes (`0x1A`) in v0xA6; the element
stream begins right after `[verified]`.

### Measure header

| Offset | Size | Description                                                    |
|--------|------|----------------------------------------------------------------|
| `0x00` | 2    | quarter-note BPM; applies forward until the next change        |
| `0x02` | 1    | time-signature glyph (see below)                               |
| `0x04` | 2    | beat ticks (standard 240 for x/4)                              |
| `0x06` | 2    | total ticks in the measure                                     |
| `0x08` | 1    | time-signature numerator                                       |
| `0x09` | 1    | time-signature denominator                                     |
| `0x0C` | 1    | start barline type (see below)                                 |
| `0x0D` | 1    | end barline type (same table)                                  |
| `0x0F` | 1    | volta (repeat-alternative) bitmask (see below)                 |
| `0x1A` | 4    | repeat-mark field; low byte = repeat type (see below)          |

Bytes `0x10`..`0x35` outside the fields above hold layout data (measure width, x-offsets, a
"Writer" UTF-16 tag). An unrelated layout field at `+0x18` holds a constant 200 in v0xC4 files;
it is not a BPM.

**Beat ticks vs whole-note ticks.** Standard beat-ticks are 240 for x/4, 120 for x/8, 480 for
x/2, and 360 for a compound dotted-quarter beat. Some builds store 240 even for 2/2 (correct value
480), so the whole-note tick count must not be computed from beat-ticks; see
[Rhythm encoding](#rhythm-encoding).

**BPM** is always quarter-note BPM regardless of meter. In 3/8, 5/8, etc. Encore's UI shows
eighth-note BPM (twice the stored value) but the file stores quarter-note BPM.

#### Time-signature glyph

| Value          | Meaning                                                                    |
|----------------|----------------------------------------------------------------------------|
| `0x00`         | numeric display; show numerator / denominator digits                       |
| `0x43`         | common time "C" (numerator 4, denominator 4); Encore 3.x / 4.x             |
| `0x63`         | common time "C" (numerator 4, denominator 4); Encore 5.x                   |

Other values (`0x01`, `0x02`, `0x06`, `0x07`) appear on unusual meter strings `[observed]`; treat
as numeric display.

#### Barline types

| Value | Meaning        |
|-------|----------------|
| 0     | normal         |
| 2     | repeat start   |
| 3     | double (left)  |
| 4     | repeat end     |
| 5     | final          |
| 6     | double (right) |
| 8     | dotted         |

A non-repeat divider (double or dotted) that visually sits between two measures is stored on the
right measure's start barline (`0x0C`), not the left measure's end barline (`0x0D`). A start
barline of type 3/6/8 therefore describes the divider at the end of the preceding measure; a
repeat-start (type 2) at `0x0C` genuinely belongs to its own measure.

#### Volta bitmask

Byte `0x0F` is a bitmask: bit `n` set means the measure belongs to ending `n+1` (bit 0 = ending 1,
bit 1 = ending 2, ...). Encore sets the same bitmask on every measure inside an ending, not only
the first. Consecutive measures with the same non-zero bitmask form one multi-ending bracket.

Encore stores no explicit "times played" count for a repeat-end barline. The number of passes is
implied by the endings: it is the highest ending number among the brackets in the repeat. A first
ending of "1.-3." (bits 0+1+2 = `0x07`) followed by a "4." ending (bit 3) plays four times. A
repeat with no alternate endings plays twice.

#### Repeat-mark ladder (low byte of `0x1A`)

| Byte   | Meaning                                                       |
|--------|---------------------------------------------------------------|
| `0x80` | D.C. al Coda                                                  |
| `0x81` | D.S. al Coda                                                  |
| `0x82` | D.C. al Fine                                                  |
| `0x83` | D.S. al Fine                                                  |
| `0x84` | D.S.                                                          |
| `0x85` | "To Coda" source (player jumps from here)                     |
| `0x86` | Fine                                                          |
| `0x87` | D.C.                                                          |
| `0x88` | Segno                                                         |
| `0x89` | Coda destination (player jumps to here)                       |

`0x85` and `0x89` are a pair: `0x85` is the To-Coda navigation point, `0x89` the Coda target.
Ornament subtype `0xA5` is the parallel encoding of the same "To Coda" navigation point.

---

## Element stream

Every element begins with a fixed 5-byte header.

| Offset | Size | Field                                                              |
|--------|------|--------------------------------------------------------------------|
| `+0`   | 2    | tick within the measure; `0xFFFF` terminates the stream            |
| `+2`   | 1    | type/voice byte: high nibble = type, low nibble = voice            |
| `+3`   | 1    | size (total element length in bytes, from `+0`)                    |
| `+4`   | 1    | staff byte (see below)                                             |

The element body begins at `+5`. In v0xA6 the on-disk element slot is twice the declared size:
stride = `size * 2` `[verified]`.

### Element types

| Type | Name                       |
|------|----------------------------|
| 0    | none                       |
| 1    | clef                       |
| 2    | key change                 |
| 3    | tie                        |
| 4    | beam                       |
| 5    | ornament                   |
| 6    | lyric                      |
| 7    | chord symbol (harmony text)|
| 8    | rest                       |
| 9    | note                       |
| 11   | MIDI control change        |

Type 7 is a chord SYMBOL (harmony text above the staff), not a note chord.

### Staff byte encoding

Byte `+4` names the destination staff, using the same encoding as the LINE packed staff index.

| Bits | Mask   | Meaning                                                              |
|------|--------|----------------------------------------------------------------------|
| 0-5  | `0x3F` | instrument index (0-based, sequential per instrument)                |
| 6-7  | `0xC0` | staff within the instrument (0 = first, 1 = second, 2/3 = further)   |

The raw byte (staff-within bits in 6-7, instrument index in 0-5) equals the packed staff index the
LINE block stores for the target staff, and is resolved to a global staff by inverse lookup through
the LINE staff indices. The instrument index in bits 0-5 equals the LINE slot only when every
instrument has one staff; for multi-staff instruments it does not (see [Multi-staff instruments](#multi-staff-instruments)).

### Voice field

The low nibble of the type/voice byte is the voice, 0-7. Beyond the four ordinary voices two
special uses occur:

- **voice 4** is a marker with two meanings resolved by the LINE multi-staff configuration. On a
  grand-staff instrument it marks second-staff content and is combined with the `0x40` bit of the
  staff byte (so the staff-within bits read 1). On a single-staff instrument it is a genuine second
  melodic voice, or, in an SATB part whose only line is stored as voice 4, the sole voice.
- **voices 5, 6, 7** are additional voices on the element's own staff (they do not indicate a
  second staff).

A voice may carry more than one interleaved MIDI tick stream (for example from live recording); a
backwards tick, or a non-chord event arriving after the voice is full, marks a fresh stream. A
voice may also carry a redundant plain (non-tuplet) rest at the same tick as a real note (Encore
writes a rest slot for the voice even where the note sits); this differs from same-tick tuplet
members, which are genuine sequential members.

### MIDI control change (type 11)

Inline MIDI Control Change events, stored for playback only; they carry no notation. Always 12
bytes.

| Offset | Size | Description                                             |
|--------|------|---------------------------------------------------------|
| `+0`   | 2    | tick                                                    |
| `+2`   | 1    | type/voice = `0xBn`                                     |
| `+3`   | 1    | size = 12                                               |
| `+4`   | 1    | MIDI channel / track index                              |
| `+5`   | 1    | CC event marker (`0xB0` = channel-0 control change)     |
| `+6`   | 4    | zeros                                                   |
| `+10`  | 1    | controller number (64 = sustain, 7 = volume, 1 = modulation) |
| `+11`  | 1    | value (127 = max/on, 0 = off)                           |

### Implicit silences

Encore does not always emit explicit rest elements. A gap between two consecutive same-voice
events (the next tick exceeds the previous tick plus the previous note's face-value duration)
represents a silence the user wrote as a rest.

---

## Note element

The note carries a face value (duration and notehead), a MIDI pitch, grace/tie flags, an optional
tuplet ratio, and optional articulations. Layout differs by version.

**Face value (`+5`).** High nibble = notehead type (0 = normal, 1 = diamond, 2 = triangle-up,
3 = square, 4 = cross, 5 = X-in-circle, 6 = plus, 7 = slash, 8 = large open diamond, 9 = invisible).
Low nibble = duration (1 = whole, 2 = half, 3 = quarter, 4 = eighth, ... 8 = 128th).

**Byte `+14` (`dotControl`).** In every version this is a layout/display byte, not a dot count:
bit 0 is an unreliable "dotted" hint that also appears set on plain notes (values such as `0x28`,
`0x30`, `0x39`, `0x60` on plain 8ths/16ths) `[observed]`. The reliable dot source is the sounding
duration relative to the face value; see [Rhythm encoding](#rhythm-encoding).

### v0xC4 note (size 28)

| Offset | Size | Description                                                             |
|--------|------|-------------------------------------------------------------------------|
| `+5`   | 1    | face value                                                              |
| `+6`   | 1    | grace1 (grace/cue flags, see below)                                     |
| `+7`   | 1    | grace2                                                                  |
| `+10`  | 1    | xoffset (notated horizontal column; see [Chord column](#chord-column-xoffset)) |
| `+12`  | 1    | staff-relative pitch, diatonic steps from C4 (display hint; see below)  |
| `+13`  | 1    | tuplet byte: high nibble = actual count, low nibble = normal count      |
| `+14`  | 1    | dotControl (see above)                                                  |
| `+15`  | 1    | MIDI pitch (0-127)                                                      |
| `+16`  | 2    | playback duration in ticks (recorded MIDI; diverges from notated for tuplets/ties) |
| `+19`  | 1    | velocity                                                                |
| `+20`  | 1    | options                                                                 |
| `+21`  | 1    | accidental / alteration glyph                                           |
| `+24`  | 1    | articulation byte (above slot)                                          |
| `+26`  | 1    | articulation byte (below slot)                                          |

The staff-relative pitch at `+12` counts diatonic steps from C4 (C4 = 0, D4 = 1, ... A5 = 12). On
pitched staves it is a legacy display hint. On a percussion-clef staff it instead encodes the
visual staff line: `line = max(-4, 10 - position)`, placing A4 on the middle line `[observed]`.

### v0xC2 note (size 22 or 24)

More compact than v0xC4. Two pitch-storage sub-variants exist, distinguished by whether `+15`
holds a plausible MIDI pitch (at least C0, MIDI 12):

- **Sub-variant A:** `+15` is empty or a small stray flag (observed 1 or 3). MIDI pitch is at
  `+13` (the slot v0xC4 uses for the tuplet byte); there is no explicit tuplet byte.
- **Sub-variant B:** `+15` holds a plausible pitch (the standard slot). `+13` then holds a genuine
  tuplet ratio (for example `0x32` = 3:2).

A value below C0 at `+15` cannot be a real note, so the pitch comes from `+13` in that case; a bare
"non-zero at `+15`" test is wrong in both directions.

| Offset | Size | Description                                                       |
|--------|------|-------------------------------------------------------------------|
| `+5`   | 1    | face value                                                        |
| `+6`   | 1    | grace1                                                            |
| `+7`   | 1    | grace2                                                            |
| `+10`  | 1    | xoffset                                                           |
| `+13`  | 1    | MIDI pitch (sub-variant A) or tuplet ratio (sub-variant B)        |
| `+14`  | 1    | dotControl                                                        |
| `+15`  | 1    | MIDI pitch (sub-variant B); stray flag in sub-variant A           |
| `+16`  | 2    | playback duration in ticks                                        |
| `+19`  | 1    | velocity                                                          |
| `+20`  | 1    | options                                                           |
| `+21`  | 1    | alteration glyph                                                  |
| `+22`  | 1    | articulation byte (size 24 only)                                  |
| `+23`  | 1    | placement/direction flag, size 24 only (`0x01` or `0x08`; not a second articulation) |

Size 22 notes carry no articulation slot; size 24 notes add one at `+22`. A `dotControl` of `0xC0`
is characteristic of size-24 notes (a layout flag; bit 0 clear, so not dotted).

In v0xC2, when a grace note is a tie sender its `grace1` low nibble is 1 (`grace1 & 0x0F == 1`); in
v0xA6 and v0xC4 that nibble is always 0.

### v0xA6 note (size 10, on-disk slot 20)

| Offset | Description                                              |
|--------|----------------------------------------------------------|
| `+5`   | face value                                               |
| `+6`   | grace1 (see below)                                       |
| `+7`   | tuplet byte (3:2 = `0x32`, 5:4 = `0x54`, ...)            |
| `+9`   | staff-position / diatonic line, NOT the MIDI pitch       |
| `+11`  | MIDI pitch (absolute 0-127)                              |

A v0xA6 note that carries one articulation is written as size 11 (a 22-byte slot); the layout is
otherwise identical (pitch at `+11`, tuplet at `+7`) with the single articulation byte at `+18`
(`0x20` there is a fermata above) `[verified]`.

### Grace and cue notes

The grace/cue flags come from Encore's "Grace / Cue Note" dialog `[verified]`.

| Bit             | Meaning                                                                     |
|-----------------|-----------------------------------------------------------------------------|
| `grace1 & 0x20` | small note (grace or cue); an ordinary note leaves it clear                 |
| `grace1 & 0x10` | member of a beamed grace group (e.g. a percussion ruff); does not change kind |
| `grace1 & 0x40` | attribute of the top chord member; unrelated to grace/cue (also a chord-extension marker, see [Known quirks](#known-quirks)) |
| `grace2 & 0x04` | slash (acciaccatura)                                                        |
| `grace2 & 0x01` | muted (playback off); a per-note flag independent of size and kind          |

Among small notes (`grace1 & 0x20`), a slash (`grace2 & 0x04`) marks an acciaccatura; a small
no-slash note is either an appoggiatura (when it ornaments an adjacent principal note) or a cue
(a full-value small note standing alone). The two are byte-identical and distinguished only by
context. A cue is small and muted by default, but any note can be muted and a cue can be un-muted.

The played length is the playback duration at `+16`, equal to the dialog's "Scale duration by N%"
applied to the face value. A cue keeps its full beat value in the measure (a normal note drawn
small, muted by default); a grace occupies no measure time and borrows from an adjacent note.

A slur can begin on a grace note stored at the same tick as its parent chord (a grace shares its
parent's written tick), so such a slur has no distinct start tick of its own. In v0xC4 Encore
serializes the main note before its grace at the same beat; in v0xC2 the grace is written first
`[verified]`.

### v0xA6 grace note time-borrowing

In v0xA6, grace notes occupy real tick positions (they are not co-located with the main note). A
grace at a real tick pushes the following notes forward, so the last real note in the group ends
with a raw gap to the measure end that is smaller than its face value: the grace "borrowed" that
time. Inner graces (`grace1 & 0x30 == 0x10`) after a leading grace (`grace1 & 0x30 == 0x20`) have a
strictly larger face-value number (a shorter note). (The reconstruction rule that restores the
borrowed duration lives in ENCORE_IMPORTER.md.)

---

## REST element

| Offset | Size | Description                                                             |
|--------|------|-------------------------------------------------------------------------|
| `+5`   | 1    | face value (same encoding as a note)                                    |
| `+10`  | 1    | xoffset                                                                 |
| `+13`  | 1    | tuplet byte (same encoding as a note)                                   |
| `+14`  | 1    | dotControl (bitmask, not a tick count; bit 0 = unreliable dotted hint)  |
| `+15`  | 1    | multi-measure rest count (v0xC4; only when size > 15)                   |

**Multi-measure rests.** When the count at `+15` is greater than 1, the single MEAS block
represents that many consecutive empty display measures (Encore draws one rest symbol with the
count above it). Multi-staff files emit one rest element per staff, all carrying the same count.

The v0xC2 dotted-eighth timing quirk: the sixteenth of a dotted-eighth + sixteenth group has its
MIDI note-on stored at `tick + 120` (a plain eighth) rather than `tick + 180`, so the dotted eighth
reads as a plain eighth and its dotControl also lacks bit 0. The dot is not encoded in the bytes at
all in this case.

---

## Articulation bytes

Each articulation byte holds one or two glyphs. The combined range `0x22`-`0x2D` is laid out in
consecutive (below, above) pairs, one pair per glyph `[verified]`.

| Value        | Meaning                                            |
|--------------|----------------------------------------------------|
| `0x01`       | flat mark (not an articulation)                    |
| `0x02`       | sharp / natural mark                               |
| `0x03`       | 3-stroke tremolo (bare, no high-nibble flag)       |
| `0x04`       | trill (plain)                                      |
| `0x05`       | trill to minor 2nd (flat upper)                    |
| `0x06`       | trill to augmented 2nd (sharp upper)               |
| `0x07`       | trill to major 2nd (natural upper)                 |
| `0x08`       | turn                                               |
| `0x09`       | wave mark                                          |
| `0x0A`       | inverted mordent (short)                           |
| `0x0B`       | mordent (simple lower)                             |
| `0x0C`       | inverted mordent (long)                            |
| `0x0D`-`0x11`| fingering 1 to 5                                   |
| `0x12`       | accent (>)                                         |
| `0x13`       | marcato (^)                                        |
| `0x14`       | staccato + heavy accent (below)                    |
| `0x15`       | marcato + staccato                                 |
| `0x16`       | accent + staccatissimo                             |
| `0x17`       | accent + staccato                                  |
| `0x18`       | up bow                                             |
| `0x19`       | down bow                                           |
| `0x1A`       | marcato (variant)                                  |
| `0x1B`       | stopped horn / brass (+)                           |
| `0x1C`       | tenuto                                             |
| `0x1D`       | staccato                                           |
| `0x1E`, `0x1F`| harmonic                                          |
| `0x20`, `0x21`| fermata (or, on a tuplet note, tuplet-bracket placement) |
| `0x22`, `0x23`| tenuto + accent                                   |
| `0x24`, `0x25`| tenuto + staccato (portato)                       |
| `0x26`, `0x27`| marcato + tenuto (heavy accent + tenuto)          |
| `0x28`, `0x29`| staccatissimo                                     |
| `0x2A`, `0x2B`| heavy accent + staccatissimo                      |
| `0x2C`, `0x2D`| tenuto + staccatissimo                            |
| `0x2E`       | inverted turn                                      |
| `0x2F`       | mordent (double / long lower)                      |
| `0x30`       | half-stopped horn (circle-plus)                    |
| `0x39`-`0x40`| scale string number 1 to 8 (`0x38 + N` = string N) |
| `0x41`       | 1-stroke tremolo (8th)                             |
| `0x42`       | 2-stroke tremolo (16th)                            |
| `0x43`       | 3-stroke tremolo (32nd; some files render 4 strokes) |
| `0x44`, `0x45`| thumb position                                    |
| `0x46`       | open string (plain fingering "0")                  |
| `0x47`       | drumstick ("stick") technique                      |
| `0x48`       | brush                                              |
| `0x49`       | soft mallet                                        |
| `0x4A`       | hard mallet                                        |

"Heavy accent" is the wedge Encore writes as marcato. Values `0x44` and above are technical
markings, not tremolos. When at least one scale-string byte (`0x39`-`0x40`) appears in a measure,
every note in that measure with options bit 0 set displays its scale-degree position as a circled
string number.

Each note in a chord carries its own articulation bytes. When several chord notes carry the same
byte it is simply repeated on each; the chord denotes at most one copy of each distinct glyph.

---

## CLEF element

Byte `+5` holds the clef type (same encoding as the LINE staff-entry clef byte). Later bytes are
padding.

| Value | Clef       |
|-------|------------|
| 0     | G (treble) |
| 1     | F (bass)   |
| 2     | C3 (alto)  |
| 3     | C4 (tenor) |
| 4     | G 8va      |
| 5     | G 8vb      |
| 6     | F 8vb      |
| 7     | percussion |
| 8     | tablature  |

A clef element does not take effect at its own stored tick. It applies before the note or rest
that physically follows it in the stream on the same staff (Encore frequently stamps the clef with
an earlier tick). When a clef element is the last element on its staff in a measure, it is a
cautionary clef effective on the next measure's downbeat, drawn just before the current measure's
final barline.

---

## Key encoding

The key index (used by both the KEYCHANGE element and the LINE staff entry) maps to a position on
the circle of fifths:

| Index | Fifths | Key            |
|-------|--------|----------------|
| 0     | 0      | C              |
| 1-7   | -1..-7 | F, Bb, ... Cb  |
| 8-14  | +1..+7 | G, D, ... C#   |

Index 0 is a legitimate value (naturals cancel prior accidentals).

## KEYCHANGE element

Size 6. Byte `+5` is the key index above.

---

## TIE element

Size 16 or 18. Byte `+5` is a signed arc-curvature value (the vertical bow), NOT a bitfield; byte
`+6` is a tie-start flag.

| Byte `+5` | Signed | Arc        |
|-----------|--------|------------|
| `0x02`    | +2     | curve down |
| `0x04`    | +4     | curve down |
| `0xFE`    | -2     | curve up   |
| `0xFC`    | -4     | curve up   |

All four values mark a real outgoing tie. Treating `+5` as a bitfield (for example
`(+5 & 0x80) || (+5 & 0x02)`) silently drops the equally valid `0x04`.

**18-byte form.** Two additional bytes encode the visual x-positions of the arc endpoints, and
these are the authoritative forward-tie signal:

| Offset | Description                                                     |
|--------|-----------------------------------------------------------------|
| `+10`  | arc-start x (measure-relative pixels)                           |
| `+12`  | arc-end x                                                       |
| `+14`  | staff position of the source note (disambiguates chord members) |

- `arcX1 < arcX2`: a genuine left-to-right span, a real forward tie regardless of `+5`.
- `arcX1 == arcX2`: zero horizontal extent, an intra-chord decorative arc (Encore connects two
  chord notes vertically), NOT a forward tie, unless byte `+6` bit 7 is set, which marks a
  cross-measure tie whose destination lives in the next measure and for which Encore stores
  `arcX2 = arcX1` as a placeholder. Intra-chord arcs often appear in groups of 2-4 identical copies
  at the same tick.

**16-byte form.** With no arc x-positions, tie-start falls back to the byte signal
`(+5 & 0x80) || (+5 & 0x02) || (+6 & 0x80)`.

A tie element marks only the start note; there is no matching tie-stop element. The receiver is the
next note of the same pitch on the same staff and voice.

---

## Ornament element

Type 5, variable size. Covers hairpins, slurs, trills, tempo marks, dynamics, staff text, breath
marks, tremolos, fingerings, bows, and articulation-like marks (distinguished by subtype).

| Offset | Size | Description                                                             |
|--------|------|-------------------------------------------------------------------------|
| `+5`   | 1    | subtype (see [Ornament subtypes](#ornament-subtypes))                   |
| `+10`  | 1    | xoffset (start x within the measure)                                    |
| `+12`  | 2    | signed s16 Cartesian y (negative = below staff, positive = above)       |
| `+16`  | 1    | v0xC2 forward measure-count (slurs; see [Slur](#slur))                  |
| `+18`  | 1    | forward measure-count to the end measure (v0xC4 spanners)               |
| `+20`  | 1    | end x within the target measure                                         |
| `+26`  | 1    | hairpin direction, bit 0: 0 = crescendo, 1 = diminuendo                 |
| `+28`  | 1    | tempo beat unit (see below)                                             |
| `+30`  | 2    | tempo BPM (v0xC4; see below)                                            |
| `+32`  | 1    | staff-text entry index (when size >= 33; otherwise shared with `+30`)   |

The two low bits of the direction byte at `+26` are kept; see [hairpin direction](#hairpin-direction).

**Tempo beat unit (`+28`).** Low 7 bits = note value (0 = whole, 1 = half, 2 = quarter,
3 = eighth, ...), high bit `0x80` = dotted; so `0x02` is a quarter, `0x82` a dotted quarter. A
value of 0 (or an out-of-range byte from an older format) means no explicit unit.

**Tempo BPM.** In v0xC4 the BPM is at `+30`, expressed in the beat unit at `+28`. v0xC2 has two
layouts: newer files match v0xC4 (BPM at `+30`, beat-unit code 0-6 at `+28`), older files store
the BPM directly at `+28` with a constant unrelated byte (observed `0x34` = 52) in the `+30` slot
`[observed]`. Distinguish by `+28`: a valid beat-unit code means the BPM is at `+30`, otherwise
`+28` itself is the BPM. In that older layout the per-mark beat unit is at `+26` (same note-value
encoding), which matters in compound meters.

**Staff-text entry index.** Present at `+32` only when the element is at least 33 bytes. In shorter
ornaments (notably v0xC2 size-32 staff-text elements) it is read from `+30`, sharing it with the
tempo byte.

**v0xA6 compact ornament.** The v0xA6 ornament is compact (declared size 15, a 30-byte slot). Its
signed s16 y is at `+8` (not `+12`) and its staff-text entry index is at `+28` (not the size-based
slot) `[verified]`.

### Ornament subtypes

Subtype byte at `+5`. Sorted by value.

| Value        | Meaning                                                                   |
|--------------|---------------------------------------------------------------------------|
| `0x10`       | ottava 8va (line above staff)                                             |
| `0x12`       | ottava 8vb (line below staff)                                             |
| `0x1C`       | user-drawn graphic line (no musical meaning)                              |
| `0x1D`       | hairpin start; end at forward measure-count (`+18`) and end-x (`+20`)     |
| `0x1E`       | staff text (payload in the TEXT block at entry index `+32`)               |
| `0x21`       | slur start; endpoint at forward measure-count and end-x                   |
| `0x22`       | arpeggio                                                                   |
| `0x28`       | guitar bend (curved arrow up)                                             |
| `0x29`       | guitar bend (curved arrow)                                                |
| `0x2A`       | guitar prebend                                                            |
| `0x2B`       | guitar prebend-release                                                    |
| `0x30`       | guitar V-shape bend                                                       |
| `0x32`       | tempo mark (BPM at `+30` in the beat unit at `+28`)                       |
| `0x35`       | trill-span end (no visible glyph)                                         |
| `0x36`       | trill-span start (tr + wavy line)                                         |
| `0x37`       | secondary trill mark within a span (plain trill glyph)                    |
| `0x41`       | slur stop (reserved; not emitted in practice)                             |
| `0x4D`       | hairpin stop (reserved; not emitted in practice)                          |
| `0x80`-`0x87`| dynamics ppp, pp, p, mp, mf, f, ff, fff                                    |
| `0x88`       | dynamic sfz                                                               |
| `0x89`       | dynamic sffz                                                              |
| `0x8A`       | dynamic fp                                                                |
| `0xA2`       | segno                                                                     |
| `0xA3`       | "%" repeat-last-bar glyph (size 16)                                       |
| `0xA5`       | "To Coda" navigation point                                                |
| `0xA6`       | Coda glyph                                                                |
| `0xA7`       | caesura (//) after the preceding note                                     |
| `0xA8`       | comma breath mark after the preceding note                                |
| `0xAA`       | dynamic fz                                                                |
| `0xAB`       | dynamic sf                                                                |
| `0xAF`       | single-chord triple tremolo (3 slashes = 32nd); always voice 0            |
| `0xB0`       | standalone "tr" mark (size 16; plain trill, never a span)                 |
| `0xB6`       | standalone short-trill mark (size 16; never a span)                       |
| `0xB8`       | double lower mordent                                                      |
| `0xB9`-`0xBD`| standalone fingering digit 1 to 5 (size 16)                               |
| `0xBE`       | accent (>) (v0xC4; v0xC2 uses `0xC4`)                                     |
| `0xBF`       | marcato (^, vertex up)                                                     |
| `0xC0`       | marcato + staccato below                                                  |
| `0xC4`       | up-bow (v0xC4); accent (v0xC2)                                            |
| `0xC5`       | down-bow                                                                   |
| `0xC6`       | marcato below (vertex down)                                               |
| `0xC8`       | tenuto dash above                                                         |
| `0xC9`       | per-chord staccato dot                                                    |
| `0xCC`       | standalone fermata above (size 16; y > 0)                                 |
| `0xCD`       | standalone fermata below (size 16; y < 0)                                 |
| `0xE6`, `0xE7`| 1-slash tremolo (eighth speed)                                           |
| `0xE9`, `0xEA`| 4-slash tremolo (sixty-fourth speed)                                     |
| `0xEE`       | 2-slash tremolo (sixteenth speed)                                         |
| `0xEF`       | alternate triple tremolo (element at `tick == durTicks`; 32nd speed)      |

Subtypes marked "confirmed by opening the file in Encore 5" are `[verified]`; the rest are
`[observed]`. A trill-span start (`0x36`) opens a trill + wavy-line span when a `0x35` or a
non-zero forward measure-count is present, otherwise it is a plain trill glyph. `0xB0` and `0xB6`
are always standalone. The accent, up-bow and down-bow marks (`0xBE`, `0xC4`, `0xC5`) carry a voice
byte that is always 0 regardless of the annotated note's voice; `0xC4` denotes an accent in v0xC2
(where size-22 notes have no articulation slot) but an up-bow in v0xC4.

### Hairpin direction

Byte `+26` bit 0: 0 = crescendo, 1 = diminuendo. Encore 5 also sets bit 1 (crescendo = `0x02`,
diminuendo = `0x03`); legacy files use `0x00` / `0x01`. Test with `& 0x01`, not `== 0`.

### Spanner endpoints

Hairpins and slurs store no separate stop element. The end is described by the forward
measure-count (`+18`) plus the end-x within that target measure (`+20`).

- In v0xC2 the `+18` count is unreliable for hairpins (often stale or zero), so it should not be
  trusted for non-slur ornaments.
- Ottava elements (`0x10` / `0x12`) store no endpoint at all: `+14` is the visual right edge of the
  "8va" text box (a cosmetic constant around 12 px), and the `+18` slot falls outside the element,
  reading the next element's type/voice byte `[observed]`.

**Ornament y sign.** The signed y at `+12` is negative for below the staff, positive for above. A
dynamic dragged up onto the staff above the one that owns it keeps its owner's staff byte but flips
its y positive. A dynamic or staff-text ornament whose tick exceeds the measure's total ticks is a
section-end marker (for example volta dynamics at the end of a short measure). A file can also
carry two dynamics at the same tick and x on one staff and voice (an identical pair, or a
score-view/part-view pair differing only in y placement); Encore renders one dynamic per beat.

### Slur

A slur start (`0x21`) stores the forward measure-count and end-x like other spanners, but the
reliable field differs by version:

- **v0xC4 / SCO5:** the end-x at `+20` is meaningful. `endX - startX` equals the pixel distance
  between the first and last covered notes (`endNote.xoffset - firstNote.xoffset`). xoffset is
  stored as a signed byte but must be read unsigned for this arithmetic (values above 127 are
  stored negative).
- **v0xC2:** the absolute end-x lives in a stale coordinate origin and must not be matched
  directly. The slur's forward measure-count is at `+16` (not `+18`) and is usually reliable,
  including the value 0 (a within-measure slur). It is unreliable for a whole file when any slur's
  `+16` points past the last measure, or when the same multi-measure value (3 or more) repeats
  across slurs starting in different measures (a per-staff constant rather than a real count).

---

## Lyric element

Variable size, null-terminated text (not fixed-width). The text offset differs by version.

| Version | Anchor byte | Text offset |
|---------|-------------|-------------|
| v0xC4   | `+10`       | `+0x14`     |
| v0xC2   | `+10`       | `+0x12`     |
| v0xA6   | `+5`        | `+6`        |

The anchor byte (`+10` in v0xC4/v0xC2, `+5` in v0xA6) is an x-offset-like layout value. In v0xA6
there is no anchor-plus-gap run: a single control byte follows the staff byte, then the text; and
like every v0xA6 element the on-disk slot is twice the declared size.

Observed sizes: v0xC4 24 (dash), 26 (word-break), 30/32/34 (2/3/4 chars); v0xC2 20 (dash), 22/24/26
(1-5 chars); v0xA6 declared 5-8 (10-16 bytes on disk) for 2-6 character syllables `[observed]`.

**Separators.** A `"-"` is a hyphen between syllables of one word; an empty string is a word-break
(resets hyphen state); anything else is a real syllable. A hyphen can open the measure after the
syllable it follows (a word breaking across a barline).

**Multi-verse.** Verse N uses voice N-1 on the same staff; all verses anchor on the voice-0 chord.
Encore stores the first verse (voice 0) with correct per-syllable ticks, but every later verse
stores tick 0 on all its syllables and distinguishes their positions only by the anchor byte at
`+10`, which matches the first verse's x-offsets syllable for syllable. The syllables are not
necessarily stored in x-offset order.

---

## CHORD symbol element

Type 7, variable size. A chord symbol (harmony marking) above the staff.

| Offset | Size | Field   | Description                                                        |
|--------|------|---------|--------------------------------------------------------------------|
| `+5`   | 1    | quality | chord quality index 0-63 (see table)                               |
| `+6`   | 1    | flags   | bit 0 = text present, bit 1 = bass note present, bit 2 = guitar frame drawn |
| `+10`  | 1    | xoffset | horizontal display offset                                          |
| `+12`  | 1    | root    | root note (see root encoding)                                      |
| `+13`  | 1    | bass    | bass note (same encoding; valid only when flags bit 1)             |
| `+14`  | 36   | text    | chord-text slot (present when flags bit 0; UTF-16 LE or Latin-1)   |

When flags bit 0 is set, the text slot overrides the quality and root (the name is taken from the
text). Flags bit 2 records whether Encore draws a guitar frame (fretboard diagram) above the
symbol; it is independent of whether the chord name is recognisable.

**Root / bass encoding.** Low nibble = note name (0=C, 1=D, 2=E, 3=F, 4=G, 5=A, 6=B). High nibble =
accidental (0 = natural, 1 = sharp, 2 = flat). Examples: `0x05` = A, `0x26` = Bb, `0x13` = F#.

**Quality table.** Encore's own chord palette, in palette order `[verified]`. Encore writes the
augmented fifth as "+5" (indices 15, 19); index 46 is "sus2,sus4".

| Index | Suffix         | Index | Suffix     |
|-------|----------------|-------|------------|
| 0     | (major)        | 32    | 9          |
| 1     | m              | 33    | 9(b5)      |
| 2     | +              | 34    | 9(#11)     |
| 3     | dim            | 35    | 11         |
| 4     | dim7           | 36    | 13         |
| 5     | 5              | 37    | 13(b5)     |
| 6     | 6              | 38    | 13(b9)     |
| 7     | 6/9            | 39    | 13(#9)     |
| 8     | (add2)         | 40    | 13(#11)    |
| 9     | (add9)         | 41    | +7         |
| 10    | (omit3)        | 42    | +7(b9)     |
| 11    | (omit5)        | 43    | +7(#9)     |
| 12    | maj7           | 44    | +9         |
| 13    | maj7(b5)       | 45    | sus2       |
| 14    | maj7(6/9)      | 46    | sus2,sus4  |
| 15    | maj7(+5)       | 47    | sus4       |
| 16    | maj7(#11)      | 48    | 7sus4      |
| 17    | maj9           | 49    | 9sus4      |
| 18    | maj9(b5)       | 50    | 13sus4     |
| 19    | maj9(+5)       | 51    | m(add2)    |
| 20    | maj9(#11)      | 52    | m(add9)    |
| 21    | maj13          | 53    | m6         |
| 22    | maj13(b5)      | 54    | m6/9       |
| 23    | maj13(#11)     | 55    | m7         |
| 24    | 7              | 56    | m(maj7)    |
| 25    | 7(b5)          | 57    | m7(b5)     |
| 26    | 7(b9)          | 58    | m7(add4)   |
| 27    | 7(#9)          | 59    | m7(add11)  |
| 28    | 7(#11)         | 60    | m9         |
| 29    | 7(b5,b9)       | 61    | m9(maj7)   |
| 30    | 7(b5,#9)       | 62    | m11        |
| 31    | 7(b9,#9)       | 63    | m13        |

Encore renders chord symbols at beat positions: the symbol's tick carries a small MIDI offset from
the notated beat, so the symbol belongs to the beat at `floor(tick / beatTicks) * beatTicks`.

---

## Multi-staff instruments

For an instrument with more than one staff (piano, harp, organ), all staves share one MEAS element
stream. The destination staff is bits 6-7 of the [staff byte](#staff-byte-encoding): 0 = first
(treble), 1 = second (bass), 2/3 = further staves. All notes of the
instrument carry that instrument's sequential index in bits 0-5 (a piano+organ score where organ
is instrument 1 has all organ notes at bits 0-5 = 1, regardless of how many LINE slots piano
occupies). For a two-staff instrument, voices 0-1 belong to the first staff and voices 2-3 to the
second (renumbered 0-1 there).

---

## Chord column (xoffset)

The note `xoffset` byte (`+10`) is the notated horizontal column of the note. It exists in v0xC2,
v0xC4, and SCO5; v0xA6 does not store it.

- Every member of one chord shares the same non-zero `xoffset`; successive chords occupy distinct
  columns. A `xoffset` of 0 means no stored column.
- The layout runs strictly left to right, so `xoffset` increases with tick and is aligned across
  the staves of a system: notes on the same beat share a column across staves.
- Adjacent columns lie at least a small minimum apart (around eight pixels in observed files),
  while a chord's members share a column (give or take a notehead width) `[observed]`.
- The notes of one chord are not always stored at the same tick. A chord recorded live or given a
  per-chord "strum" keeps its members at slightly staggered playback ticks (drift up to a sizeable
  fraction of the note value has been observed) while still sharing one column `[observed]`.
- A note whose column matches an earlier beat but whose MIDI tick is later is a stale-tick artifact
  (the note was moved in Encore and kept its old playback tick); Encore draws it at the column's
  beat.

(How a parser uses the column to rebuild chords, split tightly played tuplets, and correct
stale ticks is in ENCORE_IMPORTER.md.)

---

## BEAM element

Type 4. Explicit beaming, one element per beam level.

| Size | Byte `+5` | Beam level             |
|------|-----------|------------------------|
| 30   | `0x01`    | 1st (eighth flag)      |
| 46   | `0x02`    | 2nd (sixteenth)        |
| 62   | `0x03`    | 3rd (thirty-second)    |

---

## TEXT block

Magic `TEXT`. Text payloads referenced by staff-text ornaments (subtype `0x1E`), indexed by the
ornament's entry index (`tind`). A file may contain several TEXT blocks; each later block is a
part-view copy with the same strings reordered, and the ornament index is relative to the first
(score) block only.

Block layout (after the 8-byte magic + varsize):

| Offset | Size | Description         |
|--------|------|---------------------|
| `+0`   | 2    | sync (`0x0000`)     |
| `+2`   | 2    | entry count         |
| `+4`   | 4    | total content bytes |
| `+8`   | var  | entries             |

Each entry:

| Offset | Size | Description                                                 |
|--------|------|-------------------------------------------------------------|
| `+0`   | 2    | payload size                                                |
| `+2`   | var  | rich-text run header (see below)                            |
| after  | var  | text (UTF-16 LE or Latin-1); lines separated by `0x04 0x00` |
| end    | 2+   | `0x00 0x00` terminator (may be followed by padding)         |

**Rich-text run header.** Measured from the payload start (entry `+2`):

| Payload offset   | Size            | Description                                       |
|------------------|-----------------|---------------------------------------------------|
| `+0`             | 2               | run-offset table count                            |
| `+2`             | 2               | descriptor count                                  |
| `+4`             | tableCount * 4  | run-offset table (per-run character positions)    |
| after table      | descCount * 6   | style descriptors                                 |
| after            | var             | the displayed text                                |

The text starts at payload offset `4 + tableCount*4 + descCount*6`. Both words at `+0` and `+2` are
genuine counts (an entry can carry two descriptors as well as one); a single-run single-descriptor
comment puts the text at offset 14. When either count is 0 or the computed offset exceeds the entry,
fall back to offset 14.

In v0xA6 the entry has no rich-text header: the text starts at payload offset 0 (entry `+2`) and is
null-terminated Latin-1.

**Line separators.** `0x04 0x00` (U+0004) separates lines within one comment; it is not the
terminator. Every line, including the last, is followed by `0x04 0x00`, and the whole string ends
at `0x00 0x00`. Text length is bounded by the null terminator, not by the payload size (some
entries carry padding after the terminator). Dynamic marks are not in the TEXT block; they use
ornament subtypes.

---

## TITL block

Magic `TITL`. One block per page (page 2+ blocks are usually empty). Holds a title, 2 subtitles, 3
instructions, 4 authors, 2 headers, 2 footers, and 6 copyright lines, in that order, each slot a
fixed-width field.

**Encoding by varsize** `[verified]`:

| varsize     | Encoding   | Bytes per line |
|-------------|------------|----------------|
| < 5000      | Latin-1    | 96             |
| >= 10000    | UTF-16 LE  | 1056           |
| 5000..9999  | (rare) not disambiguated by varsize; fall back to the file's instrument-name encoding | |

The two normal layouts differ by roughly 10x (Latin-1: 2 + 20 lines x 96 + 504 pad = 2426;
UTF-16: 2 + 20 lines x 1056 + 120 pad = 21242), so varsize resolves them unambiguously.

Each line is a 30-byte prefix followed by the text field (66 Latin-1 bytes or 1026 UTF-16 bytes),
NUL-terminated and zero-padded; bytes after the NUL are prior-edit debris.

**Header/footer alignment.** Prefix byte `+14`: `0x02` = right, `0x04` = left, `0x06` = center;
`0x00` on other line types.

**Slot counts.** title 1, subtitle 2, instruction 3, author 4, header 2, footer 2, copyright 6.
Multiple non-empty slots in a category render as stacked lines.

**Replaceable tokens (header/footer).** `#P` = page number, `#D` = date, `#T` = time.

---

## WINI block

Magic `WINI`. Page margins. Optional: files never taken through Encore's Page Setup have no WINI
block `[observed]`.

Layout (after the 8-byte magic + varsize):

| Offset | Size | Type    | Description                                                    |
|--------|------|---------|----------------------------------------------------------------|
| `+0`   | 24   | bytes   | window/screen data (not page geometry)                         |
| `+24`  | 4    | int32   | top margin                                                     |
| `+28`  | 4    | int32   | left margin                                                    |
| `+32`  | 4    | int32   | bottom edge of printable area (pageHeight - bottomMargin)      |
| `+36`  | 4    | int32   | right edge of printable area (pageWidth - rightMargin)         |
| `+40`  | 2    | uint16  | flags (observed 1)                                             |

Content size is 42 bytes; some older files omit the trailing uint16 (varsize 40). Both are valid.

**Units, two variants** `[observed]`:

- Encore 5.x stores margins in typographic points (1/72 inch).
- Earlier versions (4.x and some 3.x) store them in screen pixels at the monitor DPI (around 84 PPI,
  the exact value being the screen the file was last saved on). Displayed margins are still inches;
  only the stored unit is device pixels.

Tell them apart by magnitude: when an edge exceeds the page dimension expressed in points
(rightEdge above pageWidth x 72, or bottomEdge above pageHeight x 72) the block is in screen pixels,
otherwise in points. In the screen-pixel case the page dimensions are not stored and must be
recovered from margin symmetry: `pageWidth = rightEdge + left`, `pageHeight = bottomEdge + top`
(assuming equal far margins), matched against standard paper sizes. All ISO A-series sizes share
the 1:root-2 ratio, so at most two fall in the plausible DPI range and the correct one minimises
`|dpiW - dpiH|`.

Encore stores `round(inches x 72)` and displays `floor(pts / 72 x 1000) / 1000`, so 0.100" stores
as 7 pts and displays back as 0.097". All four values 0 means no stored margins. Encore's Page
Setup dialog may show a non-zero margin for a side with a zero stored value; that displayed value is
the active printer's hardware non-printable zone, read at display time and not stored in the file.

---

## PREC block

Magic `PREC`. Printer / page-setup state, 132 bytes to several KiB, in one of two encodings by
platform:

- **Windows (`SCOW`):** a Windows DEVMODE structure.
- **macOS (`SCO5`):** a macOS NSPrintInfo XML plist (begins with `<?xml ... <plist ...`). Paper from
  `PMTiogaPaperName` / `PMPaperName` (`na-letter`, `iso-a4`, ...), orientation from
  `PMOrientation` (1 = portrait, 2 = landscape), notation scale from `PMScaling` (a fraction, 1.2 =
  120%). The plist carries only the printer's imageable rects, not document margins, so `SCO5` page
  margins are not available from any block.

PREC is present in almost every file across all versions, while WINI exists only in some, so the
page size, orientation, and notation scale come from PREC even for v0xA6/v0xC2 and for files with no
WINI.

**DEVMODE.** A device-name field (32 bytes ANSI, or 64 bytes for a Unicode UTF-16 name) followed by
fixed fields. Relative to the field base (32 for ANSI, 64 for Unicode):

| Offset (from base) | Field         | Meaning                                                    |
|--------------------|---------------|------------------------------------------------------------|
| `+12`              | orientation   | 1 = portrait, 2 = landscape                                |
| `+14`              | paper size    | Windows DMPAPER enum (see below)                           |
| `+16`              | paper length  | tenths of a millimetre (custom sizes only; 0 for standard) |
| `+18`              | paper width   | tenths of a millimetre (custom sizes only)                 |
| `+20`              | scale         | notation-size percent (100 = default)                      |

Detect the variant by reading orientation at both bases and keeping the one that is 1 or 2. Paper
size values: 1 = Letter, 5 = Legal, 7 = Executive, 8 = A3, 9 = A4, 11 = A5, 12 = B4, 13 = B5. When
the paper size is custom/unknown, fall back to paper length/width. Across the corpus the scale is a
clean distribution of round percentages and the paper size is dominated by A4 and Letter
`[observed]`.

---

## Rhythm encoding

240 ticks per quarter note. The whole-note tick count is always 960 for any time signature and is
computed as `durTicks * timeSigDen / timeSigNum`. Do NOT use `beatTicks * timeSigDen`: in compound
meters `beatTicks` is the compound beat (360 for the dotted quarter in 6/8), giving 2880 instead of
960.

| Face value | Ticks | Duration |
|------------|-------|----------|
| 1          | 960   | whole    |
| 2          | 480   | half     |
| 3          | 240   | quarter  |
| 4          | 120   | eighth   |
| 5          | 60    | 16th     |
| 6          | 30    | 32nd     |
| 7          | 15    | 64th     |
| 8          | 7     | 128th    |

The 128th value is stored as 7 (960 / 128 = 7.5, truncated).

Notated duration = face value + dots + tuplet ratio. The playback duration at note `+16` diverges
from the notated value (live recording, ties, tuplets), and the last note of a tuplet that ends at
the barline often has a playback duration far shorter than its face value because Encore truncates
playback durations at the barline.

**Dots.** The dot count is not reliably in the bytes (byte `+14` is a layout `dotControl`, not a
count; see [Note element](#note-element)). It is derived from the sounding duration relative to the
face value (`playbackTicks == faceTicks x 3/2` = one dot, `x 7/4` = two dots).

**Tuplets.** Either an explicit byte `(actualN << 4) | normalN` (`0x32` = 3:2, `0x54` = 5:4) or,
in v0xC2 files, implicit (playback duration approximately `faceTicks x 2/3` or `x 4/5`). Ratios
whose bracket span (`normalN x baseLen`) is not a standard note value cannot be notated as a
bracket.

| Ratio                   | Name       |
|-------------------------|------------|
| 2:1                     | (whole duplet) |
| 2:3                     | compound duplet |
| 2:4                     | 2 in 4     |
| 3:2                     | triplet    |
| 4:1, 4:2, 4:3           | quadruplet |
| 5:2, 5:3, 5:4, 5:6, 5:8 | quintuplet |
| 6:4, 6:7, 6:8           | sextuplet  |
| 7:4, 7:6, 7:8           | septuplet  |
| 8:4, 8:6                | octuplet   |
| 9:4, 9:6, 9:8           | nontuplet  |
| 10:6, 10:8              | decuplet   |

In compound and simple meters where one beat equals an eighth (6/8, 8/8, 12/8), Encore stores the
face value as a number of beats rather than an absolute note value, so the written duration is
`faceTicks x actualN / normalN` when that product is a standard tick count.

---

## Encoding probe

Text-bearing fields are Latin-1 or UTF-16 LE, chosen per field by a byte probe. The general rule:
byte 0 printable ASCII (`0x20`-`0x7E`) followed by byte 1 == `0x00` means UTF-16 LE; anything else
(especially an accented Latin-1 byte in byte 1) means Latin-1.

| Field                    | Probe position                                              |
|--------------------------|-------------------------------------------------------------|
| TK instrument name       | first two bytes of the name                                 |
| positional name recovery | first two bytes at the formula offset                       |
| LYRIC text               | first two bytes of the text                                 |
| TEXT block entry         | bytes 14/15 of the entry; lines split on `0x04 0x00`        |
| CHORD-symbol text        | first two bytes of the 36-byte slot                         |
| TITL block               | by varsize: < 5000 Latin-1, >= 10000 UTF-16 (see [TITL](#titl-block)) |

Forcing one encoding turns legacy Latin-1 into byte-swapped CJK gibberish, or drops the second half
of every UTF-16 code unit; the probe is always applied.

---

## Known quirks

### Per-version deltas

Every systematic deviation, with a link back to the section that details it.

| Aspect                        | v0xA6                    | v0xC2                          | v0xC4 / SCO5           |
|-------------------------------|--------------------------|--------------------------------|------------------------|
| [Header](#header) end         | `0xA6` (166)             | `0xC2` (194)                   | `0xC2` (194)           |
| Size selector offset          | `0x8D`                   | `0x52`                         | `0x52`                 |
| [MEAS](#measure-block-meas) header | `0x1A` (26)         | `0x36` (54)                    | `0x36` (54)            |
| Element slot stride           | `size * 2`               | `size`                         | `size`                 |
| [Note](#note-element) size    | 10 / 11                  | 22 / 24                        | 28                     |
| Note pitch offset             | `+11`                    | `+13` or `+15` (sub-variant)   | `+15`                  |
| Note tuplet offset            | `+7`                     | `+13` (sub-variant B)          | `+13`                  |
| [Lyric](#lyric-element) text  | `+6`                     | `+0x12`                        | `+0x14`                |
| [Ornament](#ornament-element) y | `+8` (compact)         | `+12`                          | `+12`                  |
| Ornament staff-text index     | `+28` (compact)          | `+32` or `+30`                 | `+32`                  |
| Slur forward-count field      | (no xoffset)             | `+16`                          | `+18` / end-x at `+20` |
| [Chord column](#chord-column-xoffset) | not stored       | stored                         | stored                 |
| [Key signature](#key-encoding)| LINE 22-byte entry `+14` | LINE 30-byte entry `+15`       | LINE 30-byte entry `+15` |
| [TEXT](#text-block) run header | absent                  | present                        | present                |
| Grace1 tie-sender nibble      | always 0                 | low nibble = 1 when tie sender | always 0               |
| [WINI](#wini-block) unit      | screen pixels            | pixels or points               | points                 |

### Other oddities

- **Duplicate NOTE elements.** Some files encode the same pitch twice in one chord: either the
  second copy has `grace1 & 0x40` set (a chord-extension marker), or, in v0xC2, both copies have
  `grace1 = 0`. Either way it is a redundant notehead.
- **v0xA6 consecutive identical rests.** Two identical rest elements (same tick, staff, voice, face
  value) in a row represent one rest.
- **Note spelling on transposing staves.** Encore stores only the sounding pitch plus the Key
  offset (see [Instrument block](#instrument-block)); it never stores an explicit enharmonic
  spelling. The bytes fix the audible pitch, not the written accidental.
- **Tempo words.** Italian tempo words ("Allegro", "Andante", ...) are stored as staff-text
  ornaments, not as a tempo element. Numeric tempo marks use the ornament TEMPO subtype (`0x32`).
  Each MEAS header also carries a quarter-note BPM that persists on every measure. The TEMPO
  ornament's stored tick rarely matches the measure where the tempo applies, so the per-measure
  header BPM is the reliable source for its position.
- **Largest legitimate block.** A MEAS block rarely exceeds 2 KiB; a significantly larger block
  indicates a corrupt file or an undocumented variant.
- **Encore 5.0.2 instrument names.** Encore 5.0.2 writes UTF-16 instrument names regardless of the
  size field, and may omit a TK header while the name is still present at the positional offset.
