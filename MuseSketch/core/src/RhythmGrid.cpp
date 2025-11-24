#include "RhythmGrid.h"

void RhythmGrid::addCell(const RhythmCell &cell) { m_cells.append(cell); }

void RhythmGrid::addCell(const QString &duration, bool tie, bool isRest) {
  m_cells.append(RhythmCell(duration, tie, isRest));
}

void RhythmGrid::insertCell(int index, const RhythmCell &cell) {
  if (index >= 0 && index <= m_cells.size()) {
    m_cells.insert(index, cell);
  }
}

void RhythmGrid::removeCell(int index) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells.removeAt(index);
  }
}

void RhythmGrid::removeLast() {
  if (!m_cells.isEmpty()) {
    m_cells.removeLast();
  }
}

void RhythmGrid::setCell(int index, const RhythmCell &cell) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index] = cell;
  }
}

void RhythmGrid::clear() { m_cells.clear(); }

RhythmCell RhythmGrid::cell(int index) const {
  if (index >= 0 && index < m_cells.size()) {
    return m_cells[index];
  }
  return RhythmCell();
}

void RhythmGrid::setDuration(int index, const QString &duration) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index].duration = duration;
  }
}

void RhythmGrid::setTie(int index, bool tie) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index].tie = tie;
  }
}

void RhythmGrid::setRest(int index, bool isRest) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index].isRest = isRest;
  }
}

void RhythmGrid::toggleRest(int index) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index].isRest = !m_cells[index].isRest;
  }
}

void RhythmGrid::toggleTie(int index) {
  if (index >= 0 && index < m_cells.size()) {
    m_cells[index].tie = !m_cells[index].tie;
  }
}

QJsonArray RhythmGrid::toJson() const {
  QJsonArray array;
  for (const RhythmCell &cell : m_cells) {
    array.append(cell.toJson());
  }
  return array;
}

RhythmGrid RhythmGrid::fromJson(const QJsonArray &json) {
  RhythmGrid grid;
  for (const auto &value : json) {
    if (value.isObject()) {
      grid.addCell(RhythmCell::fromJson(value.toObject()));
    } else if (value.isString()) {
      // Legacy format: just duration strings
      grid.addCell(value.toString());
    }
  }
  return grid;
}

QList<QString> RhythmGrid::toDurationList() const {
  QList<QString> durations;
  for (const RhythmCell &cell : m_cells) {
    durations.append(cell.duration);
  }
  return durations;
}

RhythmGrid RhythmGrid::fromDurationList(const QList<QString> &durations) {
  RhythmGrid grid;
  for (const QString &dur : durations) {
    grid.addCell(dur);
  }
  return grid;
}

double RhythmGrid::totalBeats() const {
  double total = 0.0;
  for (const RhythmCell &cell : m_cells) {
    total += durationToBeats(cell.duration);
  }
  return total;
}

double RhythmGrid::durationToBeats(const QString &duration) {
  // Assuming quarter note = 1 beat
  if (duration == "whole")
    return 4.0;
  if (duration == "half")
    return 2.0;
  if (duration == "dotted-half")
    return 3.0;
  if (duration == "quarter")
    return 1.0;
  if (duration == "dotted-quarter")
    return 1.5;
  if (duration == "eighth")
    return 0.5;
  if (duration == "dotted-eighth")
    return 0.75;
  if (duration == "sixteenth")
    return 0.25;
  if (duration == "triplet-eighth")
    return 1.0 / 3.0;
  return 1.0; // Default to quarter
}

