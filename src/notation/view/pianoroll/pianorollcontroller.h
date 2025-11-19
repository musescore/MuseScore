/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include <QObject>
#include <QVariant>
#include "notation/notation.h"
#include "engraving/dom/drumset.h"

namespace mu::notation {

class PianoRollController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant drumset READ drumset NOTIFY drumsetChanged)

public:
    explicit PianoRollController(QObject* parent = nullptr);

    void setNotation(INotationPtr notation);

    QVariant drumset() const;

    Q_INVOKABLE QStringList drumNames() const;
    Q_INVOKABLE int pitch(const QString& name) const;
    Q_INVOKABLE void addNote(int pitch, int tick);

signals:
    void drumsetChanged();

private:
    const mu::engraving::Drumset* currentDrumset() const;

    INotationPtr m_notation;
};

} // namespace mu::notation
