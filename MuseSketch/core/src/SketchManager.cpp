#include "SketchManager.h"
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
