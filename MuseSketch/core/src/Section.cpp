#include "Section.h"
#include <QJsonArray>
#include <QUuid>

Section::Section(const QString &id, const QString &name, int lengthBars)
    : m_id(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                        : id),
      m_name(name), m_lengthBars(lengthBars) {}

void Section::addPlacement(const MotifPlacement &placement) {
  m_placements.append(placement);
}

void Section::removePlacement(const QString &motifId) {
  m_placements.erase(std::remove_if(m_placements.begin(), m_placements.end(),
                                    [&motifId](const MotifPlacement &p) {
                                      return p.motifId == motifId;
                                    }),
                     m_placements.end());
}

QJsonObject MotifPlacement::toJson() const {
  QJsonObject json;
  json["motifId"] = motifId;
  json["startBar"] = startBar;
  json["voice"] = voice;
  return json;
}

MotifPlacement MotifPlacement::fromJson(const QJsonObject &json) {
  MotifPlacement placement;
  placement.motifId = json["motifId"].toString();
  placement.startBar = json["startBar"].toInt();
  placement.voice = json["voice"].toInt();
  return placement;
}

QJsonObject Section::toJson() const {
  QJsonObject json;
  json["id"] = m_id;
  json["name"] = m_name;
  json["lengthBars"] = m_lengthBars;

  QJsonArray placementsArray;
  for (const auto &placement : m_placements) {
    placementsArray.append(placement.toJson());
  }
  json["placements"] = placementsArray;

  return json;
}

Section Section::fromJson(const QJsonObject &json) {
  Section section(json["id"].toString(), json["name"].toString(),
                  json["lengthBars"].toInt(4));

  QJsonArray placementsArray = json["placements"].toArray();
  for (const auto &placementValue : placementsArray) {
    section.addPlacement(MotifPlacement::fromJson(placementValue.toObject()));
  }

  return section;
}
