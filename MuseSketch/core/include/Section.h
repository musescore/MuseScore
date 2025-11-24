#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

struct MotifPlacement {
  QString motifId;
  int startBar;
  int voice; // 0=soprano, 1=alto, 2=tenor, 3=bass

  QJsonObject toJson() const;
  static MotifPlacement fromJson(const QJsonObject &json);
};

class Section {
public:
  Section() = default;
  Section(const QString &id, const QString &name, int lengthBars);

  QString id() const { return m_id; }
  QString name() const { return m_name; }
  int lengthBars() const { return m_lengthBars; }
  QList<MotifPlacement> placements() const { return m_placements; }

  void setName(const QString &name) { m_name = name; }
  void setLengthBars(int length) { m_lengthBars = length; }
  void addPlacement(const MotifPlacement &placement);
  void removePlacement(const QString &motifId);

  QJsonObject toJson() const;
  static Section fromJson(const QJsonObject &json);

private:
  QString m_id;
  QString m_name;
  int m_lengthBars = 4;
  QList<MotifPlacement> m_placements;
};
