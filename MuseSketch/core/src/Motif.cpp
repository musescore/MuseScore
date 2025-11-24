#include "Motif.h"
#include <QJsonArray>
#include <QUuid>

Motif::Motif(const QString &id, const QString &name, int lengthBars)
    : m_id(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                        : id),
      m_name(name), m_lengthBars(lengthBars) {}

QJsonObject Motif::toJson() const {
  QJsonObject json;
  json["id"] = m_id;
  json["name"] = m_name;
  json["lengthBars"] = m_lengthBars;
  json["keyRef"] = m_keyRef;

  QJsonArray contourArray;
  for (int degree : m_pitchContour) {
    contourArray.append(degree);
  }
  json["pitchContour"] = contourArray;

  QJsonArray rhythmArray;
  for (const QString &rhythm : m_rhythmPattern) {
    rhythmArray.append(rhythm);
  }
  json["rhythmPattern"] = rhythmArray;

  return json;
}

Motif Motif::fromJson(const QJsonObject &json) {
  Motif motif(json["id"].toString(), json["name"].toString(),
              json["lengthBars"].toInt(1));

  motif.setKeyRef(json["keyRef"].toString());

  QJsonArray contourArray = json["pitchContour"].toArray();
  QList<int> contour;
  for (const auto &value : contourArray) {
    contour.append(value.toInt());
  }
  motif.setPitchContour(contour);

  QJsonArray rhythmArray = json["rhythmPattern"].toArray();
  QList<QString> rhythm;
  for (const auto &value : rhythmArray) {
    rhythm.append(value.toString());
  }
  motif.setRhythmPattern(rhythm);

  return motif;
}
