#pragma once

#include "MotifQuantizer.h"
#include "RhythmGrid.h"
#include "SketchRepository.h"
#include <QObject>
#include <QString>
#include <QVariantList>

class MotifEditorController : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      QString currentSketchId READ currentSketchId NOTIFY currentSketchChanged)
  Q_PROPERTY(
      QVariantList pitchContour READ pitchContour NOTIFY pitchContourChanged)
  Q_PROPERTY(
      QVariantList rhythmPattern READ rhythmPattern NOTIFY rhythmPatternChanged)
  Q_PROPERTY(
      QVariantList rhythmCells READ rhythmCells NOTIFY rhythmPatternChanged)
  Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
  Q_PROPERTY(int noteCount READ noteCount NOTIFY pitchContourChanged)
  Q_PROPERTY(int motifBars READ motifBars WRITE setMotifBars NOTIFY
                 motifBarsChanged)

public:
  explicit MotifEditorController(QObject *parent = nullptr);

  Q_INVOKABLE void startNewMotif(const QString &sketchId);
  Q_INVOKABLE void addNote(int scaleDegree);
  Q_INVOKABLE void addRhythm(const QString &duration);
  Q_INVOKABLE void removeLastNote();
  Q_INVOKABLE void clear();
  Q_INVOKABLE QString saveMotif(const QString &name);

  // Contour path quantization (enhanced with time-based rhythm)
  Q_INVOKABLE void applyContourPath(const QVariantList &pathPoints);
  Q_INVOKABLE int quantizeToScaleDegree(qreal normalizedY);

  // RhythmGrid functions
  Q_INVOKABLE void setRhythmDuration(int index, const QString &duration);
  Q_INVOKABLE void toggleRest(int index);
  Q_INVOKABLE void toggleTie(int index);
  Q_INVOKABLE bool isRest(int index) const;
  Q_INVOKABLE bool isTied(int index) const;
  Q_INVOKABLE QString getDuration(int index) const;

  // Quantizer configuration
  Q_INVOKABLE void setSubdivision(int subdivisionsPerBeat);

  QString currentSketchId() const { return m_currentSketchId; }
  QVariantList pitchContour() const;
  QVariantList rhythmPattern() const;
  QVariantList rhythmCells() const;
  QString key() const { return m_key; }
  int noteCount() const { return m_pitchContour.size(); }
  int motifBars() const { return m_motifBars; }

  void setKey(const QString &key);
  void setMotifBars(int bars);

signals:
  void currentSketchChanged();
  void pitchContourChanged();
  void rhythmPatternChanged();
  void keyChanged();
  void motifBarsChanged();
  void motifSaved(const QString &motifId);

private:
  SketchRepository m_repository;
  MotifQuantizer m_quantizer;
  QString m_currentSketchId;
  QString m_key;
  int m_motifBars = 1;
  QList<int> m_pitchContour;
  RhythmGrid m_rhythmGrid;
};
