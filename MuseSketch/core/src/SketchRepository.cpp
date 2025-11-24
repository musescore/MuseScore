#include "SketchRepository.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

SketchRepository::SketchRepository() { ensureDataDirectoryExists(); }

QString SketchRepository::createSketch(const QString &name) {
  Sketch sketch("", name);
  saveSketch(sketch);
  return sketch.id();
}

Sketch SketchRepository::loadSketch(const QString &id) {
  // Check cache first
  if (m_cache.contains(id)) {
    return m_cache[id];
  }

  // Load from disk
  QString filePath = getSketchFilePath(id);
  QFile file(filePath);

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open sketch file:" << filePath;
    return Sketch();
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull() || !doc.isObject()) {
    qWarning() << "Invalid JSON in sketch file:" << filePath;
    return Sketch();
  }

  Sketch sketch = Sketch::fromJson(doc.object());
  m_cache[id] = sketch;

  return sketch;
}

void SketchRepository::saveSketch(const Sketch &sketch) {
  // Update cache
  m_cache[sketch.id()] = sketch;

  // Save to disk
  QString filePath = getSketchFilePath(sketch.id());
  QFile file(filePath);

  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << "Failed to open sketch file for writing:" << filePath;
    return;
  }

  QJsonDocument doc(sketch.toJson());
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
}

QList<SketchInfo> SketchRepository::listSketches() {
  QList<SketchInfo> sketches;

  QString dataDir = getDataDirectory();
  QDir dir(dataDir);

  QStringList filters;
  filters << "*.json";
  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

  for (const QFileInfo &fileInfo : files) {
    QString id = fileInfo.baseName();

    // Try to load minimal info from cache or file
    Sketch sketch;
    if (m_cache.contains(id)) {
      sketch = m_cache[id];
    } else {
      sketch = loadSketch(id);
    }

    if (!sketch.id().isEmpty()) {
      SketchInfo info;
      info.id = sketch.id();
      info.name = sketch.name();
      info.modifiedAt = sketch.modifiedAt();
      sketches.append(info);
    }
  }

  return sketches;
}

void SketchRepository::deleteSketch(const QString &id) {
  // Remove from cache
  m_cache.remove(id);

  // Delete file
  QString filePath = getSketchFilePath(id);
  QFile::remove(filePath);
}

QString SketchRepository::getDataDirectory() {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return dataPath + "/sketches";
}

QString SketchRepository::getSketchFilePath(const QString &id) {
  return getDataDirectory() + "/" + id + ".json";
}

void SketchRepository::ensureDataDirectoryExists() {
  QString dataDir = getDataDirectory();
  QDir dir;
  if (!dir.exists(dataDir)) {
    dir.mkpath(dataDir);
  }
}
