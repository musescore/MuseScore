//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_MIDI_EDITMIDIMAPPINGMODEL_H
#define MU_MIDI_EDITMIDIMAPPINGMODEL_H

#include <QObject>

namespace mu::midi {
class EditMidiMappingModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString mappingTitle READ mappingTitle NOTIFY mappingTitleChanged)

public:
    explicit EditMidiMappingModel(QObject* parent = nullptr);

    QString mappingTitle() const;

    Q_INVOKABLE void load(int originValue);
    Q_INVOKABLE int inputedValue() const;

signals:
    void mappingTitleChanged(const QString& title);

private:
    int m_originValue = -1;
    int m_inputedValue = -1;
};
}

#endif // MU_MIDI_EDITMIDIMAPPINGMODEL_H
