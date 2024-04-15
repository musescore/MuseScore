/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef IMPORTMIDI_MODEL_H
#define IMPORTMIDI_MODEL_H

#include "importmidi_operations.h"

#include <memory>

#include <QAbstractTableModel>

namespace mu::iex::midi {
class TracksModel : public QAbstractTableModel
{
public:
    TracksModel();
    ~TracksModel();

    void reset(const MidiOperations::Opers& opers, const QList<std::string>& lyricsList, int trackCount, const QString& midiFile,
               bool hasHumanBeats, bool hasTempoText, bool hasChordNames);
    void clear();
    void updateCharset();
    void notifyAllApplied();

    const MidiOperations::Opers& trackOpers() const;
    int trackCount() const { return _trackCount; }
    int trackCountForImport() const;
    int frozenRowCount() const;
    int frozenColCount() const;
    int rowFromTrackIndex(int trackIndex) const;
    int trackIndexFromRow(int row) const;
    bool isAllApplied() const { return _isAllApplied; }

    int rowCount(const QModelIndex& /*parent*/) const;
    int columnCount(const QModelIndex& /*parent*/) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

private:
    bool isTrackIndexValid(int trackIndex) const;
    bool isRowValid(int row) const;
    bool isColumnValid(int column) const;
    void forceRowDataChanged(int row);
    void forceColumnDataChanged(int col);
    void forceAllChanged();
    bool editableSingleTrack(int trackIndex, int column) const;
    Qt::ItemFlags editableFlags(int row, int col) const;

    MidiOperations::Opers _trackOpers;
    int _trackCount;
    int _frozenColCount;
    QString _midiFile;
    class Column;
    std::vector<std::unique_ptr<Column> > _columns;
    bool _isAllApplied;
};
} // namespace mu::iex::midi

#endif // IMPORTMIDI_MODEL_H
