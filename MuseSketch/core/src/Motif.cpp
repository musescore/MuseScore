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

  // Use new RhythmGrid format
  json["rhythmGrid"] = m_rhythmGrid.toJson();

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

  // Support both new rhythmGrid format and legacy rhythmPattern format
  if (json.contains("rhythmGrid")) {
    motif.setRhythmGrid(RhythmGrid::fromJson(json["rhythmGrid"].toArray()));
  } else if (json.contains("rhythmPattern")) {
    // Legacy format - convert from string array
    QJsonArray rhythmArray = json["rhythmPattern"].toArray();
    QList<QString> rhythm;
    for (const auto &value : rhythmArray) {
      rhythm.append(value.toString());
    }
    motif.setRhythmPattern(rhythm);
  }

  return motif;
}
