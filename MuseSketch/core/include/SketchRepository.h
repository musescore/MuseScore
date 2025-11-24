#pragma once

#include "Sketch.h"
#include <QList>
#include <QMap>
#include <QString>

struct SketchInfo {
  QString id;
  QString name;
  QDateTime modifiedAt;
};

class SketchRepository {
public:
  SketchRepository();

  QString createSketch(const QString &name);
  Sketch loadSketch(const QString &id);
  void saveSketch(const Sketch &sketch);
  QList<SketchInfo> listSketches();
  void deleteSketch(const QString &id);

private:
  QString getDataDirectory();
  QString getSketchFilePath(const QString &id);
  void ensureDataDirectoryExists();

  QMap<QString, Sketch> m_cache;
};
