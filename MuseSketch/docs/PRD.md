# 🎼 **PRD: MuseScore Sketch — A Classical Motif & Partwriting Composer for Mobile**

**Platforms:** iOS & Android (phone + tablet)
**Form Factor Strategy:** Phone-first minimalist UI with progressive enhancement on tablets
**Version:** 1.0 (MVP)

---

# 1. **Vision & Positioning**

### **Vision**

A mobile-first classical composition notebook where ideas are created as **motifs**, developed through **transformations**, and assembled into **simple contrapuntal and SATB textures**, all rendered with professional engraving via the MuseScore engine.

### **Primary Goal**

Enable composers, students, and hobbyists to **quickly shape musical ideas on-the-go** and turn them into clean, classical notation without wrestling with tiny staff elements on a phone.

### **Tagline**

> *“Sketch motifs. Shape ideas. Build counterpoint. Anywhere.”*

---

# 2. **Target Users**

### 2.1 Primary Users

* **Classical composers** (professional/amateur)
* **Music students** (high school → conservatory)
* **Choir/school/church arrangers**
* **Film/game composers needing fast idea capture**

### 2.2 Secondary Users

* **Theory students** learning counterpoint
* **Composers from DAW backgrounds** who want classical motifs quickly
* **MuseScore desktop users** who need a mobile companion

---

# 3. **Core Principles**

### 3.1 **Mobile-First Ideation**

Motifs are created through *touch-native gestures* (drawing contours, tapping scale-degree pads, recording ideas), not staff micro-editing.

### 3.2 **Notation as Output, not Interface**

The app uses MuseScore’s engraving engine during editing, but creation is abstracted above notation.

### 3.3 **Motifs as First-Class Objects**

Motifs act like “clips” (Ableton Note) — transformable, rearrangeable, deployable into counterpoint or SATB textures.

### 3.4 **Constraint-Aware Partwriting**

Voice-leading assistance enhances classical correctness, without being pedantic.

### 3.5 **Phone to Tablet Scaling**

* **Phone:** motif editing → simple sections → light SATB
* **Tablet:** full SATB editing → more staves visible simultaneously

---

# 4. **High-Level Product Structure**

1. **Sketch Library (Home)**
2. **Sketch View**

   * Motif Library
   * Sections
   * Global settings (key, tempo, scale, time signature)
3. **Motif Editor**

   * Shape Mode (primary)
   * Notation Mode (refinement)
4. **Section Editor**

   * Place motifs
   * Generate textures (2-part, 3-part, SATB, imitative)
5. **Partwriting Tools**

   * SATB view
   * Voice selection
   * Constraint warnings
6. **Playback**
7. **Export / Sync**

   * Export to .mscz
   * Export MusicXML
   * Optional cloud sync (later)

---

# 5. **Feature Requirements**

## 5.1 Motif System (Core Feature)

### **5.1.1 Motif Object Model**

Each motif contains:

* Pitch contour
* Rhythmic grid
* Key-relative intervals
* Labels (“Theme A”, “Bass Gutters”, “Counterline 1”)
* Length (1–4 bars, expandable later)

### **5.1.2 Creation: Shape Mode**

**UI Elements (Phone + Tablet):**

* **Contour Canvas:** user draws up/down curve → quantized to scale
* **Scale-Degree Pads:** 1–7 buttons

  * Swipe up/down = 3rd/4th/5th jumps
  * Hold & slide = stepwise line
* **Rhythm Strip:**

  * Tap = quarter
  * Double tap = dotted
  * Press & drag = tie
  * Pinch = compress/expand subdivisions
* **Record Button:**

  * Plays on-screen piano or pad input
  * Captures rhythm & pitch in real time

**Functional Requirements:**

* Quantize to nearest scale degree
* Quantize timing to nearest subdivision
* Undo/redo stack
* Preview playback

### **5.1.3 Notation Mode**

After shaping:

* Engrave with MuseScore engine
* Staff-only view (1 staff)
* Edits allowed:

  * adjust pitch
  * change one rhythm value
  * add slurs
  * fix accidentals
  * change duration
* Disable adding new measures (motifs are atomic)

---

## 5.2 Sections (Arrangement Workspace)

Each Sketch contains multiple **Sections** (like Ableton Clips, but classical).

### **5.2.1 Section Model**

* Name: “Intro,” “A,” “A’,” “B,” etc.
* Time length: 4–32 bars
* Key & meter (inherits from Sketch unless overridden)

### **5.2.2 Placing Motifs**

Drag motifs from the Motif Library into:

* Soprano
* Alto
* Tenor
* Bass
* Or a single melodic line

Autofit:

* Aligns motifs to bar boundaries
* Repeats motif to fill section
* Suggests harmonic underpinning if needed

---

## 5.3 Partwriting Engine

### **5.3.1 Texture Presets**

#### **A. SATB Chorale Preset**

* S: motif
* A: diatonic inner voice
* T: contrary motion to S/A
* B: outline (I–V–vi–IV or root progression)

#### **B. Two-Part Counterpoint**

* Free counterpoint with species-inspired rules
* App suggests consonant intervals
* Allows controlled dissonance (passing/neighbor)

#### **C. Imitative Texture**

* S: motif
* A: motif at 1 bar delay
* T: motif inversion
* B: augmentation of motif

#### **D. Block Chord Texture**

* Converts motif into chord plan:

  * S = melody
  * A/T/B = smooth voicing from chord suggestions

### **5.3.2 Constraint Warnings**

Non-blocking indicators:

* Parallel P5/P8
* Voice crossing
* Large leaps (> octave)
* Unprepared dissonance
* Hidden parallels (soft warning)

---

## 5.4 SATB Editing Interface

### **5.4.1 Voice Controls**

* S, A, T, B tabs
* Tap → highlight active voice
* Others dimmed
* Editing acts only on active voice

### **5.4.2 Mobile Adaptive Layout**

**Phone:**

* Only 1–2 staves visible at once
* Swipe vertically switches through SATB
* “Chorale stack” mini-map shows positions

**Tablet:**

* Full SATB vertical layout

### **5.4.3 Input Tools**

On any voice track:

* Tap-drag to change pitch
* Rhythm picker wheel (phone-friendly)
* Quick articulation buttons

---

## 5.5 Playback

### Requirements

* Section playback
* Loop motif
* Loop section
* Adjustable tempo
* Instrument sound selection (piano default)

---

## 5.6 Export

### MVP Export Options

* **.mscz** (MuseScore native)
* **MusicXML**
* **MIDI**

### Future Export Options

* Cloud sync with MuseScore.com
* QR code transfer
* AirDrop/ShareSheet hooks

---

# 6. **Non-Goals (for v1)**

* No full orchestration
* No plugin system
* No engraving settings editor
* No advanced text layout
* No page layout view
* No score-style engravable print mode
* No full StaffPad-style handwriting recognition
* No mixing console or advanced playback instruments

---

# 7. **Technical Architecture (High-Level)**

### **7.1 Core Components**

* **C++ MuseScore Engine** (engraving, model, I/O)
* **Mobile UI Layer** (Qt/QML or RN/Flutter + FFI)
* **Motif Engine Module**

  * Transformations
  * Quantization
  * Partwriting rules
* **Section Manager**

  * Motif arrangement
  * Bar alignment
* **Renderer**

  * Score → image/texture
  * Adaptive zooming

---

# 8. **UX Principles for Phone + Tablet**

### **8.1 Phone**

* Focus on Motif Editing
* One/two-voice partwriting
* SATB via vertical-swiping
* Minimalist UI
* Modal screens for motif creation

### **8.2 Tablet**

* Full SATB
* Drag motifs across big canvas
* Split-view motif + section
* Larger contour canvas
* Optional pencil input (tap-notes & contour sketching)

---

# 9. **Risks & Mitigations**

### **R1. Engraving engine complexity**

* ✔ Mitigation: limit editing scope inside notation mode
* ✔ Mitigation: no multi-page layout v1

### **R2. Mobile screen constraints**

* ✔ Motif Shape Mode avoids staff entry
* ✔ Voice-swapping on phone reduces clutter

### **R3. Algorithmic partwriting unpredictability**

* ✔ Constrain outputs to simple presets
* ✔ Provide “cycle suggestions” button
* ✔ Allow manual override

---

# 10. **v1 Success Criteria**

### Qualitative

* Users report: “It feels easier to sketch a melody here than in any notation app.”
* Students use motifs for theory/homework
* Composers export sketches to MuseScore Studio

### Quantitative

* 2+ motifs created per user per session
* > 50% of motifs exported to sections
* > 40% of users export a score to desktop within 2 weeks

---

# 11. Future Roadmap (Post-v1)

* Handwriting/Pencil input (limited)
* AI-assisted counterpoint suggestions
* Built-in voice-leading exercises
* Film scoring “sketch cue” mode
* Motif similarity searching (“find variants”)
* Harmonic analysis overlay
* Performance-mode playback
* Cloud sync & collaboration

