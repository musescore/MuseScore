#pragma once

#include "RhythmGrid.h"
#include <QList>
#include <QString>
#include <QVariantList>

// Represents a single point from the contour canvas
struct ContourPoint {
  qreal x;         // Normalized x position [0.0, 1.0] = time
  qreal y;         // Normalized y position [0.0, 1.0] = pitch
  qreal timestamp; // Time since stroke start in seconds

  ContourPoint() : x(0), y(0), timestamp(0) {}
  ContourPoint(qreal px, qreal py, qreal t = 0) : x(px), y(py), timestamp(t) {}
};

// Represents a quantized note event
struct QuantizedNote {
  int scaleDegree;    // 1-7 (0 = rest)
  qreal startBeat;    // Start position in beats
  qreal durationBeats; // Duration in beats
  bool isRest;        // True if this is a rest

  QuantizedNote()
      : scaleDegree(1), startBeat(0), durationBeats(1), isRest(false) {}
};

// Configuration for the quantizer
struct QuantizerConfig {
  int beatsPerBar = 4;          // Time signature numerator
  int barsCount = 1;            // Number of bars in motif
  int subdivisionsPerBeat = 4;  // Grid resolution (4 = 16th notes)
  qreal minNoteDuration = 0.25; // Minimum note duration in beats
  qreal restThreshold = 0.15;   // Gap in x-space to trigger rest (normalized)
  qreal leapThreshold = 0.25;   // Y-delta threshold for leap detection
  bool useTimestamps = true;    // Use timestamps for rhythm detection

  int totalBeats() const { return beatsPerBar * barsCount; }
  qreal gridStep() const { return 1.0 / subdivisionsPerBeat; }
  int totalSlots() const { return totalBeats() * subdivisionsPerBeat; }
};

class MotifQuantizer {
public:
  MotifQuantizer();

  // Configure the quantizer
  void setConfig(const QuantizerConfig &config);
  QuantizerConfig config() const { return m_config; }

  // Main quantization function - converts raw path points to notes
  QList<QuantizedNote> quantize(const QList<ContourPoint> &points);

  // Convenience function for QML - takes QVariantList
  QList<QuantizedNote> quantizeFromVariant(const QVariantList &pathPoints);

  // Convert quantized notes to pitch contour and rhythm grid
  void toMotifData(const QList<QuantizedNote> &notes, QList<int> &pitchContour,
                   RhythmGrid &rhythmGrid);

  // Quantize y-coordinate to scale degree (1-7)
  int quantizeToScaleDegree(qreal normalizedY);

  // Convert duration in beats to rhythm string
  static QString beatsToRhythmString(qreal beats);

private:
  QuantizerConfig m_config;

  // Internal processing stages
  QList<ContourPoint> normalizeAndClean(const QList<ContourPoint> &points);
  QList<int> buildPitchGrid(const QList<ContourPoint> &points);
  QList<QuantizedNote> gridToNotes(const QList<int> &pitchGrid);

  // Detect rests from gaps in the stroke
  void detectRests(const QList<ContourPoint> &points, QList<int> &pitchGrid);

  // Calculate note duration based on drag speed
  qreal calculateDurationFromSpeed(const QList<ContourPoint> &points, int startIdx,
                                  int endIdx);
};

