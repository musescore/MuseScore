#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

class Motif {
public:
  Motif() = default;
  Motif(const QString &id, const QString &name, int lengthBars);

  QString id() const { return m_id; }
  QString name() const { return m_name; }
  int lengthBars() const { return m_lengthBars; }
  QList<int> pitchContour() const { return m_pitchContour; }
  QList<QString> rhythmPattern() const { return m_rhythmPattern; }
  QString keyRef() const { return m_keyRef; }

  void setName(const QString &name) { m_name = name; }
  void setLengthBars(int length) { m_lengthBars = length; }
  void setPitchContour(const QList<int> &contour) { m_pitchContour = contour; }
  void setRhythmPattern(const QList<QString> &pattern) {
    m_rhythmPattern = pattern;
  }
  void setKeyRef(const QString &key) { m_keyRef = key; }

  QJsonObject toJson() const;
  static Motif fromJson(const QJsonObject &json);

private:
  QString m_id;
  QString m_name;
  int m_lengthBars = 1;
  QList<int> m_pitchContour;      // Scale degrees (1-7)
  QList<QString> m_rhythmPattern; // e.g., ["quarter", "eighth", "eighth"]
  QString m_keyRef;               // Reference to parent sketch key
};
