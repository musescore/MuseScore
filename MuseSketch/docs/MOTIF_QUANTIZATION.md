1. **Concrete gesture design** for the motif contour canvas (how users actually draw, jump, rest, and define length)
2. **A motif quantization algorithm** (from raw touch points → discrete notes/rests in a grid, ready for MuseScore)

---

## 1. Motif Contour Canvas – Gesture Design

### 1.1 Canvas basics

* **Horizontal axis = time**

  * Left edge = motif start (beat 0)
  * Right edge = motif end (e.g. 1–4 measures, chosen in advance: `motifLengthBars`)
* **Vertical axis = pitch**

  * Center line = chosen “pivot” pitch (e.g. scale degree 3 in middle register)
  * Up = higher; down = lower
* Grid is **visible but light**:

  * Vertical grid lines = beats / subdivisions
  * Horizontal guide lines = reference scale degrees (e.g. 1, 3, 5 of the key)

User sets **motif length** *before* drawing (1, 2, 4 measures). You can expose that as a segmented control:

> [ 1 bar ] [ 2 bars ] [ 4 bars ]

---

### 1.2 Core gesture vocabulary

#### A. Draw a melodic line (stepwise or leaping)

* **Gesture:** Touch down, drag right with up/down movement, lift finger.
* Interpretation:

  * The stroke’s **x path** defines when notes happen.
  * The stroke’s **y path** defines pitch contour (intervals, direction).

**Jumps in melody** are natural: the user just moves their finger sharply up/down.
No special gesture needed — vertical distance + quantization handles it.

#### B. Create rests

You have two simple options that work well together:

1. **Gaps in x-space**:

   * When the stroke is absent over certain beats (no drawn line crossing that time), the quantizer inserts rests.
2. **Explicit “lift” gesture**:

   * User draws a bit, **lifts finger**, moves horizontally, then draws again.
   * The gap between the end of stroke A and start of stroke B becomes rest time.

Optionally, add a “rest eraser”:

* Tap-hold on the canvas at an empty spot → toggles a rest over that grid cell.

#### C. Change motif length (bars)

* User picks 1, 2 or 4 bars **before drawing**.

  * Canvas width scales accordingly.
* If they later change length:

  * **Shortening**:

    * Option A: warn and crop notes beyond new end.
    * Option B: proportionally compress motif (more advanced).
  * **Extending**:

    * Empty time → implicit rests; user can fill with another stroke.

For v1, simplest is **crop on shorten** and **pad with rests on extend**.

---

### 1.3 Extra gestures (optional but nice)

* **Double-tap on a beat** → place a simple “anchor” note at default pitch.
* **Two-finger tap** → undo last stroke.
* **Long-press on a note (in preview)** → open pitch/rhythm “nudge” wheel.
* **Pinch zoom** horizontally to zoom into rhythm; vertically to zoom register.

You can still keep this all fairly minimal on phone screens.

---

## 2. Motif Quantization Algorithm

Goal:
Convert raw touch strokes (continuous lines) into a list of **note events**:

```ts
type MotifEvent = {
  startBeat: number;      // e.g. 0.0, 0.25, 0.5
  durationBeats: number;  // e.g. 0.25, 0.5, 1.0
  pitchMidi: number | null;  // null = rest
};
```

We’ll assume:

* You know the **motif length** in bars: `L_bars`.
* You know the **time signature**: e.g. `4/4`.
* You choose a **smallest subdivision** (e.g. 16th notes) for the grid.

Let:

* `beatsPerBar = timeSigNumerator`
* `totalBeats = L_bars * beatsPerBar`
* `subdivision = 1/4` (for 16ths in 4/4; 1 beat / 4)
* `gridStep = subdivision`

### 2.1 Data you collect from the canvas

Each stroke is a sequence of points:

```ts
type Point = { x: number; y: number; t: number };

type Stroke = {
  points: Point[];
};
```

Where:

* `x` is in canvas pixels → mapped to [0, totalBeats]
* `y` is in canvas pixels → normalized to [0,1] or [-1,1] relative to middle line.

You may have **multiple strokes** per motif (user can draw in passes).

---

### 2.2 Algorithm overview

**Phase 1: Normalize coordinates**
**Phase 2: Build a beat grid**
**Phase 3: Assign pitches per grid cell**
**Phase 4: Convert grid to notes/rests with durations**

---

### 2.3 Phase 1 – Normalize

1. Compute canvas width `W` and height `H`.

2. Map point.x → time in beats:

   ```ts
   beat = (point.x / W) * totalBeats;
   ```

3. Map point.y → normalized pitch position:

   You choose a convention, e.g.

   ```ts
   yNorm = 1.0 - (point.y / H);  // top=1, bottom=0
   ```

4. Store for each stroke: `normalizedPoints = [{ beat, yNorm }]`.

---

### 2.4 Phase 2 – Build grid and segment strokes

Create an array of **grid slots**:

```ts
let numSlots = Math.round(totalBeats / gridStep);
let slots = Array(numSlots).fill(null); 
// each slot will hold pitch info or null
```

Each slot `i` represents the time interval:

* `[i * gridStep, (i+1) * gridStep)`

Now, for each stroke:

1. For each grid slot:

   * Collect all stroke points whose `beat` falls inside that slot.
   * If there are no points from any stroke → it’s a **rest candidate**.
   * If there are points → we’ll estimate a pitch for that slot.

Because strokes are continuous, you may want to **interpolate** between points so every slot has something if the line crosses it. For MVP, you can:

* For each slot, take the stroke **point nearest in time** to slot center.

---

### 2.5 Phase 3 – Pitch quantization

For a grid slot with one or more `yNorm` values (from maybe multiple strokes):

1. Compute an aggregate `y`. E.g.:

   ```ts
   ySlot = median(yNormsInSlot);
   ```

2. Map this `ySlot` to a **continuous pitch** in semitones relative to a reference:

   Define a reference pitch:

   ```ts
   let refMidi = 60; // middle C, or user’s chosen center
   ```

   And a range, say top to bottom spans ±`R` scale steps (e.g. 12 steps = a 12-degree range).

   ```ts
   // ySlot in [0,1], map to [-R/2, R/2]
   let relScalePosition = (ySlot - 0.5) * R;
   ```

3. Convert `relScalePosition` to **scale degrees**:

   * You know the key and mode (e.g. C major, D dorian).
   * Build a scale vector in semitone offsets: e.g. C major = [0, 2, 4, 5, 7, 9, 11].
   * Multiply `relScalePosition` by some factor to choose nearest degree index:

     ```ts
     // For example, 1.0 unit in relScalePosition = 1 scale degree
     let degreeIndexFloat = relScalePosition + centerDegreeIndex; // center maybe 3rd

     let degreeIndex = Math.round(degreeIndexFloat);
     degreeIndex = clamp(degreeIndex, 0, scaleLength - 1);
     ```

4. From degree index, get semitone offset:

   ```ts
   let octaveOffset = Math.floor(degreeIndex / scaleLength);
   let degreeInOctave = degreeIndex % scaleLength;

   let semitoneOffset = octaveOffset * 12 + scale[degreeInOctave];
   let midiPitch = refMidi + semitoneOffset;
   ```

5. Store `midiPitch` in `slots[i]`.

**Handling leaps:**

Leaps are automatic — if `degreeIndex` jumps from e.g. 2 → 9, that’s a big leap in the resulting MIDI.

You can optionally smooth:

* If the jump between consecutive slots exceeds `maxLeap` (e.g. 12 semitones), you:

  * Either accept it,
  * Or offer a “smoothing” mode that compresses large jumps.

For v1, **just accept leaps**; composers want control.

---

### 2.6 Phase 4 – From grid slots to note events

Now each `slots[i]` is either:

* `null` → rest candidate
* `midiPitch` → note at that slot

We’ll compress consecutive slots with same pitch into a **single event**.

Algorithm:

```ts
let events: MotifEvent[] = [];
let currentPitch = null;     // midi or null
let currentStartBeat = 0;

for (let i = 0; i < numSlots; i++) {
  let slotPitch = slots[i]; // midi or null
  let slotStartBeat = i * gridStep;
  let slotEndBeat = slotStartBeat + gridStep;

  if (i === 0) {
    currentPitch = slotPitch;
    currentStartBeat = slotStartBeat;
    continue;
  }

  if (slotPitch === currentPitch) {
    // continue current note/rest
    continue;
  } else {
    // close old event, start new
    let duration = slotStartBeat - currentStartBeat;
    events.push({
      startBeat: currentStartBeat,
      durationBeats: duration,
      pitchMidi: currentPitch, // null means rest
    });

    currentPitch = slotPitch;
    currentStartBeat = slotStartBeat;
  }
}

// close last event
let lastDuration = totalBeats - currentStartBeat;
events.push({
  startBeat: currentStartBeat,
  durationBeats: lastDuration,
  pitchMidi: currentPitch,
});
```

Post-processing:

* Remove leading/trailing full-rest events surrounded by silence (if desired).
* Merge very short blips under a threshold into neighbors (e.g. < 1/32 note, treat as noise).
* Optionally enforce that total `durationBeats` sums exactly to `totalBeats`.

---

### 2.7 Mapping to notation values

To send this to MuseScore:

1. Convert `durationBeats` to note values:

   * Use time signature & subdivision to decide:

     * 0.25 beat → 16th
     * 0.5 beat → 8th
     * 1 beat → quarter
     * 1.5 → dotted quarter, etc.
2. Insert notes/rests at `startBeat` in a MuseScore measure structure.
3. Let MuseScore handle beam grouping & engraving.

You can start with simple fraction-to-note mappings and expand later.

---

### 2.8 Handling motif length & user feedback

* Motif length `L_bars` sets `totalBeats`.
* The canvas visually shows barlines; quantization strictly clamps events inside `[0, totalBeats]`.
* If user draws beyond canvas width, you ignore or clamp those points.
* Show them a quick preview (“ghost notes”) before committing to full notation.

