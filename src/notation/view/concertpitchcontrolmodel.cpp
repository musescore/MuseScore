//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "concertpitchcontrolmodel.h"

using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

ConcertPitchControlModel::ConcertPitchControlModel(QObject* parent)
    : QObject(parent)
{
}

void ConcertPitchControlModel::load()
{
    dispatcher()->reg(this, "concert-pitch", [this](const ActionCode& actionCode, const ActionData& args) {
        Q_UNUSED(actionCode)
        bool enabled = args.count() > 0 ? args.arg<bool>(0) : false;
        setConcertPitchEnabled(enabled);
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        emit concertPitchEnabledChanged();

        if (!style()) {
            return;
        }

        style()->styleChanged().onNotify(this, [this]() {
            emit concertPitchEnabledChanged();
        });
    });
}

bool ConcertPitchControlModel::concertPitchEnabled() const
{
    return style() ? style()->styleValue(StyleId::concertPitch).toBool() : false;
}

void ConcertPitchControlModel::setConcertPitchEnabled(bool enabled)
{
    if (!notation()) {
        return;
    }

    if (concertPitchEnabled() == enabled) {
        return;
    }

    notation()->undoStack()->prepareChanges();
    style()->setStyleValue(StyleId::concertPitch, enabled);
    notation()->undoStack()->commitChanges();
}

INotationStylePtr ConcertPitchControlModel::style() const
{
    return notation() ? notation()->style() : nullptr;
}

INotationPtr ConcertPitchControlModel::notation() const
{
    return context()->currentNotation();
}
