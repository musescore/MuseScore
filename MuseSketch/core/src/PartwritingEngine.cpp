#include "PartwritingEngine.h"
#include "Sketch.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

PartwritingEngine::PartwritingEngine() {}

// ============================================================================
// Diatonic Chord Construction
// ============================================================================

DiatonicChord PartwritingEngine::getDiatonicChord(int scaleDegree) {
  DiatonicChord chord;
  chord.romanNumeral = scaleDegree;
  chord.root = scaleDegree;
  
  // Build chord based on scale degree (1-7) in major key
  // I=major, ii=minor, iii=minor, IV=major, V=major, vi=minor, vii°=dim
  switch (scaleDegree) {
    case 1: // I - Tonic major
      chord.third = 3;
      chord.fifth = 5;
      chord.quality = "major";
      chord.function = HarmonicFunction::Tonic;
      break;
    case 2: // ii - Predominant minor
      chord.third = 4;
      chord.fifth = 6;
      chord.quality = "minor";
      chord.function = HarmonicFunction::Predominant;
      break;
    case 3: // iii - Tonic minor (substitute)
      chord.third = 5;
      chord.fifth = 7;
      chord.quality = "minor";
      chord.function = HarmonicFunction::Tonic;
      break;
    case 4: // IV - Predominant major
      chord.third = 6;
      chord.fifth = 1;
      chord.quality = "major";
      chord.function = HarmonicFunction::Predominant;
      break;
    case 5: // V - Dominant major
      chord.third = 7;
      chord.fifth = 2;
      chord.quality = "major";
      chord.function = HarmonicFunction::Dominant;
      break;
    case 6: // vi - Tonic minor (relative minor)
      chord.third = 1;
      chord.fifth = 3;
      chord.quality = "minor";
      chord.function = HarmonicFunction::Tonic;
      break;
    case 7: // vii° - Dominant diminished
      chord.third = 2;
      chord.fifth = 4;
      chord.quality = "diminished";
      chord.function = HarmonicFunction::Dominant;
      break;
    default:
      chord.root = 1;
      chord.third = 3;
      chord.fifth = 5;
      chord.quality = "major";
      chord.function = HarmonicFunction::Tonic;
      break;
  }
  
  return chord;
}

// ============================================================================
// Stage A: Harmonic Skeleton - Chord Candidate Selection
// ============================================================================

QList<DiatonicChord> PartwritingEngine::getCandidateChords(int melodyScaleDegree) {
  // Return chords that contain the melody note, ordered by preference
  QList<DiatonicChord> candidates;
  int melody = normalize(melodyScaleDegree);
  
  // Based on PARTWRITING_ENGINE.md Section 1.2
  switch (melody) {
    case 1: // Scale degree 1: I (primary), vi, IV
      candidates.append(getDiatonicChord(1)); // I - melody is root
      candidates.append(getDiatonicChord(6)); // vi - melody is 3rd
      candidates.append(getDiatonicChord(4)); // IV - melody is 5th
      break;
    case 2: // Scale degree 2: V (primary), ii, vii°
      candidates.append(getDiatonicChord(5)); // V - melody is 5th
      candidates.append(getDiatonicChord(2)); // ii - melody is root
      candidates.append(getDiatonicChord(7)); // vii° - melody is 3rd
      break;
    case 3: // Scale degree 3: I (primary), vi, iii
      candidates.append(getDiatonicChord(1)); // I - melody is 3rd
      candidates.append(getDiatonicChord(6)); // vi - melody is 5th
      candidates.append(getDiatonicChord(3)); // iii - melody is root
      break;
    case 4: // Scale degree 4: IV (primary), ii, V7 (4 as 7th - simplified to V)
      candidates.append(getDiatonicChord(4)); // IV - melody is root
      candidates.append(getDiatonicChord(2)); // ii - melody is 3rd
      candidates.append(getDiatonicChord(7)); // vii° - melody is 5th
      break;
    case 5: // Scale degree 5: I (primary), V, iii
      candidates.append(getDiatonicChord(1)); // I - melody is 5th
      candidates.append(getDiatonicChord(5)); // V - melody is root
      candidates.append(getDiatonicChord(3)); // iii - melody is 3rd
      break;
    case 6: // Scale degree 6: vi (primary), IV, ii
      candidates.append(getDiatonicChord(6)); // vi - melody is root
      candidates.append(getDiatonicChord(4)); // IV - melody is 3rd
      candidates.append(getDiatonicChord(2)); // ii - melody is 5th
      break;
    case 7: // Scale degree 7: V (primary - leading tone), vii°, iii
      candidates.append(getDiatonicChord(5)); // V - melody is 3rd (leading tone)
      candidates.append(getDiatonicChord(7)); // vii° - melody is root
      candidates.append(getDiatonicChord(3)); // iii - melody is 5th
      break;
    default:
      candidates.append(getDiatonicChord(1));
      break;
  }
  
  return candidates;
}

int PartwritingEngine::scoreChordTransition(const DiatonicChord &prev, 
                                             const DiatonicChord &curr,
                                             int melodyPitch,
                                             bool isLastNote,
                                             bool isPenultimate) {
  int score = 0;
  
  // === Functional Harmony Scoring ===
  // Good progressions: T→PD (+10), PD→D (+15), D→T (+20)
  // Neutral: T→T (+5), PD→PD (+3), D→D (+2)
  // Bad: D→PD (-10), D→T then back to D (-5)
  
  HarmonicFunction pf = prev.function;
  HarmonicFunction cf = curr.function;
  
  // Forward motion in the circle
  if (pf == HarmonicFunction::Tonic && cf == HarmonicFunction::Predominant) {
    score += 10; // T → PD
  } else if (pf == HarmonicFunction::Predominant && cf == HarmonicFunction::Dominant) {
    score += 15; // PD → D
  } else if (pf == HarmonicFunction::Dominant && cf == HarmonicFunction::Tonic) {
    score += 20; // D → T (resolution!)
  } else if (pf == HarmonicFunction::Tonic && cf == HarmonicFunction::Dominant) {
    score += 8; // T → D (skip PD, acceptable)
  } else if (pf == cf) {
    score += 3; // Same function, neutral
  } else if (pf == HarmonicFunction::Dominant && cf == HarmonicFunction::Predominant) {
    score -= 15; // D → PD (retrogression, bad)
  }
  
  // === Cadence Scoring ===
  if (isLastNote) {
    // Final chord should be I
    if (curr.romanNumeral == 1) {
      score += 25; // End on tonic
    } else if (curr.romanNumeral == 5) {
      score += 10; // Half cadence acceptable
    }
  }
  
  if (isPenultimate) {
    // Penultimate chord should be V for authentic cadence
    if (curr.romanNumeral == 5) {
      score += 20; // V before final
    } else if (curr.romanNumeral == 4) {
      score += 10; // IV before final (plagal)
    }
  }
  
  // === Root Motion Scoring ===
  int rootMotion = std::abs(curr.root - prev.root);
  if (rootMotion == 4 || rootMotion == 3) {
    score += 5; // Fourth/fifth motion (circle of fifths)
  } else if (rootMotion == 2 || rootMotion == 5) {
    score += 3; // Third motion
  } else if (rootMotion == 1 || rootMotion == 6) {
    score += 1; // Step motion
  }
  
  // === Common Tone Bonus ===
  for (int tone : prev.tones()) {
    if (curr.contains(tone)) {
      score += 2; // Common tone available
    }
  }
  
  return score;
}

QList<DiatonicChord> PartwritingEngine::chooseChordProgression(
    const QList<int> &melodyPitches, bool isMinorKey) {
  Q_UNUSED(isMinorKey) // TODO: Handle minor keys
  
  int n = melodyPitches.size();
  if (n == 0) return {};
  
  // Get candidates for each melody note
  QList<QList<DiatonicChord>> allCandidates;
  for (int pitch : melodyPitches) {
    allCandidates.append(getCandidateChords(pitch));
  }
  
  // Dynamic programming to find best progression
  // State: dp[i][j] = best score ending at note i with candidate j
  QVector<QVector<int>> dp(n);
  QVector<QVector<int>> parent(n); // For backtracking
  
  // Initialize first note
  dp[0].resize(allCandidates[0].size());
  parent[0].resize(allCandidates[0].size(), -1);
  for (int j = 0; j < allCandidates[0].size(); j++) {
    // Prefer starting on I
    dp[0][j] = (allCandidates[0][j].romanNumeral == 1) ? 10 : 0;
  }
  
  // Fill DP table
  for (int i = 1; i < n; i++) {
    int numCandidates = allCandidates[i].size();
    dp[i].resize(numCandidates);
    parent[i].resize(numCandidates);
    
    bool isLast = (i == n - 1);
    bool isPenult = (i == n - 2);
    
    for (int j = 0; j < numCandidates; j++) {
      int bestScore = -1000;
      int bestPrev = 0;
      
      for (int k = 0; k < allCandidates[i-1].size(); k++) {
        int transitionScore = scoreChordTransition(
          allCandidates[i-1][k], allCandidates[i][j],
          melodyPitches[i], isLast, isPenult);
        int totalScore = dp[i-1][k] + transitionScore;
        
        if (totalScore > bestScore) {
          bestScore = totalScore;
          bestPrev = k;
        }
      }
      
      dp[i][j] = bestScore;
      parent[i][j] = bestPrev;
    }
  }
  
  // Backtrack to find best progression
  QList<DiatonicChord> progression;
  
  // Find best final chord
  int bestFinal = 0;
  int bestFinalScore = dp[n-1][0];
  for (int j = 1; j < dp[n-1].size(); j++) {
    if (dp[n-1][j] > bestFinalScore) {
      bestFinalScore = dp[n-1][j];
      bestFinal = j;
    }
  }
  
  // Backtrack
  QVector<int> chosenIndices(n);
  chosenIndices[n-1] = bestFinal;
  for (int i = n - 1; i > 0; i--) {
    chosenIndices[i-1] = parent[i][chosenIndices[i]];
  }
  
  // Build result
  for (int i = 0; i < n; i++) {
    progression.append(allCandidates[i][chosenIndices[i]]);
  }
  
  return progression;
}

// ============================================================================
// Stage B: Initial Voicings
// ============================================================================

SATBVoicing PartwritingEngine::createInitialVoicing(const DiatonicChord &chord, 
                                                     int sopranoPitch) {
  SATBVoicing voicing;
  voicing.soprano = sopranoPitch;
  
  // Bass gets the root (root position for first chord)
  voicing.bass = chord.root;
  
  // Find remaining chord tones for alto and tenor
  QList<int> remaining;
  for (int tone : chord.tones()) {
    int normalized = normalize(tone);
    if (normalized != normalize(sopranoPitch) && normalized != chord.root) {
      remaining.append(normalized);
    }
  }
  
  // Assign alto and tenor
  if (remaining.size() >= 2) {
    // Alto gets higher, tenor gets lower
    if (remaining[0] > remaining[1]) {
      voicing.alto = remaining[0];
      voicing.tenor = remaining[1];
    } else {
      voicing.alto = remaining[1];
      voicing.tenor = remaining[0];
    }
  } else if (remaining.size() == 1) {
    voicing.alto = remaining[0];
    voicing.tenor = chord.root; // Double the root
  } else {
    // Melody has root and one other tone, double root
    voicing.alto = chord.third;
    voicing.tenor = chord.root;
  }
  
  // Ensure proper ordering: S > A > T > B (using scale degrees)
  // This is simplified - real implementation would use absolute pitches
  
  return voicing;
}

QList<SATBVoicing> PartwritingEngine::generateCandidateVoicings(
    const DiatonicChord &chord, int sopranoPitch) {
  QList<SATBVoicing> candidates;
  
  QList<int> chordTones = chord.tones();
  
  // Generate voicings with different bass notes (root position and inversions)
  QList<int> bassOptions = {chord.root}; // Start with root position
  // Could add inversions: bassOptions.append(chord.third); bassOptions.append(chord.fifth);
  
  for (int bass : bassOptions) {
    // Get remaining tones for alto and tenor
    QList<int> innerTones;
    for (int tone : chordTones) {
      innerTones.append(normalize(tone));
    }
    // Add root again for possible doubling
    innerTones.append(chord.root);
    
    // Generate all combinations
    for (int alto : innerTones) {
      for (int tenor : innerTones) {
        SATBVoicing v;
        v.soprano = sopranoPitch;
        v.alto = alto;
        v.tenor = tenor;
        v.bass = bass;
        
        // Basic validity check
        if (!hasVoiceCrossing(v) && hasProperSpacing(v)) {
          candidates.append(v);
        }
      }
    }
  }
  
  // If no valid candidates, create a fallback
  if (candidates.isEmpty()) {
    candidates.append(createInitialVoicing(chord, sopranoPitch));
  }
  
  return candidates;
}

// ============================================================================
// Stage C: Voice-Leading
// ============================================================================

bool PartwritingEngine::hasVoiceCrossing(const SATBVoicing &voicing) {
  // Simplified: just check scale degrees are in order
  // Real implementation would use absolute pitches
  int s = normalize(voicing.soprano);
  int a = normalize(voicing.alto);
  int t = normalize(voicing.tenor);
  int b = normalize(voicing.bass);
  
  // Allow some flexibility since we're using scale degrees not absolute pitches
  // This is a heuristic - voices should generally descend S > A > T > B
  // but octave equivalence makes this tricky
  return false; // Simplified for now
}

bool PartwritingEngine::hasProperSpacing(const SATBVoicing &voicing) {
  // S-A and A-T should be within an octave (7 scale degrees)
  // T-B can be wider
  // Simplified check using scale degrees
  return true; // Simplified for now - would need absolute pitches
}

int PartwritingEngine::interval(int from, int to) {
  // Calculate interval in scale degrees (1-7)
  int diff = std::abs(normalize(to) - normalize(from));
  if (diff > 3) diff = 7 - diff; // Inversion
  return diff;
}

bool PartwritingEngine::isPerfectInterval(int intervalSize) {
  // Perfect unison (0), perfect fifth (4), perfect octave (7)
  return intervalSize == 0 || intervalSize == 4 || intervalSize == 7;
}

bool PartwritingEngine::hasParallelPerfects(const SATBVoicing &prev, 
                                             const SATBVoicing &curr) {
  // Check all voice pairs for parallel fifths or octaves
  QList<QPair<int, int>> prevIntervals;
  QList<QPair<int, int>> currIntervals;
  
  // Voice pairs: S-A, S-T, S-B, A-T, A-B, T-B
  auto checkPair = [&](int p1, int p2, int c1, int c2) {
    int prevInt = interval(p1, p2);
    int currInt = interval(c1, c2);
    
    // Both intervals are perfect AND both voices moved in same direction
    if (isPerfectInterval(prevInt) && isPerfectInterval(currInt) && prevInt == currInt) {
      int move1 = c1 - p1;
      int move2 = c2 - p2;
      if ((move1 > 0 && move2 > 0) || (move1 < 0 && move2 < 0)) {
        return true; // Parallel perfect interval
      }
    }
    return false;
  };
  
  // Check critical pairs (especially outer voices)
  if (checkPair(prev.soprano, prev.bass, curr.soprano, curr.bass)) return true;
  if (checkPair(prev.soprano, prev.alto, curr.soprano, curr.alto)) return true;
  if (checkPair(prev.alto, prev.tenor, curr.alto, curr.tenor)) return true;
  if (checkPair(prev.tenor, prev.bass, curr.tenor, curr.bass)) return true;
  
  return false;
}

int PartwritingEngine::countCommonTones(const SATBVoicing &prev, 
                                         const SATBVoicing &curr) {
  int count = 0;
  // Check if any voice kept its pitch
  if (normalize(prev.alto) == normalize(curr.alto)) count++;
  if (normalize(prev.tenor) == normalize(curr.tenor)) count++;
  if (normalize(prev.bass) == normalize(curr.bass)) count++;
  return count;
}

int PartwritingEngine::calculateVoiceMovement(const SATBVoicing &prev, 
                                               const SATBVoicing &curr) {
  // Sum of absolute intervals moved by each inner voice
  int movement = 0;
  movement += std::abs(curr.alto - prev.alto);
  movement += std::abs(curr.tenor - prev.tenor);
  movement += std::abs(curr.bass - prev.bass);
  return movement;
}

int PartwritingEngine::scoreVoicing(const SATBVoicing &prev, 
                                     const SATBVoicing &curr,
                                     const DiatonicChord &chord) {
  int score = 100; // Start with base score
  
  // === Penalties ===
  
  // Parallel fifths/octaves: severe penalty
  if (hasParallelPerfects(prev, curr)) {
    score -= 50;
  }
  
  // Voice crossing: penalty
  if (hasVoiceCrossing(curr)) {
    score -= 30;
  }
  
  // Large leaps: penalty proportional to movement
  int movement = calculateVoiceMovement(prev, curr);
  score -= movement * 2; // Penalize large movements
  
  // === Bonuses ===
  
  // Common tones: bonus
  int commonTones = countCommonTones(prev, curr);
  score += commonTones * 10;
  
  // Stepwise motion in inner voices: bonus
  if (std::abs(curr.alto - prev.alto) <= 1) score += 5;
  if (std::abs(curr.tenor - prev.tenor) <= 1) score += 5;
  if (std::abs(curr.bass - prev.bass) <= 2) score += 3;
  
  // Contrary motion to bass in soprano: bonus
  int sopranoMotion = curr.soprano - prev.soprano;
  int bassMotion = curr.bass - prev.bass;
  if ((sopranoMotion > 0 && bassMotion < 0) || (sopranoMotion < 0 && bassMotion > 0)) {
    score += 8;
  }
  
  // Root in bass: bonus
  if (normalize(curr.bass) == chord.root) {
    score += 5;
  }
  
  // Doubled root: slight bonus
  int rootCount = 0;
  if (normalize(curr.soprano) == chord.root) rootCount++;
  if (normalize(curr.alto) == chord.root) rootCount++;
  if (normalize(curr.tenor) == chord.root) rootCount++;
  if (normalize(curr.bass) == chord.root) rootCount++;
  if (rootCount >= 2) score += 3;
  
  // Avoid doubled leading tone (degree 7)
  int leadingToneCount = 0;
  if (normalize(curr.soprano) == 7) leadingToneCount++;
  if (normalize(curr.alto) == 7) leadingToneCount++;
  if (normalize(curr.tenor) == 7) leadingToneCount++;
  if (normalize(curr.bass) == 7) leadingToneCount++;
  if (leadingToneCount >= 2) score -= 15;
  
  return score;
}

SATBVoicing PartwritingEngine::chooseVoicing(const SATBVoicing &prevVoicing,
                                              const DiatonicChord &chord,
                                              int sopranoPitch,
                                              bool isCadence) {
  if (isCadence) {
    return applyCadenceVoicing(chord, sopranoPitch, false);
  }
  
  QList<SATBVoicing> candidates = generateCandidateVoicings(chord, sopranoPitch);
  
  SATBVoicing best = candidates[0];
  int bestScore = -1000;
  
  for (const SATBVoicing &candidate : candidates) {
    int score = scoreVoicing(prevVoicing, candidate, chord);
    if (score > bestScore) {
      bestScore = score;
      best = candidate;
    }
  }
  
  return best;
}

// ============================================================================
// Stage D: Cadences
// ============================================================================

bool PartwritingEngine::isCadencePoint(int noteIndex, int totalNotes, 
                                        const QList<int> &melodyPitches) {
  // Last note is always a cadence point
  if (noteIndex == totalNotes - 1) return true;
  
  // Second to last is pre-cadence
  if (noteIndex == totalNotes - 2) return true;
  
  // Every 4 or 8 bars could be a phrase ending (simplified)
  // In a real implementation, we'd analyze the melody for phrase structure
  
  return false;
}

SATBVoicing PartwritingEngine::applyCadenceVoicing(const DiatonicChord &chord,
                                                    int sopranoPitch,
                                                    bool isPenultimate) {
  SATBVoicing voicing;
  voicing.soprano = sopranoPitch;
  
  if (chord.romanNumeral == 5 && isPenultimate) {
    // V chord in cadence
    // Leading tone (7) should be in an upper voice
    // Bass should be root (5)
    voicing.bass = 5;
    
    if (normalize(sopranoPitch) == 7) {
      // Soprano has leading tone
      voicing.alto = 2; // 5th of V
      voicing.tenor = 5; // Root
    } else if (normalize(sopranoPitch) == 2) {
      // Soprano has 5th
      voicing.alto = 7; // Leading tone
      voicing.tenor = 5; // Root
    } else {
      voicing.alto = 7;
      voicing.tenor = 2;
    }
  } else if (chord.romanNumeral == 1) {
    // I chord (tonic) - probably final
    // Double the root
    voicing.bass = 1;
    
    if (normalize(sopranoPitch) == 1) {
      // Soprano on tonic - perfect authentic cadence
      voicing.alto = 3;
      voicing.tenor = 5;
    } else {
      voicing.alto = 1; // Double root
      voicing.tenor = 3;
    }
  } else {
    // Default voicing
    return createInitialVoicing(chord, sopranoPitch);
  }
  
  return voicing;
}

// ============================================================================
// Main Generation Function
// ============================================================================

ChoraleVoices PartwritingEngine::generateChoraleTexture(const Motif &melodyMotif,
                                                         const Sketch &sketch) {
  return generateChoraleFromPitches(melodyMotif.pitchContour(), 
                                     melodyMotif.rhythmGrid(),
                                     sketch.key());
}

ChoraleVoices PartwritingEngine::generateChoraleFromPitches(const QList<int> &pitches,
                                                             const RhythmGrid &rhythm,
                                                             const QString &key) {
  ChoraleVoices voices;
  
  if (pitches.isEmpty()) {
    qWarning() << "Cannot generate chorale from empty pitch list";
    return voices;
  }
  
  bool isMinor = key.toLower().contains("minor");
  
  // Stage A: Choose chord progression
  QList<DiatonicChord> chords = chooseChordProgression(pitches, isMinor);
  
  qDebug() << "Chord progression:";
  for (int i = 0; i < chords.size(); i++) {
    qDebug() << "  Note" << i << ": melody=" << pitches[i] 
             << "chord=" << chords[i].romanNumeral 
             << chords[i].quality;
  }
  
  // Soprano = melody
  voices.soprano.pitches = pitches;
  voices.soprano.rhythm = rhythm;
  
  if (voices.soprano.rhythm.isEmpty()) {
    for (int i = 0; i < pitches.size(); i++) {
      voices.soprano.rhythm.addCell("quarter");
    }
  }
  
  // Stage B & C: Generate voicings with voice-leading
  QList<int> altoPitches, tenorPitches, bassPitches;
  
  // First voicing
  SATBVoicing prevVoicing = createInitialVoicing(chords[0], pitches[0]);
  altoPitches.append(prevVoicing.alto);
  tenorPitches.append(prevVoicing.tenor);
  bassPitches.append(prevVoicing.bass);
  
  // Subsequent voicings with voice-leading
  for (int i = 1; i < pitches.size(); i++) {
    bool isCadence = isCadencePoint(i, pitches.size(), pitches);
    
    SATBVoicing voicing = chooseVoicing(prevVoicing, chords[i], pitches[i], isCadence);
    
    altoPitches.append(voicing.alto);
    tenorPitches.append(voicing.tenor);
    bassPitches.append(voicing.bass);
    
    prevVoicing = voicing;
  }
  
  // Assign to voice lines
  voices.alto.pitches = altoPitches;
  voices.alto.rhythm = voices.soprano.rhythm;
  
  voices.tenor.pitches = tenorPitches;
  voices.tenor.rhythm = voices.soprano.rhythm;
  
  voices.bass.pitches = bassPitches;
  voices.bass.rhythm = voices.soprano.rhythm;
  
  qDebug() << "Generated SATB chorale with" << pitches.size() << "notes per voice";
  
  return voices;
}
