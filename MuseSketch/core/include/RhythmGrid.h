#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>

// Represents a single rhythmic cell in the grid
struct RhythmCell {
  QString duration = "quarter"; // quarter, eighth, half, whole, sixteenth, etc.
  bool tie = false;             // Tied to the next note
  bool isRest = false;          // Rest instead of a sounding note

  RhythmCell() = default;
  RhythmCell(const QString &dur, bool t = false, bool rest = false)
      : duration(dur), tie(t), isRest(rest) {}

  bool operator==(const RhythmCell &other) const {
    return duration == other.duration && tie == other.tie &&
           isRest == other.isRest;
  }

  QJsonObject toJson() const {
    QJsonObject json;
    json["duration"] = duration;
    json["tie"] = tie;
    json["isRest"] = isRest;
    return json;
  }

  static RhythmCell fromJson(const QJsonObject &json) {
    RhythmCell cell;
    cell.duration = json["duration"].toString("quarter");
    cell.tie = json["tie"].toBool(false);
    cell.isRest = json["isRest"].toBool(false);
    return cell;
  }
};

// Represents a grid of rhythmic cells for a motif
class RhythmGrid {
public:
  RhythmGrid() = default;

  // Cell management
  void addCell(const RhythmCell &cell);
  void addCell(const QString &duration, bool tie = false, bool isRest = false);
  void insertCell(int index, const RhythmCell &cell);
  void removeCell(int index);
  void removeLast();
  void setCell(int index, const RhythmCell &cell);
  void clear();

  // Accessors
  int cellCount() const { return m_cells.size(); }
  bool isEmpty() const { return m_cells.isEmpty(); }
  RhythmCell cell(int index) const;
  QList<RhythmCell> cells() const { return m_cells; }

  // Duration helpers
  void setDuration(int index, const QString &duration);
  void setTie(int index, bool tie);
  void setRest(int index, bool isRest);
  void toggleRest(int index);
  void toggleTie(int index);

  // Serialization
  QJsonArray toJson() const;
  static RhythmGrid fromJson(const QJsonArray &json);

  // Legacy compatibility - convert to/from simple string list
  QList<QString> toDurationList() const;
  static RhythmGrid fromDurationList(const QList<QString> &durations);

  // Calculate total duration in beats (assuming quarter = 1 beat)
  double totalBeats() const;

private:
  QList<RhythmCell> m_cells;

  static double durationToBeats(const QString &duration);
};

