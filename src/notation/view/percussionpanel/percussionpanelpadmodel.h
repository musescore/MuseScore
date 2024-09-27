/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

class PercussionPanelPadModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString instrumentName READ instrumentName WRITE setInstrumentName NOTIFY instrumentNameChanged)
    Q_PROPERTY(bool isEmptySlot READ isEmptySlot WRITE setIsEmptySlot NOTIFY isEmptySlotChanged)

public:
    explicit PercussionPanelPadModel(QObject* parent = nullptr);

    QString instrumentName() const { return m_instrumentName; }
    void setInstrumentName(const QString& instrumentName);

    bool isEmptySlot() const { return m_isEmptySlot; }
    void setIsEmptySlot(bool isEmptySlot);

signals:
    void instrumentNameChanged();
    void isEmptySlotChanged();

private:
    bool m_isEmptySlot = true;
    QString m_instrumentName;
};
