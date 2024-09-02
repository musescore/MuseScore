/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include <functional>
#include <QString>
#include <QTimer>
#include <QQuickPaintedItem>

#include "global/types/version.h"

namespace muse::diagnostics {
class GSLogDest;
class GSTestObj;
class GSProblemDetector
{
public:

    GSProblemDetector(const Version& appVersion);
    ~GSProblemDetector();

    using OnResult = std::function<void (bool res)>;

    bool isNeedUseSoftwareRender() const;
    void setIsNeedUseSoftwareRender(bool arg);

    void listen(const OnResult& f);
    void destroy();

    static GSTestObj* gsTestObj;

private:

    QString softwareMarkerFilePath() const;

    Version m_appVersion;
    GSLogDest* m_logDest = nullptr;
    OnResult m_onResult;
    QTimer m_timer;
};

class GSTestObj : public QQuickPaintedItem
{
public:
    GSTestObj();
    ~GSTestObj();

    bool painted = false;

    void paint(QPainter*) override;
};
}
