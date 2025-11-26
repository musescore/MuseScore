#pragma once

#include "SketchRepository.h"
#include <QObject>
#include <QVariantList>

class PartwritingEngine;

class SketchManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantList sketches READ sketches NOTIFY sketchesChanged)

public:
  explicit SketchManager(QObject *parent = nullptr);
  ~SketchManager();

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
  
  // Section CRUD operations (PR-07)
  Q_INVOKABLE QString createSection(const QString &sketchId, const QString &name, int lengthBars = 8);
  Q_INVOKABLE QVariantMap getSection(const QString &sketchId, const QString &sectionId);
  Q_INVOKABLE QVariantList getSections(const QString &sketchId);
  Q_INVOKABLE void renameSection(const QString &sketchId, const QString &sectionId, const QString &newName);
  Q_INVOKABLE void setSectionLength(const QString &sketchId, const QString &sectionId, int lengthBars);
  Q_INVOKABLE void deleteSection(const QString &sketchId, const QString &sectionId);
  
  // Section placement operations
  Q_INVOKABLE void addPlacement(const QString &sketchId, const QString &sectionId, 
                                 const QString &motifId, int startBar, int repetitions = 1);
  Q_INVOKABLE void removePlacement(const QString &sketchId, const QString &sectionId, int placementIndex);
  Q_INVOKABLE void movePlacement(const QString &sketchId, const QString &sectionId, 
                                  int placementIndex, int newStartBar);
  Q_INVOKABLE void setPlacementRepetitions(const QString &sketchId, const QString &sectionId,
                                            int placementIndex, int repetitions);
  
  // Get flattened timeline for playback
  Q_INVOKABLE QVariantList getSectionTimeline(const QString &sketchId, const QString &sectionId);
  
  // Texture/SATB operations (PR-08)
  Q_INVOKABLE void setSectionTexture(const QString &sketchId, const QString &sectionId, 
                                      const QString &textureType);
  Q_INVOKABLE QString getSectionTexture(const QString &sketchId, const QString &sectionId);
  Q_INVOKABLE void generateSATBChorale(const QString &sketchId, const QString &sectionId,
                                        const QString &sopranoMotifId);
  Q_INVOKABLE void generateSATBFromTimeline(const QString &sketchId, const QString &sectionId);
  Q_INVOKABLE QVariantList getSATBVoiceTimeline(const QString &sketchId, const QString &sectionId,
                                                 int voiceIndex);
  Q_INVOKABLE bool hasSATBVoices(const QString &sketchId, const QString &sectionId);

  QVariantList sketches() const;

signals:
  void sketchesChanged();
  void sketchOpened(const QString &id);

private:
  void loadSketches();

  SketchRepository m_repository;
  QVariantList m_sketches;
  PartwritingEngine *m_partwritingEngine;
};
