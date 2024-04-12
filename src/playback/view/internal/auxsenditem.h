/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
    Q_PROPERTY(int audioSignalPercentage READ audioSignalPercentage WRITE setAudioSignalPercentage NOTIFY audioSignalPercentageChanged)

public:
    explicit AuxSendItem(QObject* parent = nullptr);

    QString title() const;
    bool isActive() const;
    int audioSignalPercentage() const;

public slots:
    void setTitle(const QString& title);
    void setIsActive(bool active);
    void setAudioSignalPercentage(int percentage);

signals:
    void titleChanged(const QString& title);
    void isActiveChanged(bool active);
    void audioSignalPercentageChanged(int percentage);

private:
    QString m_title;
    bool m_isActive = false;
    int m_audioSignalPercentage = 0;
};
}

#endif // MU_PLAYBACK_AUXSENDITEM_H
