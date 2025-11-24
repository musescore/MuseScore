#include "MotifEditorController.h"
#include "Motif.h"
#include "Sketch.h"
#include <QDebug>

MotifEditorController::MotifEditorController(QObject *parent)
    : QObject(parent), m_key("C major"), m_motifBars(1) {
  // Configure default quantizer settings
  QuantizerConfig config;
  config.beatsPerBar = 4;
  config.barsCount = 1;
  config.subdivisionsPerBeat = 4; // 16th note resolution
  config.minNoteDuration = 0.25;
  config.restThreshold = 0.08;    // 8% of canvas width gap to trigger rest
  config.useTimestamps = true;
  m_quantizer.setConfig(config);
}

void MotifEditorController::startNewMotif(const QString &sketchId) {
  m_currentSketchId = sketchId;

  // Load sketch to get key
  Sketch sketch = m_repository.loadSketch(sketchId);
  if (!sketch.id().isEmpty()) {
    setKey(sketch.key());
  }

  clear();
  emit currentSketchChanged();
}

void MotifEditorController::addNote(int scaleDegree) {
  if (scaleDegree < 1 || scaleDegree > 7) {
    qWarning() << "Invalid scale degree:" << scaleDegree;
    return;
  }

  m_pitchContour.append(scaleDegree);

  // Auto-add default rhythm cell if none specified
  if (m_rhythmGrid.cellCount() < m_pitchContour.size()) {
    m_rhythmGrid.addCell("quarter");
  }

  emit pitchContourChanged();
  emit rhythmPatternChanged();
}

void MotifEditorController::addRhythm(const QString &duration) {
  // Update rhythm for the last note, or add new cell
  if (m_rhythmGrid.cellCount() < m_pitchContour.size()) {
    m_rhythmGrid.addCell(duration);
  } else if (m_rhythmGrid.cellCount() > 0) {
    m_rhythmGrid.setDuration(m_rhythmGrid.cellCount() - 1, duration);
  }

  emit rhythmPatternChanged();
}

void MotifEditorController::removeLastNote() {
  if (!m_pitchContour.isEmpty()) {
    m_pitchContour.removeLast();
    m_rhythmGrid.removeLast();
    emit pitchContourChanged();
    emit rhythmPatternChanged();
  }
}

void MotifEditorController::clear() {
  m_pitchContour.clear();
  m_rhythmGrid.clear();
  emit pitchContourChanged();
  emit rhythmPatternChanged();
}

QString MotifEditorController::saveMotif(const QString &name) {
  if (m_currentSketchId.isEmpty()) {
    qWarning() << "No sketch selected";
    return QString();
  }

  if (m_pitchContour.isEmpty()) {
    qWarning() << "Cannot save empty motif";
    return QString();
  }

  // Load sketch
  Sketch sketch = m_repository.loadSketch(m_currentSketchId);
  if (sketch.id().isEmpty()) {
    qWarning() << "Sketch not found:" << m_currentSketchId;
    return QString();
  }

  // Create motif
  Motif motif("",
              name.isEmpty()
                  ? "Motif " + QString::number(sketch.motifs().size() + 1)
                  : name,
              1);
  motif.setPitchContour(m_pitchContour);
  motif.setRhythmGrid(m_rhythmGrid);
  motif.setKeyRef(m_key);

  // Add to sketch
  sketch.addMotif(motif);
  m_repository.saveSketch(sketch);

  emit motifSaved(motif.id());

  // Clear for next motif
  clear();

  return motif.id();
}

QVariantList MotifEditorController::pitchContour() const {
  QVariantList list;
  for (int degree : m_pitchContour) {
    list.append(degree);
  }
  return list;
}

QVariantList MotifEditorController::rhythmPattern() const {
  QVariantList list;
  for (const QString &rhythm : m_rhythmGrid.toDurationList()) {
    list.append(rhythm);
  }
  return list;
}

QVariantList MotifEditorController::rhythmCells() const {
  QVariantList list;
  for (const RhythmCell &cell : m_rhythmGrid.cells()) {
    QVariantMap cellMap;
    cellMap["duration"] = cell.duration;
    cellMap["tie"] = cell.tie;
    cellMap["isRest"] = cell.isRest;
    list.append(cellMap);
  }
  return list;
}

void MotifEditorController::setKey(const QString &key) {
  if (m_key != key) {
    m_key = key;
    emit keyChanged();
  }
}

// RhythmGrid functions

void MotifEditorController::setRhythmDuration(int index,
                                              const QString &duration) {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    m_rhythmGrid.setDuration(index, duration);
    emit rhythmPatternChanged();
  }
}

void MotifEditorController::toggleRest(int index) {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    m_rhythmGrid.toggleRest(index);
    emit rhythmPatternChanged();
  }
}

void MotifEditorController::toggleTie(int index) {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    m_rhythmGrid.toggleTie(index);
    emit rhythmPatternChanged();
  }
}

bool MotifEditorController::isRest(int index) const {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    return m_rhythmGrid.cell(index).isRest;
  }
  return false;
}

bool MotifEditorController::isTied(int index) const {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    return m_rhythmGrid.cell(index).tie;
  }
  return false;
}

QString MotifEditorController::getDuration(int index) const {
  if (index >= 0 && index < m_rhythmGrid.cellCount()) {
    return m_rhythmGrid.cell(index).duration;
  }
  return "quarter";
}

// Configuration functions

void MotifEditorController::setMotifBars(int bars) {
  if (bars >= 1 && bars <= 4 && bars != m_motifBars) {
    m_motifBars = bars;

    // Update quantizer config
    QuantizerConfig config = m_quantizer.config();
    config.barsCount = bars;
    m_quantizer.setConfig(config);

    emit motifBarsChanged();
  }
}

void MotifEditorController::setSubdivision(int subdivisionsPerBeat) {
  QuantizerConfig config = m_quantizer.config();
  config.subdivisionsPerBeat = qBound(1, subdivisionsPerBeat, 8);
  m_quantizer.setConfig(config);
}

// Pitch contour helper functions

int MotifEditorController::quantizeToScaleDegree(qreal normalizedY) {
  return m_quantizer.quantizeToScaleDegree(normalizedY);
}

void MotifEditorController::applyContourPath(const QVariantList &pathPoints) {
  // Clear existing contour
  clear();

  if (pathPoints.isEmpty()) {
    return;
  }

  // Use the advanced quantizer to process the path
  QList<QuantizedNote> notes = m_quantizer.quantizeFromVariant(pathPoints);

  if (notes.isEmpty()) {
    qDebug() << "No notes quantized from" << pathPoints.size() << "points";
    return;
  }

  // Convert quantized notes to our internal format
  // Only add to pitchContour for actual notes, not rests
  for (const QuantizedNote &note : notes) {
    if (!note.isRest) {
      m_pitchContour.append(note.scaleDegree);
    }

    RhythmCell cell;
    cell.duration = MotifQuantizer::beatsToRhythmString(note.durationBeats);
    cell.isRest = note.isRest;
    cell.tie = false;

    m_rhythmGrid.addCell(cell);
  }

  emit pitchContourChanged();
  emit rhythmPatternChanged();

  int noteCount = std::count_if(notes.begin(), notes.end(), 
                                [](const QuantizedNote &n) { return !n.isRest; });
  int restCount = notes.size() - noteCount;
  qDebug() << "Applied contour path:" << pathPoints.size() << "points ->"
           << noteCount << "notes," << restCount << "rests";
}
