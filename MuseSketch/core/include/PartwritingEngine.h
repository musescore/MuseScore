#pragma once

#include "Motif.h"
#include "RhythmGrid.h"
#include <QList>
#include <QString>

class Sketch;

// Represents a single voice line (pitch sequence with rhythm)
struct VoiceLine {
  QList<int> pitches;      // Scale degrees (1-7) or 0 for rest
  RhythmGrid rhythm;       // Rhythm pattern matching the pitches
  QString name;            // "Soprano", "Alto", "Tenor", "Bass"
  
  bool isEmpty() const { return pitches.isEmpty(); }
  int noteCount() const { return pitches.size(); }
};

// Holds all four SATB voices
struct ChoraleVoices {
  VoiceLine soprano;
  VoiceLine alto;
  VoiceLine tenor;
  VoiceLine bass;
  
  ChoraleVoices() {
    soprano.name = "Soprano";
    alto.name = "Alto";
    tenor.name = "Tenor";
    bass.name = "Bass";
  }
  
  bool isValid() const {
    return !soprano.isEmpty() && !alto.isEmpty() && 
           !tenor.isEmpty() && !bass.isEmpty();
  }
};

// Harmonic function categories
enum class HarmonicFunction {
  Tonic,        // I, vi
  Predominant,  // ii, IV
  Dominant      // V, vii°
};

// Chord representation for harmonization
struct DiatonicChord {
  int root;                 // Scale degree (1-7)
  int third;                // Scale degree
  int fifth;                // Scale degree
  QString quality;          // "major", "minor", "diminished"
  HarmonicFunction function;
  int romanNumeral;         // 1-7 for I-vii
  
  // Get all chord tones
  QList<int> tones() const { return {root, third, fifth}; }
  
  // Check if a scale degree is in this chord
  bool contains(int scaleDegree) const {
    int normalized = ((scaleDegree - 1) % 7) + 1;
    return normalized == root || normalized == third || normalized == fifth;
  }
};

// SATB voicing for a single chord
struct SATBVoicing {
  int soprano;  // Scale degrees (can be negative or >7 for octave displacement)
  int alto;
  int tenor;
  int bass;
  
  bool isValid() const {
    return soprano > 0 && alto > 0 && tenor > 0 && bass > 0;
  }
};

class PartwritingEngine {
public:
  PartwritingEngine();
  
  // Main API: Generate SATB chorale from a melody motif
  ChoraleVoices generateChoraleTexture(const Motif &melodyMotif, 
                                        const Sketch &sketch);
  
  // Generate SATB from pitch/rhythm data directly (for timeline-based generation)
  ChoraleVoices generateChoraleFromPitches(const QList<int> &pitches,
                                            const RhythmGrid &rhythm,
                                            const QString &key);
  
  // Get the diatonic chord built on a scale degree
  static DiatonicChord getDiatonicChord(int scaleDegree);

private:
  // =========== Stage A: Harmonic Skeleton ===========
  
  // Get candidate chords that can harmonize a melody note
  QList<DiatonicChord> getCandidateChords(int melodyScaleDegree);
  
  // Choose best chord progression using functional harmony scoring
  QList<DiatonicChord> chooseChordProgression(const QList<int> &melodyPitches,
                                               bool isMinorKey);
  
  // Score a chord transition (higher = better)
  int scoreChordTransition(const DiatonicChord &prev, const DiatonicChord &curr,
                           int melodyPitch, bool isLastNote, bool isPenultimate);
  
  // =========== Stage B: Initial Voicings ===========
  
  // Generate initial voicing for the first chord
  SATBVoicing createInitialVoicing(const DiatonicChord &chord, int sopranoPitch);
  
  // Generate candidate voicings for a chord given soprano
  QList<SATBVoicing> generateCandidateVoicings(const DiatonicChord &chord, 
                                                 int sopranoPitch);
  
  // =========== Stage C: Voice-Leading ===========
  
  // Choose best voicing considering voice-leading from previous
  SATBVoicing chooseVoicing(const SATBVoicing &prevVoicing,
                            const DiatonicChord &chord,
                            int sopranoPitch,
                            bool isCadence);
  
  // Score a voicing (higher = better)
  int scoreVoicing(const SATBVoicing &prev, const SATBVoicing &curr,
                   const DiatonicChord &chord);
  
  // Check for parallel perfect intervals (fifths/octaves)
  bool hasParallelPerfects(const SATBVoicing &prev, const SATBVoicing &curr);
  
  // Check for voice crossing
  bool hasVoiceCrossing(const SATBVoicing &voicing);
  
  // Check spacing rules (S-A, A-T <= octave)
  bool hasProperSpacing(const SATBVoicing &voicing);
  
  // Count common tones between voicings
  int countCommonTones(const SATBVoicing &prev, const SATBVoicing &curr);
  
  // Calculate total voice movement (lower = smoother)
  int calculateVoiceMovement(const SATBVoicing &prev, const SATBVoicing &curr);
  
  // =========== Stage D: Cadences ===========
  
  // Detect if position is a cadence point
  bool isCadencePoint(int noteIndex, int totalNotes, const QList<int> &melodyPitches);
  
  // Apply cadence voicing rules
  SATBVoicing applyCadenceVoicing(const DiatonicChord &chord, int sopranoPitch,
                                   bool isPenultimate);
  
  // =========== Utilities ===========
  
  // Normalize scale degree to 1-7
  static int normalize(int degree) {
    int normalized = ((degree - 1) % 7) + 1;
    return normalized <= 0 ? normalized + 7 : normalized;
  }
  
  // Calculate interval between two scale degrees
  static int interval(int from, int to);
  
  // Check if interval is a perfect fifth or octave
  static bool isPerfectInterval(int interval);
  
  // Voice ranges (as semitones from middle C = 0)
  // Based on PARTWRITING_ENGINE.md specifications
  static constexpr int SOPRANO_LOW = 0;   // C4
  static constexpr int SOPRANO_HIGH = 19; // G5
  static constexpr int ALTO_LOW = -5;     // G3
  static constexpr int ALTO_HIGH = 14;    // D5
  static constexpr int TENOR_LOW = -12;   // C3
  static constexpr int TENOR_HIGH = 7;    // G4
  static constexpr int BASS_LOW = -20;    // E2
  static constexpr int BASS_HIGH = 0;     // C4
};
