#pragma once

#include "SketchRepository.h"
#include <QObject>
#include <QVariantList>

class SketchManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantList sketches READ sketches NOTIFY sketchesChanged)

public:
  explicit SketchManager(QObject *parent = nullptr);

  Q_INVOKABLE QString createNewSketch(const QString &name);
  Q_INVOKABLE void openSketch(const QString &id);
  Q_INVOKABLE void deleteSketch(const QString &id);
  Q_INVOKABLE void refreshSketches();
  Q_INVOKABLE QVariantMap getSketch(const QString &id);
  Q_INVOKABLE QVariantMap getMotif(const QString &sketchId, const QString &motifId);
  Q_INVOKABLE void updateMotifPitches(const QString &sketchId, const QString &motifId, const QVariantList &pitches);
  
  // Motif CRUD operations (PR-06)
  Q_INVOKABLE void renameMotif(const QString &sketchId, const QString &motifId, const QString &newName);
  Q_INVOKABLE QString duplicateMotif(const QString &sketchId, const QString &motifId);
  Q_INVOKABLE void deleteMotif(const QString &sketchId, const QString &motifId);

  QVariantList sketches() const;

signals:
  void sketchesChanged();
  void sketchOpened(const QString &id);

private:
  void loadSketches();

  SketchRepository m_repository;
  QVariantList m_sketches;
};
