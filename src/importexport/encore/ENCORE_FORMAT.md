# The Encore `.enc` binary format

Encore is a music notation program, first published by Passport Designs and sold since 1991. This document describes the bytes of the files it writes: what is stored, where, and what it means. It is written to be read start to finish by a person, and to be complete enough that a reader can write a working parser from it alone.

First documented by Felipe Castro (enc2ly) and Leon Vinken (Enc2MusicXML, GPL v3+), then extended here by observation across a corpus large enough, and varied enough in versions and in notation features, to support the statements below. The method throughout is the same: open or write a score in a licensed copy of Encore, save it again from another version of the program, compare the two files field by field, and check every reading against what Encore itself displays and, where its MusicXML export carries the detail at all, against that too. The display is the better witness of the two: the export drops and garbles enough of what the file holds that it settles a question only when it happens to state it.

## How to read this document

Every non-trivial statement carries a tag saying how well it is known. This matters more than usual here, because the format was never published: every statement comes from reading files and from checking the reading against Encore.

| Tag          | What it means                                                      |
|--------------|--------------------------------------------------------------------|
| `[verified]` | Round-tripped against Encore, or confirmed by a one-attribute diff |
| `[observed]` | Consistent across many files, but never confirmed against Encore   |
| `[assumed]`  | Plausible and consistent with the data, but neither of the above   |
| `[external]` | From a source outside the files themselves                         |

Statements with no tag are structural facts read straight off the byte stream: block framing, element headers, the offsets a parser must follow to advance at all.

**Offsets.** Every element field offset is measured from the start of the element, so `+0` is the first tick byte, `+2` the type and voice byte, `+3` the size, `+4` the staff, and the body begins at `+5`. Block field offsets are measured from the start of the block content, past the eight bytes of magic and size. Header offsets are absolute from the start of the file.

---

# 1. The formats, and how they came to be

Four generations of the format occur in the wild. They are not variations on a theme: each boundary is one decision that moved many things at once, and a parser that gets the generation wrong does not misread one field, it misreads every element in the file.

Two programs write it. Alongside Encore, the same publisher sold MusicTime, a smaller and cheaper sibling, and it writes the same file down to the element geometry: its own magic, and everything below it as described here. It is not a fifth generation off to one side; it moves through the same ones Encore does, and the corpus holds MusicTime documents in two of them. The two programs open each other's documents because there is nothing to convert, so every page here describes both and the rest of the document has no reason to name MusicTime again `[verified]`.

## 1.1 Three independent coordinates

The header carries three values that a reader might use to decide what it is holding. They are not redundant, and no one of them is sufficient.

| Coordinate      | Where  | What it tracks                                |
|-----------------|--------|-----------------------------------------------|
| container magic | `0x00` | byte order, and whether the file is encrypted |
| version byte    | `0x04` | the ornament and articulation vocabulary      |
| format version  | `0x28` | the size and layout of every element          |

The format version is the one that governs the bytes. The version byte matters only for the ornament vocabulary, and the two genuinely disagree in one place, described in 1.5.

## 1.2 The containers

The first four bytes are the file magic, and they fix the byte order for every multi-byte integer in the file. There is one exception, the size field of an instrument block, noted in 5.1.

| Magic  | Storage   | Byte order    | Notes                                           |
|--------|-----------|---------------|-------------------------------------------------|
| `SCOW` | plaintext | little-endian | Windows Encore, every version `[verified]`      |
| `SCOR` | plaintext | little-endian | Windows Encore, as `SCOW` reads `[verified]`    |
| `SCO5` | plaintext | big-endian    | macOS Encore 5 `[verified]`                     |
| `MTIW` | plaintext | little-endian | Windows MusicTime `[verified]`                  |
| `MTIM` | plaintext | big-endian    | macOS MusicTime `[verified]`                    |
| `ZBOT` | encrypted | ,             | wraps `SCOW`, older Encore 4 saves `[verified]` |
| `ZBOP` | encrypted | ,             | wraps `SCOS`, no sample `[verified]`            |
| `ZBO6` | encrypted | ,             | wraps `SCO5` `[verified]`                       |

`SCOW` covers most files, `ZBOT` a sizeable minority and `SCO5` a handful; every `ZBOT` decrypts to a `SCOW` body.

The names pair up the same way on both sides: `SCOW` and `MTIW` on Windows, `SCO5` and `MTIM` on the Macintosh, big-endian. A reader that handles the compact generation handles MusicTime by accepting the two extra magics.

**A magic never says which generation a file belongs to**, only which program wrote it and which way its integers run. Every `SCOR` file seen is an ordinary Windows document of format 3.07, header ending at `0xC2` and blocks in the usual order, and the MusicTime documents split between two generations the same way Encore's do. Read the format version and the version byte for the layout, and the magic for nothing else `[verified]`.

**The magic is the whole test.** A file whose first four bytes are none of the above is not an Encore document and is to be rejected. There is no fallback signature and no recovery: the byte order, the header layout and the position of the first block all follow from the magic, so nothing below it can be read without one.

The three encrypted magics are the three plaintext ones seen through the keystream, which is fixed and does not depend on the file: its first four bytes are `09 01 00 03`, and applying them turns `ZBOT` into `SCOW`, `ZBOP` into `SCOS` and `ZBO6` into `SCO5` `[verified]`. The `ZBO6` case is no longer arithmetic: the files carrying it decrypt to a `SCO5` document, every one big-endian format 4.20 with its first block at `0xC2`, which is a macOS Encore 5 file wrapped exactly as a Windows one is `[verified]`. No `ZBOP` file has turned up, so that row still rests on the keystream alone; the risk is contained, since a decrypted buffer has to pass the magic and header check and a file that fails it is rejected rather than imported as wrong music.

`SCOX` appears in earlier descriptions of the format and is absent from the table because it has never been seen.

`SCOS` is a third name of that shape, and it stands on firmer ground than `SCOX`: it is what `ZBOP` decrypts to, so the program has a container by that name even though no file carrying it has turned up.

**Encryption is a wrapper, not a format.** A `ZBOT` family file is an ordinary Encore document under a layer of XOR, and once that layer is undone everything in this document applies to it unchanged. Nothing below the magic, the version byte included, can be read until then. Section 2.3 describes the layer.

## 1.3 The four generations

The format version at `0x28` is BCD, the major digit in the high byte, so `0x0420` reads as 4.20. It is the version of Encore that wrote the file: Encore 2.5.1 ships an example score stamped 2.50 and Encore 3 ships one stamped 3.05, both read out of the original distributions `[verified]`. The field stops tracking the release at 4.20, which Encore 4.x and every 5.x write unchanged.

| Format | Bytes    | Release        | First dated | Evidence                            |  |
|--------|----------|----------------|-------------|-------------------------------------|--|
| 2.50   | `0x0250` | Encore 2.5     | 1993        | its own `SILENT.ENC` `[verified]`   |  |
| 2.62   | `0x0262` | MusicTime      | ,           | its own documents `[verified]`      |  |
| 3.05   | `0x0305` | Encore 3       | 1996        | a file on its disks `[verified]`    |  |
| 3.07   | `0x0307` | unsampled      | 1999        | corpus only `[observed]`            |  |
| 4.20   | `0x0420` | Encore 4.x-5.x | 1997        | its own example scores `[verified]` |  |

Every generation is represented in the corpus, 4.20 in most files and 2.50 in the fewest. The dates are the earliest file seen carrying each, which is a lower bound: a file cannot predate the program that wrote it, but a user may keep an old release for years, and many did.

Format 2.62 belongs to MusicTime rather than to Encore, and it reads with the 2.50 geometry in full. It is not the only version MusicTime writes, though: of the MusicTime documents in the corpus, roughly half carry 2.62 and the rest carry 3.07, with the header, blocks and element geometry of that generation and nothing of its own `[observed]`. A reader therefore gains nothing by treating the MusicTime magics as a generation; it should read the format version and forget which program was named.

Which release wrote format 3.07 is not established. Its files run from 1999 and its element geometry sits squarely between 3.05 and 4.20, so it belongs between them, but no distribution in hand produces it, and the Encore 4 example scores are dated November 1997 and carry 4.20, which is earlier. Both programs write it, so it is not a MusicTime detour either.

## 1.4 What changed at each boundary

Read down this table and the whole history of the format is there.

| Boundary     | What moved                                                                     |
|--------------|--------------------------------------------------------------------------------|
| into 2.50    | compact bodies: sizes in 2-byte units, a 10-byte note, a 7-byte rest           |
| 2.50 to 3.05 | full-width bodies, sizes in bytes, every element roughly doubled               |
| 3.05 to 3.07 | **two bytes inserted at `+6` of every element**, five articulations renumbered |
| 3.07 to 4.20 | the note alone grows by four, for its two articulation slots                   |

**The two-byte insertion is the single most consequential fact in this document.** Bytes `+0` to `+5` are byte-identical between a 3.05 element and its 3.07 conversion, and from `+6` onward the later element carries two extra bytes and then continues identically, verified across ties, beams, ornaments and rests on a converted file `[verified]`. So a field at absolute offset `X` in a 3.05 file is at `X + 2` from 3.07 on, for every element type at once.

Element sizes follow from it. These are the most common size of each element in each generation, in bytes, and for 2.50 in its own 2-byte units:

| Element   | 2.50 | 3.05 | 3.07 | 4.20 |
|-----------|------|------|------|------|
| CLEF      | 5    | 14   | 16   | 16   |
| KEYCHANGE | 5    | 12   | 14   | 14   |
| TIE       | 7    | 16   | 18   | 18   |
| BEAM      | 9    | 28   | 30   | 30   |
| ORNAMENT  | 5    | 26   | 28   | 28   |
| LYRIC     | 5    | 22   | 24   | 24   |
| CHORD     | 6    | 14   | 18   | 16   |
| REST      | 7    | 16   | 18   | 18   |
| NOTE      | 10   | 22   | 24   | 28   |
| MIDI CC   | 4    | 10   | 12   | 12   |

Every element gains exactly two bytes from 3.05 to 3.07 `[verified]`. From 3.07 to 4.20 only the note changes.

## 1.5 The version byte, and where it disagrees

The byte at `0x04` is present in the plaintext containers only. `SCO5` has none: it is recognised by its magic alone and otherwise reads as the 4.20 layout.

| Byte   | Generation it names                                          |
|--------|--------------------------------------------------------------|
| `0xA6` | format 2.50 only `[verified]`                                |
| `0xC2` | formats 3.05 and the earlier 3.07 saves `[verified]`         |
| `0xC4` | the later 3.07 saves, and 4.20 through Encore 5 `[verified]` |

The version byte and the format version are not redundant, and format 3.07 is where they part. It occurs with both `0xC2` and `0xC4`; the two groups share every element size but use disjoint articulation codes, the `0xC2` group writing an accent as `0xC4` and the `0xC4` group as `0xBE`, with no overlap across the corpus `[observed]`. Whichever release wrote 3.07 renumbered its ornament vocabulary partway through its life and bumped the version byte for it, leaving the format version alone. This is why the version byte is worth reading at all: it is precisely the vocabulary axis.

## 1.6 The revision byte

The byte at `0x3E` takes the values 0, 1, 2 and 4. It is **not** a layout selector and nothing in a parser should branch on it.

| Value | What it marks                                                                        |
|-------|--------------------------------------------------------------------------------------|
| 0, 1  | nothing that separates one release from another; every generation writes both        |
| 2     | macOS Encore 5 `[observed]`                                                          |
| 4     | Encore 5.0, never seen before 2009 in any dated file `[verified]`                    |

Two of the values have a release behind them, measured by saving one score through each program: Encore 4.5 writes 0 and Encore 5.0 writes 4 `[verified]`. Encore 4.5 also refuses a file whose `0x3E` is newer than it supports, which is why a 5.0 file will not open there.

Only two of the four values say anything about a release, and a sweep of the whole corpus shows why. Revisions 0 and 1 appear in every generation the format has, from 2.50 through 4.20, and in MusicTime as well, so the pair cannot be read as an era. The 0 against 1 split is not a build boundary either: one author working in a single release produced files at both revisions in comparable numbers, and the two groups have identical instrument entry strides, element sizes, ornament vocabulary and score shapes `[verified]`. What the byte actually tracks is unresolved. Revision 2 is the sharper of the two remaining values: every `SCO5` file in the corpus carries it, and no file of any other container ever does `[observed]`.

## 1.7 Choosing a reader

Everything above reduces to this procedure.

1. Read four bytes. If they are not one of the magics listed in 1.2, reject the file.
2. If the magic is a `ZBOT` family one, decrypt, then start again from step 1 on the result.
3. Set the byte order from the magic: big-endian for `SCO5`, little-endian otherwise.
4. Read the format version at `0x28`. It gives the element geometry, and it is ordered, so a value this document does not list belongs to the layout of the highest listed version it is not older than.
5. Read the version byte at `0x04`, if the container has one. It selects the articulation vocabulary, which matters only below format 3.07.

---

# 2. How a file is laid out

## 2.1 Blocks

A file is a fixed header followed by a sequence of blocks. Each block begins with a four-byte ASCII magic and a four-byte size, then that many bytes of content.

| Block          | Count         | Contents                                                |
|----------------|---------------|---------------------------------------------------------|
| header         | 1             | version, counts, global staff size; no magic of its own |
| `TK00`..`TKnn` | 0 to n        | one per instrument: name, MIDI program, transposition   |
| `PAGE`         | 0 to n        | page geometry, not decoded                              |
| `LINE`         | 1 per system  | the staves of that system, with clef, key and size      |
| `MEAS`         | 1 per measure | the elements: notes, rests, ornaments and the rest      |
| `PREC`         | 0 or 1        | printer and page-setup state                            |
| `WINI`         | 0 or 1        | page margins                                            |
| `TITL`         | 1 per page    | title, subtitles, authors, copyright, header, footer    |
| `TEXT`         | 0 to n        | text payloads referenced by staff-text ornaments        |

They appear roughly in that order, but a parser locates each by scanning for its magic rather than trusting the order, and skips unknown bytes between known magics.

**The size field excludes the eight bytes of magic and size**, so the next block begins at `blockStart + 8 + size`. There is one exception and it matters: **a measure block's size excludes its own measure header as well**, so the next block begins at `blockStart + 8 + size + headerLength`, where the header is 54 bytes from format 3.05 on and 26 bytes in 2.50. A parser that misses this walks off the rails at the second measure.

The instrument blocks have a second exception, described in 5.1, where some files store the total block size instead.

## 2.2 What a reader must not trust

Three fields look authoritative and are not.

- **The digits in a `TK` magic.** They usually name the instrument the block describes, but files exist whose seven consecutive blocks read `TK00 TK01 TK02 TK04 TK04 TK05 TK06`, skipping an index and repeating another, and others that zero an entire block header so it vanishes from a magic scan `[observed]`. What a block describes is decided by where it sits in the entry table, not by its digits.
- **The size a `TK` block declares.** Encore 4 saves routinely declare 112 whatever the entry really measures, and Encore 5 files with 2158-byte entries still declare 112 `[observed]`. Section 5.1 derives the real stride from the file.
- **The measure count in the header.** It is the number of measures Encore *displays*. A file can carry extra measure blocks left from earlier edits; only the first `0x34` of them are real. A file has been seen rendering barely two thirds of the blocks it held `[observed]`.

## 2.3 The encrypted wrapper

Many of the files in the corpus are encrypted, all of them from the Encore 4 line: every one carries revision 0 or 1 once decrypted, and a few carry format 3.05, so the wrapper is no newer than Encore 4.3 `[external]`. Encore 5.0.2 opens such files directly and saves them plaintext `[verified]`.

The wrapper is a byte for byte XOR of the entire file, magic included, against a keystream that does not depend on the file: it is the same sequence for every document, and since XOR is its own inverse, one pass both encrypts and decrypts. Run it over the buffer from the first byte and what comes out is a plaintext file, read from there exactly like any other `[verified]`.

The keystream is generated from two tables. The first is a step table of 17 entries, short enough to state here:

```
14, 2, 8, 9, 1, 0, 3, 7, 6, 5, 4, 11, 12, 15, 10, 13, 16
```

The second is a substitution table of 9776 rows of four values, 39104 bytes in all. It is not reproduced in this document; the note below says where it comes from. Nearly every value in it is a digit from 0 to 9, and exactly nine of the 39104 are larger and hold ordinary byte values `[verified]`.

Generation walks the file one byte at a time and keeps three cursors: a row `pos` starting at `0xAB`, a column `sub` starting at 0, and a step index cycling through the 17 entries of the step table.

```
for each byte of the file:
    delta = stepTable[step], then advance step, wrapping to 0 after the 17th entry
    row   = (pos + delta) mod 9776
    out   = in XOR table[row][sub]
    advance sub; when it passes the fourth value of the row, reset it to 0 and set pos = (pos + 1) mod 9776
```

Within a row the four values are consumed in the order third, fourth, first, second, relative to how they sit in a build for a little-endian processor `[verified]`. The row is one 32-bit word, which is why its order matters and why a big-endian build stores it mirrored: the PowerPC Macintosh releases carry the identical table with every word byte-swapped. Getting the order wrong produces plausible looking noise rather than an obvious failure, so check the output and not the reasoning: the first eight keystream bytes are `09 01 00 03 01 06 04 05`, which is exactly what carries `ZBOT` to `SCOW`.

**Where the substitution table comes from.** It is data, not program logic, and it ships verbatim with the program: the same 39104 bytes sit contiguously in the Windows releases of Encore 4.5 and 5.0.2, in the macOS releases of Encore 4.5.6 and 5.0.5, and in both platforms' releases of MusicTime Deluxe, a different product of the same publisher `[verified]`. Comparing what those releases ship and keeping only the regions they hold in common isolates it, with no examination of program instructions involved.

**Verify before trusting.** Decryption always produces bytes, so it never fails on its own. Treat the result as a candidate: it counts as decrypted only if the leading magic is now a plaintext one and the header reads sensibly. Anything else is rejected, which is what keeps an unfamiliar variant from being imported as wrong music.

---

# 3. A file, byte by byte

A complete file, 404 bytes, in format 2.50. It has one instrument, one system and one measure of 2/4 holding two quarter notes. Everything in the rest of this document is an elaboration of this walk.

**The header**, at offset 0. In 2.50 it ends at `0xA6`; from 3.05 on it ends at `0xC2`.

```
0000  53 43 4f 57 a6 00 00 00  SCOW....     magic, then the version byte 0xA6
0028  50 02 00 00 00 00 01 00  P.......     format version 0x0250, then counts
0030  01 00 01 00 01 00 00 00  ........     1 page, 1 instrument, 1 measure
```

**The instrument block**, at `0xA6`. Magic, size 0x40, then the name.

```
00A6  54 4b 30 30 40 00 00 00  TK00@...     magic TK00, content size 64
00AE  56 6f 7a 00 00 00 00 00  Voz.....     the name, NUL-terminated
```

**The system block**, at `0xE6`. Its content holds a 14-byte preamble and then one 22-byte staff entry per staff, in this generation.

```
00E6  4c 49 4e 45 24 00 00 00  LINE$...     magic LINE, content size 36
```

**The measure block**, at `0x112`. Magic and size, then the 26-byte measure header, then the elements.

```
0112  4d 45 41 53 60 00 00 00  MEAS`...     magic MEAS, content size 96
011A  64 00 00 00 f0 00 e0 01  d.......     100 bpm, 240 beat ticks, 480 total
0122  02 04                    ..           2/4
```

The element stream starts once the measure header ends, at `0x134`. The first element:

```
0134  00 00 90 0a 00 03 00 00  ........     tick 0, note in voice 0, size 10, staff 0
013C  00 05 00 45              ...E         face value quarter, position 5, pitch 0x45
```

Reading it field by field: `+0` is the tick, 0. `+2` is `0x90`, whose high nibble 9 is the note type and whose low nibble 0 is the voice. `+3` is the size, 10. `+4` is the staff, 0. `+5` is the face value, 3, a quarter note. `+9` is the staff position, 5, five diatonic steps above middle C. `+11` is the MIDI pitch, `0x45`, which is 69, the A above middle C. Consistent: position 5 above C4 is A4.

In this generation the on-disk slot is twice the declared size, so the next element sits 20 bytes on, at `0x148`:

```
0148  f0 00 90 0a 00 03 00 00  ........     tick 240, another quarter note
0150  00 ff 00 3b              ...;         position -1, pitch 0x3B
```

Tick 240 is the second beat. Position `0xFF` is -1 as a signed byte, one step below middle C, and pitch `0x3B` is 59, the B below middle C. Consistent again.

Two quarter notes, A4 then B3, in a 2/4 bar at 100 beats per minute. The stream ends with a tick of `0xFFFF`.

---

# 4. The header

A fixed region at the start of the file with no magic of its own. It ends at `0xA6`, 166 bytes, in format 2.50, and at `0xC2`, 194 bytes, from 3.05 on `[verified]`. The first block begins immediately after.

| Offset | Size | Field                                                               |
|--------|------|---------------------------------------------------------------------|
| `0x00` | 4    | container magic                                                     |
| `0x04` | 1    | version byte; absent in `SCO5`                                      |
| `0x28` | 2    | format version, BCD                                                 |
| `0x2A` | 2    | purpose unconfirmed, possibly a total staff-entry count `[assumed]` |
| `0x2C` | 2    | default beat ticks, 240 for a quarter-note grid                     |
| `0x2E` | 2    | number of system blocks                                             |
| `0x30` | 2    | number of pages                                                     |
| `0x32` | 1    | number of instrument blocks                                         |
| `0x33` | 1    | staves per system; always 0 in formats 2.50 and 2.62 `[observed]`   |
| `0x34` | 2    | rendered measure count; see 2.2                                     |
| `0x3E` | 1    | revision byte; see 1.6                                              |
| `0x52` | 1    | global staff-size selector, formats 3.05 and later                  |
| `0x8D` | 1    | global staff-size selector, format 2.50                             |

Everything else up to the header end is padding.

**Staff size.** The selector takes a value from 1 to 4, choosing one of four scales: 60%, 75%, 100% and 130% `[observed]`. From format 3.05 on it lives at `0x52` and is only a global fallback, because the authoritative per-staff size is a byte in the system block; in Encore 4.x the `0x52` field is unrelated to size altogether. In format 2.50 the header byte is `0x8D` rather than `0x52`, and it is a fallback there too: that generation has a per-staff size of its own, in the system block, and the header byte only tends to agree with it (see 5.2).

---

# 5. The blocks

## 5.1 Instrument block (`TK00`, `TK01`, ...)

One per instrument, carrying the name, the MIDI program and the Key transposition. The name is Latin-1 or UTF-16 LE, chosen by the probe in 7.8, NUL-terminated within the block.

**The size field is little-endian even in `SCO5`**, whose every other multi-byte field is big-endian. A size of 112 is stored `70 00 00 00`; read big-endian that is `0x70000000`, whose low 16 bits are zero, which would bound every name to zero length. Read this one field little-endian regardless of the file.

**The entry table.** The instrument blocks form a table of fixed-size entries starting at offset 194, each holding one instrument: the eight-byte block header, the name, then per-staff tables at the entry's tail. The entry size is not the size the header declares (see 2.2), and is derived from the file instead:

1. the distance between two blocks naming instruments `n` and `m`, divided by `m - n`;
2. failing that, the distance from 194 to a single block naming instrument `n > 0`, divided by `n`;
3. failing that, the span from 194 to the first `PAGE`, `LINE` or `MEAS` block, divided by the instrument count.

Observed sizes are 2158 for Encore 5.0 files, 242 for Encore 4.x files with version byte `0xC4`, and 112 for `0xC2` files `[observed]`. Every entry begins with its name eight bytes in, whether or not it carries a magic, so an instrument whose header was zeroed is still named at `194 + n * entrySize + 8`.

**MIDI program and Key.** Key is a signed byte in semitones matching Encore's "Key" dropdown, 0 meaning it sounds as written and -12 an octave lower, with a range of about -33 to +24; Encore shifts every pitch by it at playback. MIDI program is a 1-indexed General MIDI number. The layout depends on the block size:

| Layout      | Detection                    | MIDI program at       | Key at                |
|-------------|------------------------------|-----------------------|-----------------------|
| large       | size > 250                   | `2278 + n * 2158`     | `2255 + n * 2158`     |
| small, 5.x  | size <= 250, stride size + 8 | `content + size + 76` | `content + size + 53` |
| 4.x total   | size <= 250, stride size     | `content + 60`        | `content + 42`        |
| format 2.50 | `0xA6`, size 64              | `content + 52`        | `content + 42`        |

`content` is the block start plus eight, and `n` is the instrument's sequential index. The layout names are used in the notes below.

- In the **large** layout the table sits after the instrument blocks. The program byte is equivalently 2084 bytes into the entry, which is how to find it when the size field claims 112 on a file whose entries really measure 2158 `[verified]`.
- The size alone cannot tell the layouts apart, so the derived entry size decides: an entry of 2000 bytes or more uses the large offsets `[verified]`.
- A small-layout offset that computes past the end of the entry means the size overstated the content. The per-staff table is then found from the entry's end instead: it finishes 46 bytes before it with version byte `0xC4` and 44 bytes before it with `0xC2`, the two differing only in how many bytes follow the tables `[verified]`.
- The **Encore 4.x total** variant stores the total block size, header included, so the stride equals the size rather than size plus eight, and the content is size minus eight bytes. Its offsets match the 2.50 layout `[verified]`.
- In **format 2.50** each block is 64 bytes with 56 of content. The MIDI byte is at content `+52`, not `+60` as in the total-size variant `[verified]`, and the Key byte at content `+42` is octave-only in practice.

**The MIDI channels** sit immediately before the program, one byte per voice of the staff, stored from zero so the value 9 is the channel Encore's Staff Sheet shows as 10. The table holds eight entries from format 3.05 on and four in 2.50, matching the voices each generation allows, and a staff normally repeats one channel across all of them; the first entry is the staff's channel. Reading it back from the program byte works in every layout above, the large table included, and the values behave as channels do: 0 to 15, one per instrument, ascending through a score `[observed]`.

One value carries meaning beyond routing. General MIDI reserves channel 10 for percussion, where a note number picks a drum rather than a pitch, and Encore honours it: a staff on channel 10 plays as percussion whatever its program says, verified in Encore itself `[verified]`. That makes the channel the one field that marks a percussion part in a file that marks it nowhere else, and it holds where the program lies: a score whose drum staves carry the oboe program of the template they came from puts them on channel 10, and each of those staves writes one repeated pitch, which is percussion notation `[observed]`.

**Percussion.** Percussion tracks report program 1, General MIDI Grand Piano, or no program at all `[observed]`. The kit itself is not stored, so which drums the part uses can only be read from the track name and the noteheads. A program of 113 or above, the General MIDI percussive range, also indicates a drum instrument.

### Files with no instrument blocks

Some `0xC4` files and many `0xC2` files carry none, and the metadata lives in a positional table instead. Three sub-layouts occur, chosen by where the first `PAGE`, `LINE` or `MEAS` block starts and whether a `~~~~` marker, the four bytes `0x7E7E7E7E`, appears in the first kilobyte:

| Sub-layout | Detection                | Name at        | MIDI at         | Key at          |
|------------|--------------------------|----------------|-----------------|-----------------|
| `0xC4`     | block <= 2278, no `~~~~` | `202 + n*112`  | `390 + n*276`   | 367, one only   |
| large      | first block > 2278       | `202 + n*2158` | `2278 + n*2158` | `2255 + n*2158` |
| `0xC2` A   | `~~~~` present           | `314 + k*112`  | `374 + k*112`   | ,               |
| `0xC2` B   | no `~~~~`, block <= 2278 | `202 + n*112`  | `262 + n*112`   | ,               |

In variant A the entry table starts at 281, with the name field at `+33` and the MIDI field at `+93`. Some instruments also have an explicit primary block at `202 + n * 2158`, marked by printable ASCII at that offset, and such an instrument's MIDI byte is 60 bytes past it. In variant B the table starts at 176, with the name field at `+26` and the MIDI field at `+86`, giving the bases 202 and 262, and every instrument is in that one linear table. The name offset for instrument 0 is 202 in every compact layout; the step between names is 2158 when the first block lies beyond 2278 and 112 otherwise.

An instrument that has its own block with an empty name field is genuinely unnamed, and the positional offsets must not be probed for it: the formula would land on unrelated music bytes.

## 5.2 System block (`LINE`)

One per system. A 21-byte header, then one staff entry per staff, 30 bytes each from format 3.05 on.

| Offset | Size | Field                                            |
|--------|------|--------------------------------------------------|
| `+10`  | 2    | 0-based index of the first measure in the system |
| `+12`  | 1    | number of measures in the system                 |

In `SCO5` the single-byte measure count reads as zero, because the meaningful byte sits in the other half of a wider big-endian field; the first-measure index stays correct.

Each 30-byte staff entry:

| Offset | Size | Field                                                             |
|--------|------|-------------------------------------------------------------------|
| `+13`  | 1    | staff display size, 0-indexed selector                            |
| `+14`  | 1    | clef type; see 6.5                                                |
| `+15`  | 1    | key signature; see 7.5                                            |
| `+16`  | 1    | page-row counter for the system, not the page number              |
| `+19`  | 1    | visibility: 0 hidden, non-zero visible                            |
| `+20`  | 1    | staff type: 0 melody, 1 tablature, 2 single-line percussion       |
| `+21`  | 1    | packed staff index: bits 0-5 instrument, bits 6-7 staff within it |

The size selector at `+13` is 0-indexed and runs through the same four scales the header selector does, so 0 is 60%, 1 is 75%, 2 is 100% and 3 is 130%. The corpus says which one is the untouched default: value 2 holds in the great majority of files in every generation from 3.05 on, with 1 next and 3 rarest, which is the shape of a field left alone at 100% `[observed]`. It is populated in both 4.x and 5.x files and is the authoritative per-staff size, with the header selector as a global fallback. The packed index at `+21` uses the same encoding as the element staff byte in 6.2. The staff type at `+20` is constant across every system for the same staff.

**The key field means nothing on a tablature staff.** A tab staff draws fret numbers and no key signature, so Encore never renders the byte there and does not keep it consistent. Across the conversion pairs the key field differs in a handful, and every one of those is a file with a tablature staff, always on the tab staff and always after the first system `[verified]`. Read the key from the notation staff.

### Format 2.50 systems

That generation reports staves-per-system as zero and uses a **22-byte** staff entry. Each entry carries a `0x0E 0xFC` marker at offset 16 which bounds the run, so the staff count is the number of consecutive entries carrying it. The four fields that matter sit together just before the marker:

| Offset | Size | Field                                                    |
|--------|------|----------------------------------------------------------|
| `+12`  | 1    | staff display size, 0-indexed, same four scales as above |
| `+13`  | 1    | clef type; see 6.5                                       |
| `+14`  | 1    | key signature; see 7.5                                   |
| `+15`  | 1    | staff index within the system                            |
| `+16`  | 2    | the `0x0E 0xFC` marker                                   |

So this generation keeps size, clef and key in the same order the 30-byte entry does, only tighter and without the fields around them. The key was the first to be read here `[observed]`; the other two were found later, against Encore itself: files whose entries carry sizes 0, 1 and 2 open in Encore showing staff sizes 1, 2 and 3, which is the same selector counted from zero `[verified]`. The clef reads the same way: a MusicTime file whose entries carry a mix of 0 and 1 opens in Encore with the matching run of G and F clefs, and the byte holds the same values in every one of its systems `[verified]`.

The header byte at `0x8D` is not this field. It agrees with the first staff's size in most files and disagrees in others, so it is a fallback at best, and the entries are the authority.

## 5.3 Page block (`PAGE`)

One block per page, 26 bytes of content in every generation, saying which systems the page holds. Only three of the twenty-six are ever anything but zero, and the page size, orientation and scale come from the printer block in 5.7 rather than from here.

| Offset | Size | Field                                                        |
|--------|------|--------------------------------------------------------------|
| `+0`   | 2    | index of the first system on the page, 0-based               |
| `+2`   | 2    | number of systems on the page                                |
| `+12`  | 2    | staff rows on the page: the systems times the staves in one  |

The three agree with the rest of the file and with each other. Across the corpus the system counts of a score's pages add up to its number of system blocks in every file, the first field of each page equals the systems of all the pages before it in every multi-page file, and `+12` is `+2` times the staves-per-system of the header in all but a few, the exceptions being scores whose systems do not all carry every staff `[verified]`.

A parser can skip the block: everything in it is derivable from the system blocks, which carry the page each system belongs to. It is worth reading only as a check on those.

### Tab tuning

Each track carries its own tab tuning in the eight bytes at the end of its instrument block's content, immediately before the next block's header. The declared size counts those eight trailing header bytes, so the tuning sits sixteen bytes before the block's nominal end; for the last track this places it just before the first `PAGE` block.

The eight slots hold the open-string MIDI pitches from lowest to highest, then pad bytes: `0x7F` when the tuning was customised and `0x58` for the untouched six-string guitar default. The string count is the number of leading non-pad slots, so a four-string mandolin reads `37 3E 45 4C 7F 7F 7F 7F`, G3 D4 A4 E5, and a six-string guitar reads `34 39 3E 43 47 4C 7F 7F`, the tab-display pitches an octave above concert.

Because the tuning is per track, a file mixing differently tuned tab staves carries a distinct tuning in each block, and each tab staff must use its own. A near-identical block also appears once in the `SCO5` header, around `0x1A1`, always the guitar default padded with `0x58`; that copy is a global default, not a per-staff tuning.

Encore stores no per-note string or fret, only the tuning: the fingering is computed from the pitches.

A tab staff is normally a derived view, its notes living on the paired notation staff while its own element stream carries only rests. The exception is a tab-only score, where Encore materialises the notes as pitch-bearing rest elements: such an element uses the rest layout but sets bit `0x8` of the voice nibble and stores the MIDI pitch at `+15`, the slot a note uses. It has no face value, and its duration comes from the gap rule in 7.1. A genuine rest has a voice nibble below 4 and no pitch, so the voice bit tells them apart.

## 5.4 Measure block (`MEAS`)

A fixed header, then an element stream terminated by a tick of `0xFFFF`. The header is 54 bytes from format 3.05 on and 26 bytes in 2.50, and the elements begin right after `[verified]`. Remember from 2.1 that the block's declared size excludes this header.

| Offset | Size | Field                                                    |
|--------|------|----------------------------------------------------------|
| `0x00` | 2    | quarter-note BPM, applying forward until the next change |
| `0x02` | 1    | time-signature glyph                                     |
| `0x04` | 2    | beat ticks, 240 for x/4                                  |
| `0x06` | 2    | total ticks in the measure                               |
| `0x08` | 1    | time-signature numerator                                 |
| `0x09` | 1    | time-signature denominator                               |
| `0x0C` | 1    | start barline type                                       |
| `0x0D` | 1    | end barline type                                         |
| `0x0F` | 1    | volta bitmask                                            |
| `0x1A` | 4    | repeat-mark field; the low byte is the type              |

The remaining bytes hold layout data: measure width, x-offsets and a UTF-16 "Writer" tag. An unrelated field at `+0x18` holds a constant 200 in `0xC4` files and is not a BPM.

**BPM is always quarter-note BPM** regardless of meter. In 3/8 or 5/8 Encore's interface shows eighth-note BPM, twice the stored value, but the file stores the quarter.

**Beat ticks are not a reliable basis for the whole-note count.** They are 240 for x/4, 120 for x/8, 480 for x/2 and 360 for a compound dotted-quarter beat, but some builds store 240 even for 2/2, where 480 is correct. Compute the whole note as described in 7.2 instead.

**Time-signature glyph.** `0x00` means show the numerator and denominator as digits; `0x43` is common time in Encore 3.x and 4.x and `0x63` the same in 5.x. Other values, `0x01`, `0x02`, `0x06` and `0x07`, appear on unusual meter strings `[observed]` and are treated as numeric.

**Barlines.**

| Value | Meaning       |
|-------|---------------|
| 0     | normal        |
| 2     | repeat start  |
| 3     | double, left  |
| 4     | repeat end    |
| 5     | final         |
| 6     | double, right |
| 8     | dotted        |

A non-repeat divider that visually sits between two measures is stored on the **right** measure's start barline, not the left one's end barline. So a start barline of type 3, 6 or 8 describes the divider at the end of the preceding measure, while a repeat start genuinely belongs to its own measure.

**Voltas.** Byte `0x0F` is a bitmask: bit `n` set means the measure belongs to ending `n + 1`. Encore sets the same mask on every measure inside an ending, not only the first, so consecutive measures sharing a non-zero mask form one bracket. There is no stored count of passes for a repeat: it is the highest ending number among the brackets, so a "1.-3." ending followed by a "4." plays four times, and a repeat with no alternate endings plays twice.

**Repeat marks**, in the low byte of `0x1A`:

| Byte   | Meaning                        |
|--------|--------------------------------|
| `0x80` | D.C. al Coda                   |
| `0x81` | D.S. al Coda                   |
| `0x82` | D.C. al Fine                   |
| `0x83` | D.S. al Fine                   |
| `0x84` | D.S.                           |
| `0x85` | To Coda, the point jumped from |
| `0x86` | Fine                           |
| `0x87` | D.C.                           |
| `0x88` | Segno                          |
| `0x89` | Coda, the point jumped to      |

`0x85` and `0x89` are a pair. Ornament subtype `0xA5` is a parallel encoding of the same To Coda point.

## 5.5 Text block (`TEXT`)

Payloads referenced by staff-text ornaments, indexed by the ornament's entry index. A file may hold several: each later block is a part-view copy with the same strings reordered, and the ornament index is relative to the first.

| Offset | Size | Field               |
|--------|------|---------------------|
| `+0`   | 2    | sync, `0x0000`      |
| `+2`   | 2    | entry count         |
| `+4`   | 4    | total content bytes |
| `+8`   | var  | the entries         |

Each entry is a two-byte payload size, then a rich-text run header, then the text, then a `0x00 0x00` terminator possibly followed by padding. The run header, measured from the payload start:

| Offset      | Size      | Field                                    |
|-------------|-----------|------------------------------------------|
| `+0`        | 2         | run-offset table count                   |
| `+2`        | 2         | descriptor count                         |
| `+4`        | count * 4 | run-offset table, per-run char positions |
| after table | count * 6 | style descriptors                        |
| after that  | var       | the displayed text                       |

So the text starts at `4 + tableCount * 4 + descCount * 6`. Both counts are genuine, an entry can carry two descriptors as well as one, and a single-run single-descriptor comment puts the text at offset 14. When either count is zero or the computed offset runs past the entry, fall back to 14.

In format 2.50 there is no run header at all: the text starts at payload offset 0 and is NUL-terminated Latin-1.

**Line separators.** `0x04 0x00` separates lines within one comment and is not the terminator; every line including the last is followed by it, and the whole string ends at `0x00 0x00`. Text length is bounded by that terminator, not by the payload size, since some entries carry padding after it. Dynamics are not here: they are ornament subtypes.

## 5.6 Title block (`TITL`)

One per page, the later ones usually empty. It holds a title, 2 subtitles, 3 instructions, 4 authors, 2 headers, 2 footers and 6 copyright lines, in that order, each a fixed-width slot. Multiple non-empty slots in one category render as stacked lines. In a header or footer, `#P` is the page number, `#D` the date and `#T` the time.

The encoding is decided by the block size `[verified]`:

| Size        | Encoding  | Bytes per line | Bytes per copyright line |
|-------------|-----------|----------------|--------------------------|
| under 5000  | Latin-1   | 96             | 160                      |
| 10000 or up | UTF-16 LE | 1056           | 1056                     |

The two layouts differ by roughly tenfold, 2426 bytes against 21242, so the size resolves them unambiguously. Sizes between 5000 and 9999 are rare and not disambiguated this way; fall back to the file's instrument-name encoding.

Each line is a 30-byte prefix then the text, NUL-terminated and zero-padded, with anything after the NUL being debris from earlier edits. The text field is 1026 bytes in UTF-16 for every line. In Latin-1 it is 66 bytes for the title, subtitles, instructions, authors, headers and footers, but **130 bytes for the six copyright lines** `[verified]`: reading those at 66 finds the first and then lands inside its own text, so the rest come back empty. Both totals close on the same 120-byte pad. Prefix byte `+14` is the alignment: `0x02` right, `0x04` left, `0x06` centre, `0x00` on other line types.

## 5.7 Printer block (`PREC`)

Printer and page-setup state, from 132 bytes to several kilobytes, in one of two encodings. It is present in almost every file across all versions while the margins block is not, so page size, orientation and notation scale come from here even for the older generations.

On Windows it is a `DEVMODE` structure: a device-name field of 32 bytes ANSI or 64 bytes UTF-16, then fixed fields relative to that base.

| Offset from base | Field        | Meaning                                   |
|------------------|--------------|-------------------------------------------|
| `+12`            | orientation  | 1 portrait, 2 landscape                   |
| `+14`            | paper size   | Windows paper enum                        |
| `+16`            | paper length | tenths of a millimetre, custom sizes only |
| `+18`            | paper width  | tenths of a millimetre, custom sizes only |
| `+20`            | scale        | notation-size percent, 100 being default  |

Detect the variant by reading the orientation at both bases and keeping the one that is 1 or 2. Paper sizes are 1 Letter, 5 Legal, 7 Executive, 8 A3, 9 A4, 11 A5, 12 B4, 13 B5; when the value is custom or unknown, fall back to the length and width. Across the corpus the scale is a clean distribution of round percentages and the paper size is dominated by A4 and Letter `[observed]`.

On macOS it is an `NSPrintInfo` XML plist, beginning `<?xml`. Paper comes from `PMTiogaPaperName` or `PMPaperName`, orientation from `PMOrientation` with 1 portrait and 2 landscape, and notation scale from `PMScaling` as a fraction. The plist carries only the printer's imageable rectangles, not document margins, so `SCO5` page margins are not available from any block.

## 5.8 Margins block (`WINI`)

Page margins. Optional: a file never taken through Encore's Page Setup has none `[observed]`.

| Offset | Size | Type   | Field                                     |
|--------|------|--------|-------------------------------------------|
| `+0`   | 24   | bytes  | window and screen data, not page geometry |
| `+24`  | 4    | int32  | top margin                                |
| `+28`  | 4    | int32  | left margin                               |
| `+32`  | 4    | int32  | bottom edge of the printable area         |
| `+36`  | 4    | int32  | right edge of the printable area          |
| `+40`  | 2    | uint16 | flags, observed as 1                      |

The content is 42 bytes; some older files omit the trailing word, giving 40, and both are valid.

**The unit varies** `[observed]`. Encore 5.x stores typographic points, 1/72 inch. Earlier versions store screen pixels at the monitor's DPI, around 84, the exact value depending on the screen the file was last saved on; the displayed margins are still inches, only the stored unit is device pixels.

Tell them apart by magnitude: when an edge exceeds the page dimension expressed in points, the block is in pixels. In that case the page dimensions are not stored and must be recovered from margin symmetry, `pageWidth = rightEdge + left` and `pageHeight = bottomEdge + top`, then matched against standard paper sizes. All ISO A-series sizes share the same ratio, so at most two fall in a plausible DPI range and the right one minimises the difference between the horizontal and vertical DPI.

Encore stores `round(inches * 72)` and displays `floor(points / 72 * 1000) / 1000`, so 0.100 inches stores as 7 points and displays back as 0.097. All four values zero means no stored margins. The Page Setup dialog may still show a non-zero margin for a side whose stored value is zero: that is the printer's hardware non-printable zone, read at display time and not in the file.

---

# 6. The element stream

## 6.1 Framing

Every element opens with the same five bytes.

| Offset | Size | Field                                             |
|--------|------|---------------------------------------------------|
| `+0`   | 2    | tick within the measure; `0xFFFF` ends the stream |
| `+2`   | 1    | type in the high nibble, voice in the low nibble  |
| `+3`   | 1    | size, the total element length from `+0`          |
| `+4`   | 1    | staff byte                                        |

The body begins at `+5`. **In format 2.50 the on-disk slot is twice the declared size** `[verified]`, so the stride from one element to the next is `size * 2` there and `size` everywhere else.

| Type | Element             |
|------|---------------------|
| 0    | none                |
| 1    | clef                |
| 2    | key change          |
| 3    | tie                 |
| 4    | beam                |
| 5    | ornament            |
| 6    | lyric               |
| 7    | chord symbol        |
| 8    | rest                |
| 9    | note                |
| 11   | MIDI control change |

Type 7 is a chord *symbol*, the harmony text above the staff, not a chord of notes.

## 6.2 Staff and voice

The staff byte at `+4` names the destination staff with the same encoding the system block uses for its packed index.

| Bits | Mask   | Meaning                                                   |
|------|--------|-----------------------------------------------------------|
| 0-5  | `0x3F` | instrument index, 0-based and sequential                  |
| 6-7  | `0xC0` | staff within that instrument: 0 first, 1 second, 2-3 more |

The raw byte equals the packed index the system block stores for the target staff, and is resolved to a global staff by looking it up there. The instrument index in the low bits equals the system slot only when every instrument has one staff; for multi-staff instruments it does not.

For an instrument with several staves, piano, harp or organ, all staves share one element stream and the destination is the high bits of this byte. Every note of the instrument carries that instrument's sequential index in the low bits, whatever the system layout. In a two-staff instrument voices 0 and 1 belong to the first staff and voices 2 and 3 to the second, renumbered 0 and 1 there.

The low nibble of the type byte is the voice, 0 to 7. Beyond the four ordinary voices, two special uses occur:

- **Voice 4** is a marker with two meanings, resolved by the system's multi-staff configuration. On a grand-staff instrument it marks second-staff content and combines with the `0x40` bit of the staff byte. On a single-staff instrument it is a genuine second melodic voice, or, in a part whose only line is stored as voice 4, the sole voice.
- **Voices 5, 6 and 7** are additional voices on the element's own staff and do not indicate a second staff.

A voice may carry more than one interleaved MIDI stream, for instance from live recording: a backwards tick, or a non-chord event arriving after the voice is full, marks a fresh stream. A voice may also carry a redundant plain rest at the same tick as a real note, because Encore writes a rest slot for the voice even where the note sits. That differs from same-tick tuplet members, which are genuine sequential members.

## 6.3 Note

The note carries a face value, a MIDI pitch, grace and tie flags, an optional tuplet ratio and optional articulations.

**The face value at `+5`** packs two things: the high nibble is the notehead, 0 normal, 1 diamond, 2 triangle up, 3 square, 4 cross, 5 X in a circle, 6 plus, 7 slash, 8 large open diamond, 9 invisible; the low nibble is the duration, 1 whole through 8 for a 128th.

**The notehead names are not the same in every generation**, in the way the articulation codes are not either. The list above is the one the 3.05 and later files use. In format 2.50 and 2.62 the value 3 is a cross rather than a square: a MusicTime score whose percussion staves carry nibble 3 on every note, against nibble 0 on its pitched staves, opens in Encore with crosses throughout `[verified]`. The other values of that generation have not been sampled, so a reader should treat the rest of the list as the same until a file says otherwise.

**The low two bits of byte `+14` are the dot count**, 0, 1 or 2, and the rest of the byte is layout. The count is what Encore draws and what its own MusicXML export writes, and it holds in every generation `[verified]`. It never states 3: a triple-dotted note carries 0 there and is only visible in the durations, see 7.3.

The 4.20 layout, a 28-byte note:

| Offset | Size | Field                                                      |
|--------|------|------------------------------------------------------------|
| `+5`   | 1    | face value                                                 |
| `+6`   | 1    | grace flags, first byte                                    |
| `+7`   | 1    | grace flags, second byte                                   |
| `+10`  | 1    | x-offset, the notated column; see 7.7                      |
| `+12`  | 1    | staff position, diatonic steps from C4                     |
| `+13`  | 1    | tuplet: actual count in the high nibble, normal in the low |
| `+14`  | 1    | layout byte, see above                                     |
| `+15`  | 1    | MIDI pitch                                                 |
| `+16`  | 2    | playback duration in ticks                                 |
| `+19`  | 1    | velocity                                                   |
| `+20`  | 1    | options                                                    |
| `+21`  | 1    | accidental glyph                                           |
| `+24`  | 1    | articulation above                                         |
| `+26`  | 1    | articulation below                                         |

The staff position at `+12` counts diatonic steps from middle C, so C4 is 0 and A5 is 12. On a pitched staff it is a legacy display hint; on a percussion staff it encodes the visual line instead, as `line = max(-4, 10 - position)`, putting A4 on the middle line `[observed]`.

### The note across generations

Sizes 22 and 24 are not two variants of one layout: they are the two sides of the two-byte insertion from 1.4. A 22-byte note is format 3.05 and a 24-byte note is 3.07, with its body from `+8` onward two bytes later. The 28-byte note is the 24-byte one with four bytes appended `[verified]`.

| Field             | 3.05  | 3.07 and later |
|-------------------|-------|----------------|
| face value        | `+5`  | `+5`           |
| grace 1           | `+6`  | `+6`           |
| grace 2           | `+7`  | `+7`           |
| x-offset          | `+8`  | `+10`          |
| staff position    | `+10` | `+12`          |
| tuplet            | `+11` | `+13`          |
| layout byte       | `+12` | `+14`          |
| MIDI pitch        | `+13` | `+15`          |
| playback duration | `+14` | `+16`          |
| velocity          | `+17` | `+19`          |
| options           | `+18` | `+20`          |
| accidental glyph  | `+19` | `+21`          |

Verified by converting a 3.05 file in Encore 4.5 and matching the streams: across every paired note the x-offset, tuplet and layout bytes agree at the 3.05 offsets, and at the 3.07 offsets they agree in a couple of percent `[verified]`.

This is why a size-based test on `+15` appears to work and is still wrong. In a 22-byte note `+15` is the low byte of the playback duration, usually small, while `+13` is the real pitch, so reading the pitch that way recovers it. But the tuplet at `+11` is then never read at all, which is exactly what makes a format 3.05 score look as though it has no explicit tuplets.

**Articulations grow the note.** The two slots sit immediately past the base note, so a note carries them only by being longer: two bytes more for the mark above, four for both. They move with the generation like everything else `[verified]`.

| Generation | Base | Mark above | Mark below | Sizes seen                         |
|------------|------|------------|------------|------------------------------------|
| 3.05       | 22   | `+22`      | `+24`      | 22 plain, 24 with one              |
| 3.07 on    | 24   | `+24`      | `+26`      | 24 plain, 26 with one, 28 with two |

Measured across the corpus: on 3.05 size-24 notes the byte at `+22` holds an articulation value in 99.8% of cases; on 3.07 the same byte never does, while `+24` is one in 95.7% of size-26 notes and `+26` in 100% of size-28 notes. A note at its base length has no slot at all, so reading one there yields an unrelated byte.

A layout byte of `0xC0` is characteristic of size-24 notes in the 3.07 generation; its low two bits are clear, so the note carries no dot.

### The compact note, format 2.50

Ten bytes declared, twenty on disk.

| Offset | Field                                  |
|--------|----------------------------------------|
| `+5`   | face value                             |
| `+6`   | grace flags                            |
| `+7`   | tuplet, `0x32` for 3:2, `0x54` for 5:4 |
| `+8`   | horizontal position across the measure |
| `+9`   | staff position, signed                 |
| `+11`  | MIDI pitch, absolute                   |
| `+12`  | playback duration in ticks, uint16     |

**The byte at `+8` is where the note sits across its measure**, counted in 256ths of the measure regardless of how wide the measure is drawn, which is this generation's answer to the x offsets the later ones store in pixels. It follows the tick: against the position the tick implies, `round(tick * 256 / measure ticks)`, it lands exactly on nearly nine notes in ten and within eight on almost all the rest `[verified]`. The remainder are the notes an engraver moved, which is what makes the byte worth reading at all rather than recomputing. The same field has not been found in the ornaments of this generation, so it does not help their endpoints; see 9.

The staff position at `+9` is a signed count of diatonic steps from middle C, so 0 is C4, 5 is A4 and -1 is B3, and an altered note shares the position of its natural `[verified]`. The later generations keep that field at `+12`, which here is the first byte of the playback duration and always reads `0x80`.

**The compact note has no dot control and no velocity, options or accidental slots.** The body ends at `+19`, and the slots later generations keep past that point belong to the following element: `+20` is its tick low byte and `+21` its tick high byte, which is why an accidental byte read there takes only the values 0, 1, 2, 3 and 255, the tick pages of a measure plus the end marker `[verified]`.

A compact note carrying one articulation is written as size 11, a 22-byte slot, with the same layout and the articulation byte at `+18`, where `0x20` is a fermata `[verified]`.

### Grace and cue notes

The flags come from Encore's Grace and Cue Note dialog `[verified]`.

| Bit           | Meaning                                                      |
|---------------|--------------------------------------------------------------|
| grace1 `0x20` | small note, grace or cue; an ordinary note leaves it clear   |
| grace1 `0x10` | member of a beamed grace group                               |
| grace1 `0x40` | attribute of the top chord member, unrelated to grace or cue |
| grace2 `0x04` | slash, marking an acciaccatura                               |
| grace2 `0x01` | muted, playback off                                          |

Only the high nibble of `grace1` carries these. Its low nibble is a separate field, the tie flags of 6.7.

A beamed grace group is a melodic run of separate grace notes joined by a beam, so two beamed graces at the same tick stay two grace notes with two stems, not one stacked grace chord.

Among small notes, a slash marks an acciaccatura; a small note without one is either an appoggiatura, when it ornaments an adjacent principal note, or a cue, when it stands alone at full value. The two are byte-identical and separated only by context. A cue keeps its full beat in the measure, drawn small and muted by default, while a grace occupies no measure time and borrows from an adjacent note. Any note can be muted, and a cue can be un-muted.

**The bit means the same in every generation.** A score saved by Encore 3 and saved again by Encore 5 carries the identical flags on the identical notes, cue passages included, so nothing about the small note changed with the format `[verified]`. Where a whole passage is small the notes are cues, at full value and usually silent: the mute bit accompanies the small bit in almost every such note in the corpus, while a small note carrying the slash is a grace `[observed]`.

The mute bit is independent of size: it is Encore's per-note Play switch and always means silence, whatever the small-note bit says. Whole passages are written this way `[verified]`.

A slur can begin on a grace note stored at the same tick as its parent chord, since a grace shares its parent's written tick, so such a slur has no distinct start tick. With version byte `0xC4` Encore serialises the main note before its grace at the same beat; with `0xC2` the grace comes first `[verified]`.

**In format 2.50 grace notes occupy real tick positions** rather than sharing the main note's. A grace at a real tick pushes the following notes forward, so the last real note of the group ends with a gap to the measure end smaller than its face value: the grace borrowed that time. Inner graces, whose grace1 masked with `0x30` is `0x10`, follow a leading grace whose value is `0x20`, and have a strictly larger face-value number, meaning a shorter note.

## 6.4 Rest

| Offset | Size | Field                                                   |
|--------|------|---------------------------------------------------------|
| `+5`   | 1    | face value, same encoding as a note                     |
| `+10`  | 1    | x-offset                                                |
| `+13`  | 1    | tuplet, same encoding as a note                         |
| `+14`  | 1    | layout byte, low two bits the dot count                 |
| `+15`  | 1    | multi-measure rest count, only when the size exceeds 15 |

When the count at `+15` is above 1, the single measure block stands for that many consecutive empty display measures, which Encore draws as one symbol with the count above it. Multi-staff files emit one rest per staff, all carrying the same count.

**The compact rest, format 2.50**, is 7 bytes declared and 14 on disk:

| Offset | Field                     |
|--------|---------------------------|
| `+5`   | face value                |
| `+10`  | x-offset                  |
| `+12`  | duration in ticks, uint16 |

The duration at `+12` is the nominal value, unscaled: 960 for a whole, 480 a half, 240 a quarter, 360 a dotted quarter, 120 an eighth `[verified]`. This rest carries no tuplet, no dot control and no multi-measure count. The two slots the later generations keep at `+13` and `+14` are, here, the high byte of that duration and the first byte of whatever element follows.

## 6.5 Clef

Byte `+5` holds the type, the same encoding the system block's staff entry uses. The remaining bytes are padding.

| Value | Clef       |
|-------|------------|
| 0     | G, treble  |
| 1     | F, bass    |
| 2     | C3, alto   |
| 3     | C4, tenor  |
| 4     | G 8va      |
| 5     | G 8vb      |
| 6     | F 8vb      |
| 7     | percussion |
| 8     | tablature  |

**A clef does not take effect at its own tick.** It applies before the note or rest that physically follows it in the stream on the same staff, because Encore frequently stamps it with an earlier tick. When a clef is the last element on its staff in a measure it is a cautionary clef, effective on the next measure's downbeat and drawn just before the current measure's final barline.

## 6.6 Key change

Size 6. Byte `+5` is the key index from 7.5.

## 6.7 Tie

Size 7 in format 2.50, 16 in 3.05, 18 from 3.07 on. A tie marks only the start note: there is no matching stop element, and the receiver is the next note of the same pitch on the same staff and voice.

From 3.05 on, byte `+5` is a signed arc curvature, the vertical bow, and **not a bitfield**; byte `+6` is the tie-start flag.

| Byte `+5` | Signed | Arc        |
|-----------|--------|------------|
| `0x02`    | +2     | curve down |
| `0x04`    | +4     | curve down |
| `0xFE`    | -2     | curve up   |
| `0xFC`    | -4     | curve up   |

All four mark a real outgoing tie. Treating `+5` as a bitfield silently drops the equally valid `0x04`.

**The arc endpoints are the authoritative signal.** They are uint16 fields and they sit two bytes earlier in the 16-byte form, following the same shift as everything else:

| Field                                | 16-byte | 18-byte |
|--------------------------------------|---------|---------|
| arc start x, measure-relative pixels | `+8`    | `+10`   |
| arc end x                            | `+10`   | `+12`   |
| staff position of the source note    | `+12`   | `+14`   |

The bytes between `+7` and the arc pair are always zero in both forms, which is where the two extra bytes went `[observed]`. The source position disambiguates which chord member the tie leaves.

**Read the arc x as uint16, not as a byte.** In little-endian files the significant byte comes first, so a byte-wide read happens to work; in big-endian `SCO5` it comes second and a byte-wide read yields zero for every tie in the file. Measured over every tie in the corpus, little-endian and big-endian alike, the first has `+10` non-zero in 100% of ties and `+11` in none, and the second is the exact mirror `[verified]`.

This does not fail gracefully. A zeroed pair reads as start equal to end, which is the decorative case below, so every tie in the file is classified as decorative and dropped.

- Start below end is a genuine left-to-right span, a real forward tie whatever `+5` says.
- Start equal to end is zero horizontal extent, an intra-chord decorative arc where Encore connects two chord notes vertically. It is not a forward tie **unless** byte `+6` has bit 7 set, which marks a cross-measure tie whose destination is in the next measure and for which Encore stores the two equal as a placeholder. Decorative arcs often appear in groups of two to four identical copies at the same tick.

**The compact tie, format 2.50**, is 7 bytes in a 14-byte slot, and its two flag bytes are **the other way round** from every later generation `[verified]`:

| Offset | Field                                                        |
|--------|--------------------------------------------------------------|
| `+5`   | tie-start flag, `0x80` outgoing, the role `+6` plays later   |
| `+6`   | arc direction, the same four-way encoding `+5` carries later |
| `+8`   | where the arc starts across the measure, in 256ths           |
| `+9`   | staff position of the source note, duplicated at `+11`       |

Measured over every compact tie in the corpus, `+6` holds a value from the four-way direction vocabulary in 99.4% of them and `+5` in none, while `+5` is `0x80` in 85% and zero in the rest. The source position at `+9` matches a converted file exactly.

**The arc is one coordinate here, not a pair.** Byte `+8` is where the tie starts across its measure, in the same 256ths the note carries at its own `+8`: across every compact tie in the corpus that begins away from a barline it sits within six of the position the tick implies in 95% of them `[verified]`. The second coordinate the later generations store is absent, which is why this generation tells a decorative arc from a forward tie by its flags rather than by comparing two numbers.

The arc x pair has not been located in this generation. In the one file where a conversion gives ground truth, the bytes at `+8`, `+10` and `+12` are constant across every tie while the converted arc positions vary, so they are not it, and x does not survive the conversion either way because this generation stores screen pixels. Without the arc span there is no equivalent of the authoritative test, so tie direction here rests on the two flag bytes alone.

### The note's own tie flags

A tie is recorded twice over, and the second record is on the note itself: the low nibble of `grace1`, at note offset `+6`, is a two-bit field saying whether the note is an end of a tie `[verified]`.

| Low nibble | Meaning                                                          |
|------------|------------------------------------------------------------------|
| `0`        | no tie touches the note                                          |
| `1`        | the note starts a tie                                            |
| `2`        | the note ends a tie                                              |
| `3`        | both: a note in the middle of a chain of tied notes              |

The reading is confirmed by what surrounds each note. A note flagged `1` is followed by a note of the same pitch in about three quarters of cases and preceded by one in a sixth; a note flagged `2` is the mirror image; a note flagged `3` has both neighbours at its pitch in about four cases in five `[verified]`.

The field means the same in all four generations. A nibble of `1` sits on 1.9% of the notes of a format 2.50 file, 2.1% in 3.05 and 3.07 and 3.9% in 4.20, and in every one of them the note that follows repeats its pitch in three cases out of four `[verified]`. The high nibble of the same byte carries the grace flags of 6.3, in 2.50 as everywhere else, and the two halves do not interfere.

**It is mostly redundant, and that is the point.** Between 93% and 96% of the notes flagged `1` also have a `TIE` element at their tick, so almost always the two records agree. The remainder is what makes the field worth reading: those notes carry no `TIE` element at all, and their tie is recorded nowhere else. They behave like the rest, with a following note of the same pitch about as often, so they are ties and not noise.

A reader should therefore take a tie as starting when either record says so, and let the receiver rule of this section do the rest.

## 6.8 Ornament

Type 5, variable size, and the busiest element in the format: it covers hairpins, slurs, trills, tempo marks, dynamics, staff text, breath marks, tremolos, fingerings, bows and articulation-like marks, told apart by the subtype at `+5`.

| Offset | Size | Field                                                |
|--------|------|------------------------------------------------------|
| `+5`   | 1    | subtype; see 8.2                                     |
| `+10`  | 1    | x-offset, the start x within the measure             |
| `+12`  | 2    | signed y: negative below the staff, positive above   |
| `+16`  | 1    | forward measure count for slurs, version byte `0xC2` |
| `+18`  | 1    | forward measure count to the end measure             |
| `+20`  | 1    | end x within the target measure                      |
| `+26`  | 1    | hairpin direction, bit 0: 0 crescendo, 1 diminuendo  |
| `+28`  | 1    | tempo beat unit                                      |
| `+30`  | 2    | tempo BPM                                            |
| `+32`  | 1    | staff-text entry index, when the size is at least 33 |

**A mark attached to a note does not always carry that note's tick.** An accent, a bow or a similar mark on the **last** note of a bar is stored at the tick where that note ends, so nothing starts there: a bar of five eighths carries its accent at 600 while the note it belongs to begins at 480 `[verified]`. The x-offset settles it, since it sits a few pixels past the note's own, 99 against 91 in that bar. A reader that takes an empty tick as a mark belonging elsewhere loses it.

**Tempo beat unit.** The low seven bits are the note value, 0 whole, 1 half, 2 quarter, 3 eighth and so on, and bit `0x80` marks it dotted, so `0x02` is a quarter and `0x82` a dotted quarter. A value of 0, or an out-of-range byte from an older generation, means no explicit unit.

**Tempo BPM** sits at `+30` expressed in that unit, with one exception: older `0xC2` files store the BPM directly at `+28` and leave a constant unrelated byte, observed as `0x34`, in the `+30` slot `[observed]`. Tell them apart by `+28`: a valid beat-unit code means the BPM is at `+30`, otherwise `+28` is itself the BPM. In that older layout the per-mark beat unit is at `+26`, which matters in compound meters.

**Staff-text entry index** is at `+32` only when the element is at least 33 bytes. In shorter ornaments, notably the size-32 staff texts of `0xC2` files, it is read from `+30`, sharing the slot with the tempo.

**Hairpin direction.** Bit 0 of `+26`: 0 crescendo, 1 diminuendo. Encore 5 also sets bit 1, giving `0x02` and `0x03` where legacy files use `0x00` and `0x01`, so test the bit rather than comparing to zero.

**The ornament y.** Negative is below the staff and positive above. A dynamic dragged onto the staff above the one that owns it keeps its owner's staff byte and flips its y positive. A dynamic or staff text whose tick exceeds the measure's total ticks is a section-end marker. A file can carry two dynamics at the same tick and x on one staff and voice, an identical pair or a score-view and part-view pair differing only in y; Encore renders one per beat.

### The ornament across generations

The offsets above are the 3.07 layout. A 3.05 ornament, of size 14, 26, 32 or 36, has every field from `+6` onward two bytes earlier `[verified]`:

| Field                  | 3.05  | 3.07 and later |
|------------------------|-------|----------------|
| subtype                | `+5`  | `+5`           |
| x-offset               | `+8`  | `+10`          |
| y, signed 16-bit       | `+10` | `+12`          |
| slur forward count     | `+14` | `+16`          |
| forward measure count  | `+16` | `+18`          |
| end x                  | `+18` | `+20`          |
| hairpin direction      | `+24` | `+26`          |
| tempo beat unit        | `+26` | `+28`          |
| tempo BPM              | `+28` | `+30`          |
| staff-text entry index | `+30` | `+32`          |

Each of the first six was verified against a 4.5 conversion in every paired ornament, and in none of them at the later offsets `[verified]`.

**The compact ornament, format 2.50**, declares sizes 5, 12 and 15 in a slot of twice that, and does not follow the field order above at all. Its layout was established by converting a file in Encore 4.5 and matching the streams `[verified]`:

| Offset | Field                                                          |
|--------|----------------------------------------------------------------|
| `+5`   | subtype, the same encoding as every other generation           |
| `+9`   | y, a **signed byte**: positive above the staff, negative below |
| `+14`  | forward measure count to the end measure                       |
| `+28`  | staff-text entry index                                         |

The y and the measure count apply to every subtype, not only to staff text, and both relocated fields sit outside the declared size but inside the doubled slot, as the text index already did.

The y is a byte, not a halfword. Reading `+8` as a 16-bit value happens to preserve the sign, because `+9` is that halfword's high byte, but the magnitude becomes `y * 256` plus whatever `+8` holds: 3840 instead of 15, -4865 instead of -20. Anything that only tests the sign survives; anything that uses the value does not.

Over every compact slur and hairpin start, the forward count read at `+14` lands inside its score every time, while read at `+18` 105 of them, 16.5%, point past the last measure `[verified]`.

**The start x is at `+8`**, the same field the later generations keep at `+10` and in the same unit, so a compact spanner carries both ends after all: this byte and the forward measure count at `+14`. A set of compact scores opened in Encore 4.5 and saved again gives a few hundred ornaments that pair one to one by measure, staff and tick, and the byte survives the conversion unchanged in over half of them, within eight in nearly two thirds, while the order of the ornaments within a measure agrees in nine comparisons out of ten `[verified]`. The rest are the ones Encore re-engraved on the way, which is what a conversion does to a hand-placed mark.

The unit is a 256th of the measure, whatever width the measure is drawn at, which is how the notes of 6.3 and the ties of 6.7 carry their own position. An ornament does not follow its tick the way those do: fewer than a fifth of these sit within eight of the position their tick implies, because a mark is placed by hand and a note is not.

### Spanner endpoints

Hairpins and slurs store no stop element. The end is the forward measure count plus the end x within that target measure. Two caveats: with version byte `0xC2` the count is unreliable for hairpins, often stale or zero, so it should not be trusted for anything but slurs; and ottava elements store no endpoint at all, their `+14` being the visual right edge of the text box, a cosmetic constant around 12 pixels, while the `+18` slot falls outside the element and reads the next element's type byte `[observed]`.

**Slurs** need more care, because the reliable field differs by generation.

With version byte `0xC4` the end x at `+20` is meaningful: the difference between end and start x equals the pixel distance between the first and last covered notes. The x-offset is stored as a signed byte but must be read unsigned for this arithmetic, since values above 127 are stored negative.

With `0xC2` the absolute end x lives in a stale coordinate origin and must not be matched directly, so the forward measure count is the only usable endpoint, and **its offset follows the element size**:

| Slur size | Forward count | Generation |
|-----------|---------------|------------|
| 26        | `+16`         | 3.05       |
| 28        | `+18`         | 3.07       |

Reading each generation at its own offset gives clean counts and reading it at the other gives noise `[verified]`:

| Generation       | Correct offset                    | Wrong offset                             |
|------------------|-----------------------------------|------------------------------------------|
| 3.05             | `+16`, 100% inside, values 0 to 3 | `+18`, 48% inside, values 24, 11, 14, 37 |
| 3.07             | `+18`, 100% inside, values 0 to 4 | `+16`, 67% inside, values include 255    |

Once the offset follows the generation the count is reliable, including the value 0, a slur within one measure. Two effects once blamed on the field itself were artefacts of reading the wrong byte: the count looking unreliable across a whole file, and the count looking like a per-staff constant. A file whose slurs all carried 11 on one staff and 13 on the other at `+16` carries 0 at `+18` for every one of them `[verified]`.

## 6.9 Lyric

Variable size, NUL-terminated text rather than a fixed width. The anchor byte is an x-offset-like layout value.

| Generation | Anchor | Text at |
|------------|--------|---------|
| 4.20       | `+10`  | `+0x14` |
| 3.05, 3.07 | `+10`  | `+0x12` |
| 2.50       | `+5`   | `+6`    |

In format 2.50 there is no anchor-and-gap run: a single control byte follows the staff byte, then the text, and as with every element there the on-disk slot is twice the declared size.

**Separators.** A single `-` is a hyphen between syllables of one word, an empty string is a word break which resets the hyphen state, and anything else is a real syllable. A hyphen can open the measure after the syllable it follows, when a word breaks across a barline.

**Verses.** Verse N uses voice N-1 on the same staff, and every verse anchors on the voice-0 chord. Encore stores the first verse with correct per-syllable ticks, but **every later verse stores tick 0 on all its syllables** and distinguishes their positions only by the anchor byte, which matches the first verse's x-offsets syllable for syllable. The syllables are not necessarily stored in x-offset order.

## 6.10 Chord symbol

Type 7, variable size: a harmony marking above the staff.

| Offset | Size           | Field                                                      |
|--------|----------------|------------------------------------------------------------|
| `+5`   | 1              | quality index, 0 to 63; see 8.3                            |
| `+6`   | 1              | flags: bit 0 text present, bit 1 bass present, bit 2 frame |
| `+10`  | 1              | x-offset                                                   |
| `+12`  | 1              | root note                                                  |
| `+13`  | 1              | bass note, valid only when flags bit 1 is set              |
| `+14`  | to element end | the chord text, when flags bit 0 is set                    |

**The text is not a fixed slot.** It runs from `+14` to the end of the element, which is why a symbol's size grows in steps of two with the length of its name: across the corpus the sizes run 14, 16, 18 and up to 54, with 16 by far the most common `[verified]`. A name that fills its slot carries no terminator inside its own element, so the element end is the only thing that bounds it.

When the text is present it overrides the quality and root. Flags bit 2 records whether Encore draws a guitar frame above the symbol, independently of whether the name is recognisable.

**Root and bass** share an encoding: the low nibble is the note name, 0 C through 6 B, and the high nibble the accidental, 0 natural, 1 sharp, 2 flat. So `0x05` is A, `0x26` is B flat and `0x13` is F sharp.

Encore renders chord symbols at beat positions, and the stored tick carries a small offset from the notated beat, so the symbol belongs to the beat at `floor(tick / beatTicks) * beatTicks`.

## 6.11 Beam

Type 4, explicit beaming, one element per beam level.

| Size | Byte `+5` | Level                |
|------|-----------|----------------------|
| 30   | `0x01`    | first, eighth flag   |
| 46   | `0x02`    | second, sixteenth    |
| 62   | `0x03`    | third, thirty-second |

## 6.12 MIDI control change

Type 11, always 12 bytes, stored for playback only and carrying no notation.

| Offset | Size | Field                                               |
|--------|------|-----------------------------------------------------|
| `+4`   | 1    | MIDI channel or track index                         |
| `+5`   | 1    | event marker, `0xB0` for a channel-0 control change |
| `+10`  | 1    | controller: 64 sustain, 7 volume, 1 modulation      |
| `+11`  | 1    | value: 127 maximum or on, 0 off                     |

---

# 7. Rules that cut across elements

## 7.1 Where durations come from

**Encore does not store how long a note lasts.** It stores where the note starts and what it looks like, and the length is the distance to whatever comes next.

An element carries a tick, its position within the measure, and a face value, which is the notehead and the written duration. Neither is the sounding length. The face value is what the note is drawn as, and it is wrong whenever the note is dotted, tied into, or part of a tuplet. The playback duration some generations store at `+16` is a recording artefact: it diverges from the notated value for live-recorded music, and the last note of a tuplet ending at a barline often has one far shorter than its face value, because Encore truncates playback at the barline.

The sounding duration of an element is the gap from its own tick to the tick of the next element **in the same voice on the same staff**, bounded by:

- **the end of the measure**, for the last element of a voice, using the measure's total ticks;
- **a mid-measure clef or key change**, which stops the gap where it sits, since a note cannot sound across one in Encore's model;
- **same-tick chord members**, which are skipped: every note of a chord shares one start, so the gap is measured to the next *different* tick;
- **near-simultaneous members of a strummed chord**, which are also skipped; see 7.7.

Two consequences follow, and both are ordinary rather than exceptional.

**Silences are usually implicit.** Encore does not always write a rest element. A gap between two consecutive events of one voice, where the next tick exceeds the previous tick plus the previous note's face value, is a silence the user wrote as a rest, and a reader must fill it.

**The written duration is then recovered from the sounding one**, by comparing it against the face value: the dots in 7.3, the tuplet ratio in 7.4. This is the reverse of most formats, where the notation is stored and the playback derived.

## 7.2 Ticks and face values

There are 240 ticks to a quarter note. The whole note is always 960 ticks whatever the time signature, and is computed as `totalTicks * denominator / numerator` from the measure header.

**Do not compute it from the beat ticks.** In a compound meter the beat is the compound one, 360 for the dotted quarter of 6/8, which would give 2880 instead of 960. Some builds also store 240 for 2/2, where 480 is correct.

| Face value | Ticks | Duration  |
|------------|-------|-----------|
| 1          | 960   | whole     |
| 2          | 480   | half      |
| 3          | 240   | quarter   |
| 4          | 120   | eighth    |
| 5          | 60    | sixteenth |
| 6          | 30    | 32nd      |
| 7          | 15    | 64th      |
| 8          | 7     | 128th     |

The 128th is stored as 7 because 960 divided by 128 is 7.5, truncated.

In compound and simple meters where one beat is an eighth, 6/8, 8/8 and 12/8, Encore stores the face value as a number of beats rather than an absolute note value, so the written duration is `faceTicks * actual / normal` when that product is a standard tick count.

## 7.3 Dots

The dot count is stated twice, and the two statements are almost always the same one. A note carries it in the low two bits of the layout byte at `+14`, described in 6.3, and it also falls out of the sounding duration against the face value: one dot when the duration is 3/2 of the face ticks, two when it is 7/4, three when it is 15/8.

**The stated count and the durations agree on 99.89% of the notes whose duration is an exact plain or dotted multiple** `[verified]`. Where they part company the stated count is the one Encore draws, since it is the field the program reads, and the durations are a reconstruction.

The disagreements are worth knowing, and two of Encore's editing habits account for most of them.

**A dot added later does not move what follows.** Put a dot on a note that already has neighbours and Encore lengthens the drawn figure without shifting their positions, so the bar's face values come up short by exactly that dot while every note-on stays where it was. Well under one note in ten thousand is in that state, and always in the middle of a voice `[verified]`.

**The last note of a bar may be longer than the space left.** Encore accepts any figure there and clips its playback to whatever remains, so a bar can display a dotted half after a quarter rest in 3/4 and still lay out normally; fill the bar first and the program refuses the note instead `[verified]`. A stated dot on the last note of a voice whose face value already fills the space is this, and not a dot the durations lost.

The habit is common rather than exceptional: in about one voice group in sixteen, spread over a quarter of the corpus's files, the last note is drawn longer than its space by at least a 32nd `[verified]`. A few percent of those owe the excess to a stated dot and the rest to a face value that never fitted, a whole note in the last two beats being the usual shape. A reader that wants the page to match Encore's has to keep the figure and give the bar the room; one that wants the playback to match has to fit the note to the space. The two cannot both be had, since a written duration in most notation models is the time it occupies.

The rest are ordinary. The largest group of them is a duration showing a dot the count does not state, which is a gap standing where a rest was not written, and the next is a note triple-dotted by duration, which the two-bit field cannot express at all.

A spacing is never evidence on its own. A sixteenth whose note-on sits a plain eighth after the note before it is what an undotted eighth followed by a sixteenth looks like, and it occurs at the same rate in every generation, so a reader that infers a dot from that shape is inventing one.

## 7.4 Tuplets

A tuplet is stated one of two ways.

**Explicitly**, in a byte packing the two counts, the actual in the high nibble and the normal in the low, so `0x32` is 3:2 and `0x54` is 5:4.

**Implicitly**, by duration alone: a run whose sounding durations sit at a constant fraction of their face values is a tuplet of that ratio, and its tuplet byte reads zero. It is uncommon, and equally so in every generation: runs of three such notes occur in between one and three files in a thousand, whether the file is format 3.05 or 4.20 `[verified]`. A single note at a tuplet ratio proves nothing, since a note-on can drift; only a full run does.

So it is the ordinary case before 4.20 and vanishes afterwards, which reads as the tuplet byte becoming reliable rather than as two different notations. A reader that trusts the byte alone loses those groups entirely.

Recognised ratios:

| Ratio                   | Name            |
|-------------------------|-----------------|
| 2:1                     | whole duplet    |
| 2:3                     | compound duplet |
| 2:4                     | 2 in 4          |
| 3:2                     | triplet         |
| 4:1, 4:2, 4:3           | quadruplet      |
| 5:2, 5:3, 5:4, 5:6, 5:8 | quintuplet      |
| 6:4, 6:7, 6:8           | sextuplet       |
| 7:4, 7:6, 7:8           | septuplet       |
| 8:4, 8:6                | octuplet        |
| 9:4, 9:6, 9:8           | nontuplet       |
| 10:6, 10:8              | decuplet        |

A ratio whose bracket span, the normal count times the base length, is not a standard note value cannot be notated as a bracket.

**Nothing in the bytes says where one bracket ends and the next begins**: the ratio is per note, not per group. A run of mixed face values under one ratio is therefore ambiguous. A quarter followed by four eighths at 3:2 reads either as one flat bracket over a half note, or as a quarter plus a bracket of three eighths nested in the second slot, and both are consistent with the face values. The tick positions decide: a nested group replaces exactly one slot of the outer bracket, so its notes must span the same distance as that slot `[verified]`.

## 7.5 Keys

The key index, used by both the key-change element and the system block's staff entry, is a position on the circle of fifths.

| Index   | Fifths   | Key                   |
|---------|----------|-----------------------|
| 0       | 0        | C                     |
| 1 to 7  | -1 to -7 | F, B flat, ... C flat |
| 8 to 14 | +1 to +7 | G, D, ... C sharp     |

Index 0 is a legitimate value: naturals cancelling prior accidentals.

**Encore stores only the sounding pitch**, plus the instrument's Key transposition from 5.1. It never stores an enharmonic spelling, so the bytes fix the audible pitch and not the written accidental.

## 7.6 Articulation bytes

Each articulation byte holds one or two glyphs. The range `0x22` to `0x2D` is laid out in consecutive pairs, below then above, one pair per glyph `[verified]`.

| Value            | Meaning                                      |
|------------------|----------------------------------------------|
| `0x01`           | flat mark, not an articulation               |
| `0x02`           | sharp or natural mark                        |
| `0x03`           | 3-stroke tremolo, bare                       |
| `0x04`           | trill, plain                                 |
| `0x05`           | trill to a minor second                      |
| `0x06`           | trill to an augmented second                 |
| `0x07`           | trill to a major second                      |
| `0x08`           | turn                                         |
| `0x09`           | wave mark                                    |
| `0x0A`           | inverted mordent, short                      |
| `0x0B`           | mordent, simple lower                        |
| `0x0C`           | inverted mordent, long                       |
| `0x0D` to `0x11` | fingering 1 to 5                             |
| `0x12`           | accent                                       |
| `0x13`           | marcato                                      |
| `0x14`           | staccato with heavy accent, below            |
| `0x15`           | marcato with staccato                        |
| `0x16`           | accent with staccatissimo                    |
| `0x17`           | accent with staccato                         |
| `0x18`           | up bow                                       |
| `0x19`           | down bow                                     |
| `0x1A`           | marcato, variant                             |
| `0x1B`           | stopped horn                                 |
| `0x1C`           | tenuto                                       |
| `0x1D`           | staccato                                     |
| `0x1E`, `0x1F`   | harmonic                                     |
| `0x20`, `0x21`   | fermata; on a tuplet note, bracket placement |
| `0x22`, `0x23`   | tenuto with accent                           |
| `0x24`, `0x25`   | tenuto with staccato, portato                |
| `0x26`, `0x27`   | marcato with tenuto                          |
| `0x28`, `0x29`   | staccatissimo                                |
| `0x2A`, `0x2B`   | heavy accent with staccatissimo              |
| `0x2C`, `0x2D`   | tenuto with staccatissimo                    |
| `0x2E`           | inverted turn                                |
| `0x2F`           | mordent, double or long lower                |
| `0x30`           | half-stopped horn                            |
| `0x39` to `0x40` | scale string number 1 to 8                   |
| `0x41`           | 1-stroke tremolo, eighth                     |
| `0x42`           | 2-stroke tremolo, sixteenth                  |
| `0x43`           | 3-stroke tremolo, 32nd                       |
| `0x44`, `0x45`   | thumb position                               |
| `0x46`           | open string                                  |
| `0x47`           | drumstick technique                          |
| `0x48`           | brush                                        |
| `0x49`           | soft mallet                                  |
| `0x4A`           | hard mallet                                  |

Heavy accent is the wedge Encore draws as a marcato. Values `0x44` and above are technical markings, not tremolos. The string numbers follow `0x38 + N`. When at least one string byte appears in a measure, every note in that measure with options bit 0 set displays its scale-degree position as a circled string number.

Each note of a chord carries its own articulation bytes, so a glyph shared by several members is simply repeated on each; the chord shows at most one copy of each distinct glyph.

## 7.7 The chord column

The note x-offset at `+10` is the notated horizontal column. It exists from format 3.05 on; format 2.50 does not store it.

- Every member of one chord shares the same non-zero column, and successive chords occupy distinct ones. A zero means no stored column.
- The layout runs strictly left to right, so the column increases with tick, and it is aligned across the staves of a system: notes on the same beat share a column across staves.
- Adjacent columns lie at least a small distance apart, around eight pixels in observed files, while a chord's members share one give or take a notehead `[observed]`.
- **The notes of one chord are not always stored at the same tick.** A chord recorded live, or given a strum, keeps its members at staggered playback ticks, with drift up to a sizeable fraction of the note value, while still sharing one column `[observed]`. This is why the gap rule in 7.1 skips near-simultaneous members.
- A note whose column matches an earlier beat but whose tick is later is a stale-tick artefact, left when the note was moved in Encore and kept its old playback tick. Encore draws it at the column's beat.

## 7.8 Text encoding

Text-bearing fields are Latin-1 or UTF-16 LE, chosen per field by a probe on the first two bytes: byte 0 printable ASCII, in the range `0x20` to `0x7E`, followed by a zero byte means UTF-16 LE, and anything else, especially an accented Latin-1 byte in the second position, means Latin-1.

| Field                    | Probe at                                  |
|--------------------------|-------------------------------------------|
| instrument name          | the first two bytes of the name           |
| positional name recovery | the first two bytes at the formula offset |
| lyric text               | the first two bytes of the text           |
| text-block entry         | bytes 14 and 15 of the entry              |
| chord symbol text        | the first two bytes of the text           |
| title block              | by block size; see 5.6                    |

Forcing one encoding turns legacy Latin-1 into byte-swapped CJK gibberish, or drops half of every UTF-16 code unit, so the probe is always applied.

Encore 5.0.2 writes UTF-16 instrument names regardless of the size field, and may omit an instrument block header while the name is still present at the positional offset `[observed]`.

---

# 8. Reference

## 8.1 Per-generation differences at a glance

Everything that moves between generations, in one table.

| Aspect                 | 2.50              | 3.05          | 3.07          | 4.20          |
|------------------------|-------------------|---------------|---------------|---------------|
| header ends at         | `0xA6`            | `0xC2`        | `0xC2`        | `0xC2`        |
| size selector at       | `0x8D`            | `0x52`        | `0x52`        | `0x52`        |
| measure header         | 26 bytes          | 54 bytes      | 54 bytes      | 54 bytes      |
| element slot stride    | size times two    | size          | size          | size          |
| note size              | 10 or 11          | 22            | 24            | 28            |
| note pitch at          | `+11`             | `+13`         | `+15`         | `+15`         |
| note tuplet at         | `+7`              | `+11`         | `+13`         | `+13`         |
| tie size               | 7                 | 16            | 18            | 18            |
| tie flags              | start, dir        | dir, start    | dir, start    | dir, start    |
| tie arc pair at        | `+8`, single      | `+8`, `+10`   | `+10`, `+12`  | `+10`, `+12`  |
| lyric text at          | `+6`              | `+0x12`       | `+0x12`       | `+0x14`       |
| ornament x             | `+8`              | `+8`          | `+10`         | `+10`         |
| ornament y             | `+9`, signed byte | `+10`, 16-bit | `+12`, 16-bit | `+12`, 16-bit |
| ornament text index at | `+28`             | `+30`         | `+32`         | `+32`         |
| spanner count at       | `+14`             | `+16`         | `+18`         | `+18`         |
| chord column           | not stored        | stored        | stored        | stored        |
| key signature at       | staff entry `+14` | entry `+15`   | entry `+15`   | entry `+15`   |
| text run header        | absent            | present       | present       | present       |
| margins unit           | screen pixels     | pixels        | pixels        | points        |

## 8.2 Ornament subtypes

| Value            | Meaning                                            |
|------------------|----------------------------------------------------|
| `0x10`           | ottava 8va, line above the staff                   |
| `0x12`           | ottava 8vb, line below                             |
| `0x1C`           | user-drawn graphic line, no musical meaning        |
| `0x1D`           | hairpin start                                      |
| `0x1E`           | staff text, payload in the text block              |
| `0x21`           | slur start                                         |
| `0x22`           | arpeggio                                           |
| `0x28`           | guitar bend, curved arrow up                       |
| `0x29`           | guitar bend, curved arrow                          |
| `0x2A`           | guitar prebend                                     |
| `0x2B`           | guitar prebend and release                         |
| `0x30`           | guitar V-shape bend                                |
| `0x32`           | tempo mark                                         |
| `0x35`           | trill-span end; see note 1                         |
| `0x36`           | trill-span start, tr with a wavy line              |
| `0x37`           | secondary trill mark within a span                 |
| `0x41`           | slur stop, reserved and not emitted in practice    |
| `0x4D`           | hairpin stop, reserved and not emitted in practice |
| `0x80` to `0x87` | dynamics ppp, pp, p, mp, mf, f, ff, fff            |
| `0x88`           | dynamic sfz                                        |
| `0x89`           | dynamic sffz                                       |
| `0x8A`           | dynamic fp                                         |
| `0xA2`           | segno                                              |
| `0xA3`           | repeat-last-bar glyph                              |
| `0xA5`           | To Coda navigation point                           |
| `0xA6`           | coda glyph                                         |
| `0xA7`           | caesura, after the preceding note                  |
| `0xA8`           | comma breath mark, after the preceding note        |
| `0xAA`           | dynamic fz                                         |
| `0xAB`           | dynamic sf                                         |
| `0xAF`           | single-chord triple tremolo, always voice 0        |
| `0xB0`           | standalone tr mark; see note 2                     |
| `0xB6`           | standalone short-trill mark, never a span          |
| `0xB8`           | standalone trill zigzag; see note 3                |
| `0xB9` to `0xBD` | standalone fingering digit 1 to 5; see note 4      |
| `0xBE`           | accent, from format 3.07 on                        |
| `0xBF`           | marcato, vertex up                                 |
| `0xC0`           | marcato with staccato, below                       |
| `0xC4`           | up bow from 3.07 on; accent before it              |
| `0xC5`           | down bow                                           |
| `0xC6`           | marcato below, vertex down                         |
| `0xC8`           | tenuto dash above                                  |
| `0xC9`           | per-chord staccato dot                             |
| `0xCC`           | standalone fermata above                           |
| `0xCD`           | standalone fermata below                           |
| `0xE6`, `0xE7`   | 1-slash tremolo, eighth speed                      |
| `0xE9`, `0xEA`   | 4-slash tremolo, 64th speed                        |
| `0xEE`           | 2-slash tremolo, sixteenth speed                   |
| `0xEF`           | alternate triple tremolo, 32nd speed               |

Subtypes confirmed by opening the file in Encore 5 are `[verified]`; the rest are `[observed]`.

1. **Trill-span end.** Closing a same-measure trill-span start it is the invisible endpoint. A lone one, with no start in its own measure, is a standalone trill drawing a tr and a wavy line on its own note, for instance a terminal trill. It never pairs with a start in another measure, since those spans use the forward measure count instead.
2. **Standalone tr.** Size 16, a plain trill and never a span. The text is drawn to the left of its note, so its stored x sits left of the notehead. When a note shares the mark's tick, that note is the target and no snapping is needed; only when no note sits at the mark's tick does it snap to the note it visually rests on.
3. **Trill zigzag.** Despite once being called a double mordent, it is not one: in the corpus it appears only as a trill mark, once, immediately after a trill-span start where Encore draws a wavy trill. Real mordents use the note articulation bytes.
4. **Standalone fingerings.** Size 16, always stored on voice 0, but the digit belongs to the note it visually sits over, which may be in another voice of the same staff. Only a genuine overflow, more digits than notes at the tick, belongs to the second staff or the next measure's downbeat.

A trill-span start opens a span when a span end or a non-zero forward measure count is present, and is otherwise a plain trill glyph. The accent, up bow and down bow marks carry a voice byte that is always 0 regardless of the annotated note's voice.

### The older articulation codes

The table above is the vocabulary from format 3.07 on. **A file older than that states five of those articulations six codes higher**, and no later generation uses the higher values.

| 3.05 and older | 3.07 and later | Meaning       | Evidence                               |
|----------------|----------------|---------------|----------------------------------------|
| `0xC4`         | `0xBE`         | accent        | 97% of cases are 3.05 `[verified]`     |
| `0xCE`         | `0xC8`         | tenuto        | conversion pair, 1 for 1 `[verified]`  |
| `0xCF`         | `0xC9`         | staccato      | conversion pair, 5 for 5 `[verified]`  |
| `0xD2`         | `0xCC`         | fermata above | two conversion pairs `[verified]`      |
| `0xD3`         | `0xCD`         | fermata below | corpus counts only `[observed]`        |

The accent is the one whose two spellings collide, because `0xC4` is a genuine up bow from 3.07 on. So the mapping must be scoped by format version and not by the version byte, which reads `0xC2` for both generations.

The rest of the vocabulary did not move. In a conversion pair whose two halves hold the same ornaments, the accent at its later code, the breath mark, the tempo mark, the staff text and the slur all keep their codes, and only the one articulation changes. Corpus-wide the same picture holds: across the format 3.05 files there is not one staccato at `0xC9`, the most common articulation in every other generation, while `0xCF` is the most common code in that range.

Codes `0xC0`, `0xC1`, `0xC2` and `0xCA` also occur in 3.05 and are absent or rare later. They are probably the same block shifted, which would make them the fingerings and the up bow, but no conversion pair covers them and they are left as stated.

## 8.3 Chord qualities

Encore's own palette, in palette order `[verified]`. The augmented fifth is written "+5" at indices 15 and 19, and index 46 is "sus2,sus4".

| Index | Suffix    | Index | Suffix     | Index | Suffix    | Index | Suffix    |
|-------|-----------|-------|------------|-------|-----------|-------|-----------|
| 0     | major     | 16    | maj7(#11)  | 32    | 9         | 48    | 7sus4     |
| 1     | m         | 17    | maj9       | 33    | 9(b5)     | 49    | 9sus4     |
| 2     | +         | 18    | maj9(b5)   | 34    | 9(#11)    | 50    | 13sus4    |
| 3     | dim       | 19    | maj9(+5)   | 35    | 11        | 51    | m(add2)   |
| 4     | dim7      | 20    | maj9(#11)  | 36    | 13        | 52    | m(add9)   |
| 5     | 5         | 21    | maj13      | 37    | 13(b5)    | 53    | m6        |
| 6     | 6         | 22    | maj13(b5)  | 38    | 13(b9)    | 54    | m6/9      |
| 7     | 6/9       | 23    | maj13(#11) | 39    | 13(#9)    | 55    | m7        |
| 8     | (add2)    | 24    | 7          | 40    | 13(#11)   | 56    | m(maj7)   |
| 9     | (add9)    | 25    | 7(b5)      | 41    | +7        | 57    | m7(b5)    |
| 10    | (omit3)   | 26    | 7(b9)      | 42    | +7(b9)    | 58    | m7(add4)  |
| 11    | (omit5)   | 27    | 7(#9)      | 43    | +7(#9)    | 59    | m7(add11) |
| 12    | maj7      | 28    | 7(#11)     | 44    | +9        | 60    | m9        |
| 13    | maj7(b5)  | 29    | 7(b5,b9)   | 45    | sus2      | 61    | m9(maj7)  |
| 14    | maj7(6/9) | 30    | 7(b5,#9)   | 46    | sus2,sus4 | 62    | m11       |
| 15    | maj7(+5)  | 31    | 7(b9,#9)   | 47    | sus4      | 63    | m13       |

## 8.4 Oddities worth knowing

- **Duplicate notes.** Some files encode the same pitch twice in one chord: either the second copy has grace1 bit `0x40` set, a chord-extension marker, or, in the `0xC2` generations, both copies have grace1 zero. Either way it is a redundant notehead.
- **Consecutive identical rests, format 2.50.** Two rest elements with the same tick, staff, voice and face value in a row represent one rest.
- **Tempo words.** Italian words such as Allegro are stored as staff-text ornaments, not as tempo elements; only numeric marks use the tempo subtype. Each measure header also carries a BPM that persists forward, and the tempo ornament's stored tick rarely matches the measure the tempo applies to, so the header BPM is the reliable source for position.
- **Largest legitimate block.** A measure block rarely exceeds 2 KiB. A significantly larger one indicates a corrupt file or an undocumented variant.

---

# 9. What is not established

Gathered here rather than scattered, because knowing the edge of the map matters as much as the map.

- **Which release wrote format 3.07.** Its files run from 1999 and its geometry sits between 3.05 and 4.20, but no distribution in hand produces it, and the format 4.20 example scores are older.
- **Why a file carries revision 1 rather than 0.** Encore 4.5 writes 0 and Encore 5.0 writes 4, both measured, but the two values 0 and 1 appear side by side in every generation and no release in hand writes 1, so what selects it is unresolved. See 1.6.
- **`SCOX`.** Reported in earlier descriptions and never seen.
- **What a `ZBOP` file holds.** Its magic decrypts to `SCOS`, but no file has been seen and the `SCOS` layout is unknown.
- **Ornament codes `0xC0`, `0xC1`, `0xC2` and `0xCA` in format 3.05.** Probably the shifted fingerings and up bow, but no conversion pair covers them.
- **Bit 3 of the note's tie nibble.** Value `8` occurs on a fraction of a percent of the notes in format 4.20 and never alongside a `TIE` element; its neighbours share its pitch no more often than chance, so it is not a tie flag.
