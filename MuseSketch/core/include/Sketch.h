#pragma once

#include "Motif.h"
#include "Section.h"
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

class Sketch {
public:
  Sketch() = default;
  Sketch(const QString &id, const QString &name);

  QString id() const { return m_id; }
  QString name() const { return m_name; }
  QString key() const { return m_key; }
  int tempo() const { return m_tempo; }
  QString timeSignature() const { return m_timeSignature; }
  QList<Motif> motifs() const { return m_motifs; }
  QList<Section> sections() const { return m_sections; }
  QDateTime createdAt() const { return m_createdAt; }
  QDateTime modifiedAt() const { return m_modifiedAt; }

  void setName(const QString &name);
  void setKey(const QString &key) { m_key = key; }
  void setTempo(int tempo) { m_tempo = tempo; }
  void setTimeSignature(const QString &timeSig) { m_timeSignature = timeSig; }

  void addMotif(const Motif &motif);
  void removeMotif(const QString &motifId);
  Motif *findMotif(const QString &motifId);

  void addSection(const Section &section);
  void removeSection(const QString &sectionId);
  Section *findSection(const QString &sectionId);

  void touch(); // Update modifiedAt timestamp

  QJsonObject toJson() const;
  static Sketch fromJson(const QJsonObject &json);

private:
  QString m_id;
  QString m_name;
  QString m_key = "C major";
  int m_tempo = 120;
  QString m_timeSignature = "4/4";
  QList<Motif> m_motifs;
  QList<Section> m_sections;
  QDateTime m_createdAt;
  QDateTime m_modifiedAt;
};
