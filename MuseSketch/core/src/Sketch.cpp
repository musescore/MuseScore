#include "Sketch.h"
#include <QJsonArray>
#include <QUuid>

Sketch::Sketch(const QString &id, const QString &name)
    : m_id(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                        : id),
      m_name(name), m_createdAt(QDateTime::currentDateTime()),
      m_modifiedAt(QDateTime::currentDateTime()) {}

void Sketch::setName(const QString &name) {
  m_name = name;
  touch();
}

void Sketch::addMotif(const Motif &motif) {
  m_motifs.append(motif);
  touch();
}

void Sketch::removeMotif(const QString &motifId) {
  m_motifs.erase(
      std::remove_if(m_motifs.begin(), m_motifs.end(),
                     [&motifId](const Motif &m) { return m.id() == motifId; }),
      m_motifs.end());
  touch();
}

Motif *Sketch::findMotif(const QString &motifId) {
  for (auto &motif : m_motifs) {
    if (motif.id() == motifId) {
      return &motif;
    }
  }
  return nullptr;
}

void Sketch::addSection(const Section &section) {
  m_sections.append(section);
  touch();
}

void Sketch::removeSection(const QString &sectionId) {
  m_sections.erase(std::remove_if(m_sections.begin(), m_sections.end(),
                                  [&sectionId](const Section &s) {
                                    return s.id() == sectionId;
                                  }),
                   m_sections.end());
  touch();
}

Section *Sketch::findSection(const QString &sectionId) {
  for (auto &section : m_sections) {
    if (section.id() == sectionId) {
      return &section;
    }
  }
  return nullptr;
}

void Sketch::touch() { m_modifiedAt = QDateTime::currentDateTime(); }

QJsonObject Sketch::toJson() const {
  QJsonObject json;
  json["id"] = m_id;
  json["name"] = m_name;
  json["key"] = m_key;
  json["tempo"] = m_tempo;
  json["timeSignature"] = m_timeSignature;
  json["createdAt"] = m_createdAt.toString(Qt::ISODate);
  json["modifiedAt"] = m_modifiedAt.toString(Qt::ISODate);

  QJsonArray motifsArray;
  for (const auto &motif : m_motifs) {
    motifsArray.append(motif.toJson());
  }
  json["motifs"] = motifsArray;

  QJsonArray sectionsArray;
  for (const auto &section : m_sections) {
    sectionsArray.append(section.toJson());
  }
  json["sections"] = sectionsArray;

  return json;
}

Sketch Sketch::fromJson(const QJsonObject &json) {
  Sketch sketch(json["id"].toString(), json["name"].toString());

  sketch.setKey(json["key"].toString("C major"));
  sketch.setTempo(json["tempo"].toInt(120));
  sketch.setTimeSignature(json["timeSignature"].toString("4/4"));
  sketch.m_createdAt =
      QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
  sketch.m_modifiedAt =
      QDateTime::fromString(json["modifiedAt"].toString(), Qt::ISODate);

  QJsonArray motifsArray = json["motifs"].toArray();
  for (const auto &motifValue : motifsArray) {
    sketch.m_motifs.append(Motif::fromJson(motifValue.toObject()));
  }

  QJsonArray sectionsArray = json["sections"].toArray();
  for (const auto &sectionValue : sectionsArray) {
    sketch.m_sections.append(Section::fromJson(sectionValue.toObject()));
  }

  return sketch;
}
