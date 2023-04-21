/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_AUXSENDITEM_H
#define MU_PLAYBACK_AUXSENDITEM_H

#include <QObject>

#include "audio/audiotypes.h"

namespace mu::playback {
class AuxSendItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(float signalAmount READ signalAmount WRITE setSignalAmount NOTIFY signalAmountChanged)

public:
    explicit AuxSendItem(QObject* parent = nullptr);

    QString title() const;
    bool isActive() const;
    float signalAmount() const;

public slots:
    void setTitle(const QString& title);
    void setIsActive(bool active);
    void setSignalAmount(float amount);

signals:
    void titleChanged(const QString& title);
    void isActiveChanged(bool active);
    void signalAmountChanged(float amount);

private:
    QString m_title;
    audio::AuxSendParams m_params;
};
}

#endif // MU_PLAYBACK_AUXSENDITEM_H
