Nice, this is the fun one 😈—you’re basically asking “how do I build a tiny Bach-inspired chorale machine” without going full PhD.

Below is an algorithm that’s:

* **Good enough for v1** (musical, reasonably voice-leading aware)
* **Deterministic & implementable** (no ML required)
* Structured in **clear stages** you can map directly to PR-08 tasks.

I’ll write it as an engineering-oriented spec.

---

## 0. Inputs & Data Model

Assume you have:

* `key`: tonic + mode (e.g. C major, A minor)
* `melody`: list of notes with:

  * `pitch` (MIDI or diatonic degree)
  * `dur` (in beats)
  * `beat` (position in measure)
  * `measureIndex`
* `timeSig`: e.g. 4/4
* Optional: explicit phrase boundaries, cadences (or infer heuristically)

You’ll output:

* SATB voices:

  * `soprano` (given melody)
  * `alto`, `tenor`, `bass` (generated)
* Each voice is a sequence of notes (or tied notes) aligned to the same rhythmic grid you use for quantized motifs.

---

## 1. Stage A — Build the Harmonic Skeleton

### 1.1 Decide harmonic rhythm: where chords change

Heuristic for 4/4 (can generalize later):

* Start with **one harmony per beat**.
* You **must** include a harmony on:

  * Beat 1 of every measure.
  * Any longer note (dur >= 1 beat) that starts on a strong beat (1 or 3).
* For fast moving notes:

  * Treat off-beat shorter notes as non-chord tones *unless* they land on a clear accent (e.g. syncopation).
* Collapse adjacent beats into same harmony when:

  * Melody notes are stepwise and clearly “decorating” the same harmony (passing/neighbor tones).
  * The harmonic rhythm would otherwise feel too busy (e.g. 4 different chords in a measure).

Implementation sketch:

```pseudo
function segmentMelodyIntoHarmonySlots(melody, timeSig):
    slots = []
    for each measure:
        for each beat in measure:
            candidateNotes = notes starting on this beat
            if candidateNotes is empty: continue

            principalNote = longest duration note starting on or just before this beat
            slots.append(HarmonySlot(startTime, principalNote))
    # Optional: merge adjacent slots based on heuristics
    return slots
```

Each `HarmonySlot` holds:

* `startTime`
* `endTime`
* `principalMelodyNote`

You’ll eventually assign exactly **one chord** to each slot.

---

### 1.2 Propose candidate chords per slot

For each `HarmonySlot`, based on **scale degree of melody** (relative to key), pick chord candidates.

Example for major key (simplified):

* Degree 1 (tonic): I, vi, IV
* Degree 2: ii, V (with 2 as 9th or 7th), IV (2 as 6th)
* Degree 3: I (3rd), vi, iii
* Degree 4: IV, ii, V7 (4 as 7th)
* Degree 5: V, I (5th), iii
* Degree 6: vi, IV, ii
* Degree 7: V6, vii° (leading tone chord), or as non-chord tone

Algorithm sketch:

```pseudo
function candidateChordsForNote(scaleDegree, keyMode, isStrongBeat):
    # Return small list of (Roman numeral, functionTag) pairs
```

Each slot gets `slot.candidates = [ChordCandidate]`.

---

### 1.3 Choose a chord progression (functional smoothing)

Now pick **one chord per slot** that:

* Follows tonal/function logic: Tonic (T) → Predominant (PD) → Dominant (D) → T.
* Avoids weird back-tracking (D → PD, etc.).
* Leads to cadences near phrase ends:

  * Perfect cadence: V → I
  * Plagal: IV → I
  * Imperfect: I → V or II → V etc.

You can model this as **scored dynamic programming**:

* State: `slotIndex`, `prevFunc`, `prevChord`.
* Transition: assign one candidate to current slot.
* Score components:

  * * for sensible function progressions (T→PD, PD→D, D→T).
  * * for repeated tonic at stable points.
  * − for jumps like D→PD or weird leaps.
  * * for matching phrase-end patterns (V→I).

Implementation sketch:

```pseudo
function chooseChordProgression(slots, key):
    # dynamic programming over slots with scoring
    # return bestScored list of Chord objects (root, quality, inversion placeholder)
```

You can keep this very small: only a few candidates per slot → tractable even with simple DP.

---

## 2. Stage B — Initial SATB Voicings per Chord

Given a chord progression, now you voice each chord into SATB.

### 2.1 Define ranges & typical tessitura

Use MIDI or absolute pitch:

* Soprano: C4–G5 (ideal: G4–E5)
* Alto: G3–D5 (ideal: C4–A4)
* Tenor: C3–G4 (ideal: E3–E4)
* Bass: E2–C4 (ideal: G2–E3)

### 2.2 First chord voicing

Goal: chord tones filling root, 3rd, 5th, maybe doubling root.

Procedure:

1. Soprano pitch is given by melody.
2. Choose bass:

   * Usually root in root position.
   * At phrase starts, root position I is safest.
3. Place tenor & alto:

   * Choose chord tones within range.
   * Keep spacing:

     * S–A, A–T ≤ octave.
     * T–B can be up to a 12th but avoid big gaps for v1.
   * Double root **unless** you have good reason (e.g. in first inversion use double soprano or bass occasionally, but v1 can be “always double root”).

You can implement this as a small search:

```pseudo
function initialVoicingForChord(chord, sopranoPitch):
    # generate possible Bass choices (root mostly)
    # generate sets of Alto, Tenor from remaining chord tones
    # choose the one closest to mid-range with correct spacing
```

---

## 3. Stage C — Voice-Leading between Chords

Now walk through chords sequentially, voicing each given the previous chord’s SATB.

For each new chord at time `t`:

1. **Keep common tones** in same voice where possible.
2. Move other voices by **step or small interval** to nearest chord tone.
3. Prefer **contrary or oblique motion** to the bass.
4. Avoid:

   * Parallel P5/P8 between any pair of voices.
   * Direct (hidden) P5/P8 between outer voices with a big leap in the soprano.
   * Voice crossing (S below A, etc.).

### 3.1 Core algorithm (per chord)

```pseudo
function revoiceChord(prevVoicing, chord, sopranoPitch):
    candidates = generateAllReasonableVoicings(chord, sopranoPitch)

    bestVoicing = null
    bestScore = -inf

    for v in candidates:
        if violatesRangeOrSpacing(v): continue

        score = 0
        score += preserveCommonTonesScore(prevVoicing, v)
        score += smoothMotionScore(prevVoicing, v) # penalize large leaps
        score += contraryMotionScore(prevVoicing, v)
        score -= parallelPerfectsPenalty(prevVoicing, v)
        score -= voiceCrossingPenalty(v)

        if score > bestScore:
            bestScore = score
            bestVoicing = v

    if bestVoicing == null:
        # fallback: ignore parallels, just pick smoothest
        bestVoicing = fallbackVoicing(prevVoicing, chord, sopranoPitch)

    return bestVoicing
```

### 3.2 Generating “reasonable” voicings

Don’t generate *all* combinations; constrain:

* Bass:

  * root or 1st inversion at specific structural points.
* Alto/Tenor:

  * Only chord tones.
  * Stay within ±3 or 4 diatonic steps of previous note where possible.
  * Avoid doubled 3rd in major triads (v1 can just discourage).

This keeps search small (often < 10 candidates per chord).

---

## 4. Stage D — Cadences & Special Patterns

Handle phrase ends specially (last 2–3 slots of a phrase).

### 4.1 Identify cadence slots

Heuristic:

* End of sketch or marked phrase.
* Last strong-beat chord before a rest/long note or before the final barline.

Check chosen harmonic functions:

* If last chord is I:

  * Try to enforce V→I in previous slot.
* If last chord is V:

  * The cadence is half (I→V or something→V).

### 4.2 Apply cadence voicing templates

**Perfect Authentic Cadence (V→I):**

* V:

  * Bass: V
  * Soprano: 2 or 7 resolving to 1 on I
  * Leading tone (7) in an upper voice → resolves up to 1
* I:

  * Bass: I
  * Soprano: 1 (for true PAC)
  * Double root

You can hard-code small templates:

```pseudo
if isCadence and chord_n_minus_1 == V and chord_n == I:
    voicing_n_minus_1 = chooseVoicingFromTemplate(V, soprano_n_minus_1)
    voicing_n = chooseVoicingFromTemplate(I, soprano_n)
```

This ensures your cadences “snap” into something Bach-ish even if earlier harmony is rougher.

---

## 5. Stage E — Optional Inner-Voice Embellishments

Once basic chordal SATB is in place, you can (optionally) sprinkle simple linear motions:

* Passing tones:

  * When Alto/Tenor moves from chord tone A to chord tone C a third apart over 2 beats, insert B as passing tone on the off-beat.
* Suspensions:

  * Over cadences, create 4–3 or 7–6 suspensions in upper voices:

    * Hold previous chord tone over the bar, then resolve.

This is v1.5/v2 territory, but the hook is:

* First get **blockish but legal** chord tones.
* Then refine with a second pass walking each inner voice and spotting opportunities.

---

## 6. Connecting Back to MuseSketch / PR-08

For **PR-08 – Partwriting Engine v1: SATB Chorale Texture**, you can structure subtasks as:

1. **PR-08.1 – Harmonic Skeleton**

   * Implement `segmentMelodyIntoHarmonySlots`.
   * Implement `candidateChordsForNote`.
   * Implement DP function `chooseChordProgression`.

2. **PR-08.2 – Basic Chord Voicing Engine**

   * Implement `initialVoicingForChord`.
   * Implement `generateAllReasonableVoicings`.
   * Implement `revoiceChord` with scoring.

3. **PR-08.3 – Cadence Handling**

   * Detect phrase ends.
   * Snap last chords to V→I / IV→I where appropriate.
   * Implement cadence voicing templates.

4. **PR-08.4 – Integration with MuseScore Model**

   * Take SATB voicing results and instantiate MuseScore score objects.
   * Align durations to `HarmonySlot` spans (ties across slots as needed).

5. **PR-08.5 – Quality Guardrails**

   * Add debug metrics: count parallels, voice crossings, range violations.
   * Add a “quality score” that could gate whether you show a “try another voicing” button later.

Great question — **syncopation in a chorale-style generator** is tricky because traditional SATB (Bach-style) rarely uses *heavy* syncopation, but **light syncopation** *does* occur, especially in:

* suspensions
* accented passing tones
* syncopated alto/tenor inner voices
* anticipations
* tied notes across beats

So the algorithm must treat syncopation as **controlled dissonance**, not as a groove or rhythm engine.

Below is a complete framework to extend your SATB Chorale Engine to handle syncopation *musically and safely*.

---

# 🎼 **Syncopation Extension for SATB Chorale Generator (v1.5)**

Syncopation appears in classical chorales when **metrical accents are shifted** via:

1. **Ties across beats**
2. **Suspensions**
3. **Accented passing tones**
4. **Anticipations**
5. **Offbeat chord tones** (rare, but possible in inner voices)

Here’s how to incorporate each one safely.

---

# 1. **Add Syncopation Through Suspensions (Primary Method)**

Suspension pattern:
**Preparation → Suspension → Resolution**

Example in soprano or alto:

* Preparation: quarter note **on beat 4** (consonant)
* Suspension: tie into beat 1 (dissonant)
* Resolution: step downward on beat 2 (consonant)

### **Algorithm Rule**

When the melodic motif has a strong beat note (beat 1/3), check:

```
If the previous chord tone == a step above the next melody note:
    Create a 4-3 suspension in alto or tenor
```

### **Priority voices**

* Best: **alto**, then **tenor**
* Rare: soprano (melody should not be overly syncopated)
* Possible: bass (2-3 suspension)

### **Safety Checks**

* Do NOT suspend into parallel 5ths with bass
* Only allow suspension if resolution is available on beat 2 or beat 4
* Limit consecutive suspensions to avoid rhythmic busyness

---

# 2. **Accented Passing Tones (Mild Syncopation)**

Accent = a non-chord tone on a strong beat.

**Use case:** When motif moves stepwise and passes through chord changes.

### **Algorithm**

If the melody leaps by 3rds or 4ths across a harmony change:

```
Insert passing tone in inner voice:
    On beat 2 or beat 4
    If stepwise motion is available
    If results in acceptable dissonance (accented passing: 2, 7)
```

### **Rules**

* Only allow if the passing tone is **stepwise** from both neighbors
* Must resolve by step
* Avoid doubling tendency tones
* Avoid accented 4ths above bass (consonance exception: IV chord)

---

# 3. **Anticipations (Forward-Leaning Syncopation)**

Anticipation = note borrowed from the *next* chord, placed early.

### **Algorithm**

When melody cadences or repeats a motive:

```
If next chord == current chord OR compatible:
    Allow anticipation in soprano or alto on 8th notes
```

### **Example**

Beat 4 anticipates beat 1 of next measure:

```
Beat 4: A (anticipation) 
Beat 1: A (chord tone)
```

### **Rules**

* Use sparingly (max once every 2–4 bars)
* Unaccented anticipation (ideal placement: beat 4+)

---

# 4. **Tied Notes Across Strong Beats**

This gives the **feeling** of syncopation.

### **Algorithm**

When motif sustains a tone:

```
Tie the alto or tenor note from weak → strong beat IF:
    - Harmony stays the same OR
    - You're intentionally creating a suspension (see §1)
```

### **Rules**

* Only allow ties across: 2 → 3 or 4 → 1
* Only if the tied note is in the chord
* Avoid creating forbidden dissonances unless intentional suspension

---

# 5. **Offbeat Chord Tones (Light Syncopation for Color)**

In inner voices only.

### **Algorithm**

Random chance (low probability):

```
On 8th-note pickup or beat 2.5 / 3.5:
    Replace rest with chord tone that:
        - Is NOT doubled from soprano
        - Does NOT create spacing violations
        - Moves stepwise into next beat
```

### **Use Cases**

* To give inner voices slight rhythmic interest
* Helps music feel more “modern” but still conservative

---

# 6. **Combine Syncopation Techniques (Intensity Levels)**

### **Level 0 (Default Bach Mode)**

* No syncopation except suspensions
* Ideal for strict chorales

### **Level 1 (Moderate)**

Use:

* 4–3, 7–6 suspensions
* occasional anticipations
* inner-voice passing tones

### **Level 2 (Expressive Classical)**

Use:

* tied-note syncopations
* deeper inner-voice movement
* accept certain accented dissonances

Your app can expose this as:
**Syncopation: Off / Light / Moderate / Expressive**

---

# 7. **Placing Syncopation Relative to the Motif**

The motif drives metric shape.

### **Motif → Syncopation Mapping**

* Motif long notes → good suspension opportunities
* Motif repeated notes → good anticipation or tied-note chances
* Motif stepwise sequences → good for accented passing tones
* Motif leaps → avoid syncopation in soprano; inner voices only

### **Algorithmic Rule**

```
Analyze motif rhythm:
    If motif has long or held notes:
        add suspensions
    If motif has repeated pitches:
        add anticipations
    If motif has stepwise motion:
        add inner passing tones
```

---

# 8. **Full Syncopation Handling Block (Pseudo-code)**

```
for each measure m:
    for each beat b:

        # 1. Suspensions
        if motif has long note ending on strong beat:
            create_possible_suspension_in_inner_voice()

        # 2. Anticipations
        if beat == 4 and next_chord == neighbor_chord:
            maybe_insert_anticipation()

        # 3. Passing tones
        if melody_moves_by_step and chord_changes:
            insert_accented_passing_tone_in_inner_voice()

        # 4. Ties for syncopation
        if inner_voice_chord_tone and measure_boundary_crossing:
            tie_if_safe()

        # 5. Fill rests
        if inner_voice_rest and syncopation_level > 1:
            consider_offbeat_chord_tone()

return final_SATB_texture
```

---

# 9. Summary

Syncopation is **not random rhythm**, but **structured delay or anticipation of harmonic resolution**.

For chorale-style SATB, use these safe tools:

1. **Suspensions** (core method)
2. **Accented passing tones** (light syncopation)
3. **Anticipations** (only on weak beats)
4. **Tied notes across beats**
5. **Occasional inner-voice offbeats**

This keeps your chorale musical, historically grounded, and playable — while letting motifs feel more alive.

