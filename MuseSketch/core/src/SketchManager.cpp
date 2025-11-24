#include "SketchManager.h"
#include "Motif.h"
#include "RhythmGrid.h"
#include "Sketch.h"
#include <QVariantMap>

SketchManager::SketchManager(QObject *parent) : QObject(parent) {
  loadSketches();
}

QString SketchManager::createNewSketch(const QString &name) {
  QString id = m_repository.createSketch(name);
  loadSketches();
  return id;
}

void SketchManager::openSketch(const QString &id) { emit sketchOpened(id); }

void SketchManager::deleteSketch(const QString &id) {
  m_repository.deleteSketch(id);
  loadSketches();
}

void SketchManager::refreshSketches() { loadSketches(); }

QVariantMap SketchManager::getSketch(const QString &id) {
  Sketch sketch = m_repository.loadSketch(id);

  QVariantMap sketchMap;
  sketchMap["id"] = sketch.id();
  sketchMap["name"] = sketch.name();
  sketchMap["key"] = sketch.key();
  sketchMap["tempo"] = sketch.tempo();
  sketchMap["timeSignature"] = sketch.timeSignature();

  // Convert motifs to QVariantList
  QVariantList motifsArray;
  for (const Motif &motif : sketch.motifs()) {
    QVariantMap motifMap;
    motifMap["id"] = motif.id();
    motifMap["name"] = motif.name();
    motifMap["lengthBars"] = motif.lengthBars();

    // Convert pitch contour
    QVariantList pitchArray;
    for (int degree : motif.pitchContour()) {
      pitchArray.append(degree);
    }
    motifMap["pitchContour"] = pitchArray;

    // Convert rhythm pattern (legacy format for compatibility)
    QVariantList rhythmArray;
    for (const QString &rhythm : motif.rhythmPattern()) {
      rhythmArray.append(rhythm);
    }
    motifMap["rhythmPattern"] = rhythmArray;

    // Convert rhythm cells (new format with tie/rest info)
    QVariantList rhythmCells;
    for (const RhythmCell &cell : motif.rhythmGrid().cells()) {
      QVariantMap cellMap;
      cellMap["duration"] = cell.duration;
      cellMap["tie"] = cell.tie;
      cellMap["isRest"] = cell.isRest;
      rhythmCells.append(cellMap);
    }
    motifMap["rhythmCells"] = rhythmCells;

    motifsArray.append(motifMap);
  }
  sketchMap["motifs"] = motifsArray;

  return sketchMap;
}

QVariantList SketchManager::sketches() const { return m_sketches; }

void SketchManager::loadSketches() {
  QList<SketchInfo> sketchInfos = m_repository.listSketches();

  m_sketches.clear();
  for (const SketchInfo &info : sketchInfos) {
    QVariantMap sketchMap;
    sketchMap["id"] = info.id;
    sketchMap["name"] = info.name;
    sketchMap["modifiedAt"] = info.modifiedAt.toString("yyyy-MM-dd hh:mm");
    m_sketches.append(sketchMap);
  }

  emit sketchesChanged();
}
