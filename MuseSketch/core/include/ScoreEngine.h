#pragma once

#include <QImage>
#include <QString>

class ScoreEngine {
public:
  ScoreEngine();
  ~ScoreEngine();

  bool loadScore(const QString &filePath);
  QImage renderPage(int pageIndex);

private:
  // TODO: Add actual MuseScore score member when implementing real integration
};
