# 🎨 **MuseSketch – Motif Contour Canvas Interaction Model**

*(Gesture & Interaction Specification)*

The **Motif Contour Canvas** is a *touch-native ideation surface*. It is not a staff; it is a fluid, musical sketchpad that translates gestures into quantized melodic/rhythmic structures.

This document defines **all user actions**, the **gesture grammar**, and the **rules** behind interpreting them.

---

# 1. 🌄 **Canvas Overview**

A simple horizontal timeline representing 1–4 measures.

* Vertical axis = **pitch contour**
* Horizontal axis = **time**
* Seasoned by:

  * A **grid** (beats)
  * A **scale** (major/minor/modals)
  * A **tonal center**

The composer does not deal with noteheads, stems, or spacing.
They draw **shapes**, which MuseSketch converts into quantized motive data.

---

# 2. ✋ **Core Gesture Vocabulary**

## 2.1 **Draw a Continuous Line → Melodic Contour**

**Gesture:**

* Touch → drag → lift
* A single fluid path

**Interpretation:**

* System samples the y-axis (pitch) and x-axis (time)
* Snap pitch to scale degrees (or chromatic if chosen)
* Snap timing to subdivision grid

**Use case:**

* Smooth scalar lines
* Curved motivic gestures
* Melodic arcs, waves, swoops

---

# 3. 🎯 **Pitch Control Gestures**

## 3.1 **Stepwise Motion**

* Gentle slope (small vertical deviation) = stepwise intervals (2nds)

## 3.2 **Leaps (3rds, 4ths, 5ths, 6ths, octaves)**

**Gesture:**

* Sharp vertical jump
* Instant drop or rise between two points (like a discontinuity)

**How the system recognizes a leap:**

* If vertical delta exceeds threshold (e.g., > 20–25 px), interpret as:

  * 3rd (short)
  * 4th/5th (medium)
  * 6th/8ve (large)

**User feedback:**

* A temporary tooltip showing: “Leap: Perfect 5th”
* Snapping preferences ensure tonal stability

---

# 4. 🕳️ **Rest Creation Gestures**

Rests represent “motif silence,” crucial for shape-driven composition.

## 4.1 **Lift Gesture → Rest**

**Gesture:**

* Draw a line
* Lift finger
* Wait 0.2s
* Re-touch canvas later horizontally

**Interpretation:**

* Time between lift and next touch = rest duration
* Quantized to nearest beat/subdivision

## 4.2 **Horizontal Skip Gesture**

**Gesture:**

* Draw stroke
* Lift
* Swipe right (no touch) above timeline
* Re-touch

**Interpretation:**

* Explicit skip of time = rest
* Useful on phone screens

## 4.3 **Two-Finger Tap on Timeline**

* Inserts a rest subdivision at cursor
* Duration selected via rhythm strip

---

# 5. ⏱️ **Rhythm & Duration Gestures**

## 5.1 **Horizontal Drag Speed**

* Fast drag → shorter rhythmic events (16ths/8ths)
* Slow drag → longer (quarters/halves)
* System still quantizes to nearest valid grid

## 5.2 **Hold at a point → Sustained Note**

**Gesture:**

* Draw contour
* Pause finger at one position for >0.15s

**Interpretation:**

* Holds that pitch and extends note length

## 5.3 **Pinch Horizontal → Change Subdivision Density**

* Pinch inward = denser grid (8ths → 16ths)
* Pinch outward = sparser (quarters → halves)

## 5.4 **Long Press on Grid → Insert Dotted Duration**

* Context menu:

  * “Dot note”
  * “Tie next”
  * “Make syncopated pattern”

---

# 6. 🎚️ **Motif Length Selection (1–4 Measures)**

Motif length is defined **before** or **during** contour drawing.

## 6.1 **Top Toolbar Selector**

* “Length: 1 | 2 | 3 | 4 bars”
* Default: 1 bar

## 6.2 **Canvas Auto-Extend**

**Gesture:**

* Draw past right boundary by ~25 px

**Interpretation:**

* Extend canvas to next bar (max 4)

**Visual Feedback:**

* Bar divider animates into view

## 6.3 **Pinch Out on Right Edge**

* Expands motif to additional measure
* Pinch In = contract by one measure
* Never under 1 measure

---

# 7. ⚖️ **Advanced Pitch Actions**

## 7.1 **Forced Chromatic Steps**

**Gesture:**

* Finger zig-zag (quick micro-jitter)

**Interpretation:**

* Chromatic line, bypassing scale restrictions

## 7.2 **Octave Toggle**

**Gesture:**

* Vertical double-tap while drawing

**Interpretation:**

* Immediately shift next pitches up or down an octave

## 7.3 **Scale Degree Lock**

**Gesture:**

* Two-finger vertical drag = lock to scale degree (like “snap-to-note”)

---

# 8. 🎶 **Transformational Gestures (Optional for v1)**

## 8.1 **Invert Contour**

* Two-finger flip upward or downward
* Immediately preview inversion

## 8.2 **Retrograde**

* Swipe left on the canvas with two fingers
* Reverses motif timeline

## 8.3 **Augment/Diminish Rhythm**

* Vertical pinch (up = augmentation, down = diminution)

*(May be v1.1 or later, but fits beautifully.)*

---

# 9. 📱 **Phone vs. Tablet UX Rules**

## Phone

* Bigger stroke smoothing
* Larger leap threshold
* Defaults to 1–2 measure motifs
* Optional “Pad Input Mode” with scale degrees + rhythm grid

## Tablet

* Exact contour fidelity
* Multi-measure view
* Pencil support (future)

---

# 10. 🎼 **Output of Contour Canvas → Motif Data Structure**

Each gesture session results in:

```
Motif {
   id: UUID,
   measures: 1-4,
   pitches: [p0, p1, ... scaled + quantized],
   rhythm: [dur0, dur1, ...],
   rests: [positions/durations],
   metadata: {
      name: “Theme A”,
      key: C minor,
      scale: natural minor,
      resolution: quarter-grid
   }
}
```

Then it renders into MuseScore notation in Notation Mode.

