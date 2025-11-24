#include "MotifQuantizer.h"
#include <QDebug>
#include <QtMath>
#include <algorithm>
#include <numeric>

MotifQuantizer::MotifQuantizer() {}

void MotifQuantizer::setConfig(const QuantizerConfig &config) {
  m_config = config;
}

int MotifQuantizer::quantizeToScaleDegree(qreal normalizedY) {
  // normalizedY: 0.0 = top (high pitch), 1.0 = bottom (low pitch)
  // Invert so higher Y = higher scale degree
  qreal invertedY = 1.0 - normalizedY;

  // Map to scale degrees 1-7
  int degree = qRound(invertedY * 6.0) + 1;

  // Clamp to valid range
  return qBound(1, degree, 7);
}

QString MotifQuantizer::beatsToRhythmString(qreal beats) {
  // Map beat duration to rhythm string
  if (beats >= 3.5)
    return "whole";
  if (beats >= 2.5)
    return "dotted-half";
  if (beats >= 1.75)
    return "half";
  if (beats >= 1.25)
    return "dotted-quarter";
  if (beats >= 0.875)
    return "quarter";
  if (beats >= 0.625)
    return "dotted-eighth";
  if (beats >= 0.375)
    return "eighth";
  if (beats >= 0.1875)
    return "sixteenth";
  return "sixteenth";
}

QList<QuantizedNote> MotifQuantizer::quantizeFromVariant(const QVariantList &pathPoints) {
  QList<ContourPoint> points;

  for (const QVariant &v : pathPoints) {
    QVariantMap map = v.toMap();
    ContourPoint pt;
    pt.x = map["x"].toReal();
    pt.y = map["y"].toReal();
    pt.timestamp = map.contains("t") ? map["t"].toReal() : 0.0;
    points.append(pt);
  }

  return quantize(points);
}

QList<QuantizedNote> MotifQuantizer::quantize(const QList<ContourPoint> &points) {
  if (points.isEmpty()) {
    return QList<QuantizedNote>();
  }

  qDebug() << "=== Quantizing" << points.size() << "points ===";

  // Phase 1: Normalize and clean the points
  QList<ContourPoint> cleaned = normalizeAndClean(points);

  if (cleaned.isEmpty()) {
    return QList<QuantizedNote>();
  }

  qDebug() << "After cleaning:" << cleaned.size() << "points";

  // Phase 2: Build pitch grid
  QList<int> pitchGrid = buildPitchGrid(cleaned);

  // Debug: Print pitch grid summary
  QString gridStr;
  for (int i = 0; i < pitchGrid.size(); i++) {
    if (pitchGrid[i] == -1) gridStr += "_";
    else if (pitchGrid[i] == 0) gridStr += "R";
    else gridStr += QString::number(pitchGrid[i]);
  }
  qDebug() << "Pitch grid:" << gridStr;

  // Phase 3: Detect and insert rests from gaps
  detectRests(cleaned, pitchGrid);

  // Phase 4: Convert grid to note events
  QList<QuantizedNote> notes = gridToNotes(pitchGrid);

  qDebug() << "Quantized" << points.size() << "points into" << notes.size()
           << "notes";

  return notes;
}

QList<ContourPoint> MotifQuantizer::normalizeAndClean(const QList<ContourPoint> &points) {
  QList<ContourPoint> cleaned;

  // Remove points that are too close together (noise reduction)
  const qreal minDistance = 0.01; // Minimum distance between points

  for (int i = 0; i < points.size(); i++) {
    if (cleaned.isEmpty()) {
      cleaned.append(points[i]);
      continue;
    }

    const ContourPoint &last = cleaned.last();
    const ContourPoint &current = points[i];

    qreal dx = current.x - last.x;
    qreal dy = current.y - last.y;
    qreal dist = qSqrt(dx * dx + dy * dy);

    if (dist >= minDistance) {
      cleaned.append(current);
    }
  }

  return cleaned;
}

QList<int> MotifQuantizer::buildPitchGrid(const QList<ContourPoint> &points) {
  int totalSlots = m_config.totalSlots();
  qreal gridStep = 1.0 / totalSlots; // Normalized grid step

  // Initialize grid with -1 (empty/no data)
  QList<int> pitchGrid(totalSlots, -1);

  if (points.isEmpty()) {
    return pitchGrid;
  }

  // For each consecutive pair of points, fill all grid slots between them
  // This ensures continuous strokes produce continuous notes
  for (int i = 0; i < points.size(); i++) {
    const ContourPoint &pt = points[i];
    
    // Fill the slot at this point's x position
    int slot = qBound(0, qFloor(pt.x / gridStep), totalSlots - 1);
    pitchGrid[slot] = quantizeToScaleDegree(pt.y);
    
    // If there's a previous point, interpolate between them
    if (i > 0) {
      const ContourPoint &prevPt = points[i - 1];
      int prevSlot = qBound(0, qFloor(prevPt.x / gridStep), totalSlots - 1);
      
      // Fill slots between prevSlot and slot
      if (slot != prevSlot) {
        int startSlot = qMin(prevSlot, slot);
        int endSlot = qMax(prevSlot, slot);
        
        for (int fillSlot = startSlot; fillSlot <= endSlot; fillSlot++) {
          // Linear interpolation of y based on x position
          qreal t = (endSlot == startSlot) ? 0.5 : 
                    qreal(fillSlot - startSlot) / (endSlot - startSlot);
          qreal interpolatedY = prevPt.y + t * (pt.y - prevPt.y);
          pitchGrid[fillSlot] = quantizeToScaleDegree(interpolatedY);
        }
      }
    }
  }

  qDebug() << "Built pitch grid: total slots =" << totalSlots 
           << ", filled =" << std::count_if(pitchGrid.begin(), pitchGrid.end(), 
                                            [](int p) { return p != -1; });

  return pitchGrid;
}

void MotifQuantizer::detectRests(const QList<ContourPoint> &points,
                                 QList<int> &pitchGrid) {
  if (points.size() < 2) {
    return;
  }

  int totalSlots = m_config.totalSlots();
  qreal gridStep = 1.0 / totalSlots;

  // Detect gaps between strokes (significant x-coordinate jumps with time gaps)
  // A rest is when there's BOTH a significant x-gap AND a significant time gap
  for (int i = 1; i < points.size(); i++) {
    qreal dx = points[i].x - points[i - 1].x;
    qreal dt = points[i].timestamp - points[i - 1].timestamp;

    // A rest requires:
    // 1. Significant x-gap (user moved horizontally)
    // 2. Significant time gap (user lifted finger - at least 0.1 seconds)
    bool isSignificantXGap = dx > m_config.restThreshold;
    bool isTimePause = dt > 0.1;  // 100ms pause indicates finger lift

    if (isSignificantXGap && isTimePause) {
      // Find slots in the gap
      int startSlot = qFloor(points[i - 1].x / gridStep) + 1;
      int endSlot = qFloor(points[i].x / gridStep);

      // Mark gap slots as rest (0 = rest)
      for (int slot = startSlot; slot < endSlot && slot < totalSlots; slot++) {
        pitchGrid[slot] = 0; // 0 indicates rest
      }
      
      qDebug() << "Detected rest from slot" << startSlot << "to" << endSlot
               << "(dx=" << dx << ", dt=" << dt << ")";
    }
  }

  // Don't auto-add rests at start/end - let the stroke define the motif boundaries
}

QList<QuantizedNote> MotifQuantizer::gridToNotes(const QList<int> &pitchGrid) {
  QList<QuantizedNote> notes;

  if (pitchGrid.isEmpty()) {
    return notes;
  }

  int totalSlots = pitchGrid.size();
  qreal beatsPerSlot = qreal(m_config.totalBeats()) / totalSlots;

  // Find first filled slot
  int startSlot = 0;
  while (startSlot < totalSlots && pitchGrid[startSlot] == -1) {
    startSlot++;
  }

  if (startSlot >= totalSlots) {
    qDebug() << "gridToNotes: No filled slots found";
    return notes; // No filled slots
  }

  // Find last filled slot
  int endSlot = totalSlots - 1;
  while (endSlot >= 0 && pitchGrid[endSlot] == -1) {
    endSlot--;
  }

  qDebug() << "gridToNotes: Processing slots" << startSlot << "to" << endSlot;

  int currentPitch = pitchGrid[startSlot];
  int currentStartSlot = startSlot;

  for (int slot = startSlot + 1; slot <= endSlot + 1; slot++) {
    int slotPitch;
    if (slot <= endSlot) {
      slotPitch = pitchGrid[slot];
      // Treat unfilled slots (-1) as continuation of current pitch (not rest)
      if (slotPitch == -1) {
        continue;
      }
    } else {
      slotPitch = -999; // Force close at end
    }

    // If pitch changes, close current event
    if (slotPitch != currentPitch) {
      QuantizedNote note;
      note.scaleDegree = (currentPitch == 0) ? 1 : currentPitch;
      note.isRest = (currentPitch == 0);
      note.startBeat = currentStartSlot * beatsPerSlot;
      note.durationBeats = (slot - currentStartSlot) * beatsPerSlot;

      // Only add if duration meets minimum threshold
      if (note.durationBeats >= m_config.minNoteDuration) {
        notes.append(note);
        qDebug() << "  Note:" << (note.isRest ? "REST" : QString::number(note.scaleDegree))
                 << "start=" << note.startBeat << "dur=" << note.durationBeats;
      }

      currentPitch = slotPitch;
      currentStartSlot = slot;
    }
  }

  qDebug() << "gridToNotes: Created" << notes.size() << "notes,"
           << std::count_if(notes.begin(), notes.end(), [](const QuantizedNote &n) { return n.isRest; })
           << "rests";

  return notes;
}

qreal MotifQuantizer::calculateDurationFromSpeed(const QList<ContourPoint> &points,
                                                 int startIdx, int endIdx) {
  if (!m_config.useTimestamps || startIdx >= endIdx || points.isEmpty()) {
    return 1.0; // Default to quarter note
  }

  // Calculate average speed (dx/dt) over the range
  qreal totalDx = 0;
  qreal totalDt = 0;

  for (int i = startIdx + 1; i <= endIdx && i < points.size(); i++) {
    totalDx += qAbs(points[i].x - points[i - 1].x);
    totalDt += points[i].timestamp - points[i - 1].timestamp;
  }

  if (totalDt < 0.001) {
    return 1.0;
  }

  qreal speed = totalDx / totalDt;

  // Map speed to duration:
  // Fast drawing (high speed) = shorter notes
  // Slow drawing (low speed) = longer notes
  // Calibrated so "normal" speed gives quarter notes

  const qreal normalSpeed = 0.5; // Normalized units per second
  qreal speedRatio = normalSpeed / qMax(speed, 0.1);

  // Clamp and quantize
  qreal duration = qBound(0.25, speedRatio, 2.0);

  return duration;
}

void MotifQuantizer::toMotifData(const QList<QuantizedNote> &notes,
                                 QList<int> &pitchContour,
                                 RhythmGrid &rhythmGrid) {
  pitchContour.clear();
  rhythmGrid.clear();

  for (const QuantizedNote &note : notes) {
    pitchContour.append(note.scaleDegree);

    RhythmCell cell;
    cell.duration = beatsToRhythmString(note.durationBeats);
    cell.isRest = note.isRest;
    cell.tie = false;

    rhythmGrid.addCell(cell);
  }
}

