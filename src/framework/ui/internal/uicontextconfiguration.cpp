/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "uicontextconfiguration.h"

#include <QScreen>
#include <QApplication>

using namespace muse::ui;

constexpr double DEFAULT_DPI = 96;

QScreen* UiContextConfiguration::screen() const
{
    if (!mainWindow()) {
        return QApplication::primaryScreen();
    }
    return mainWindow()->screen();
}

double UiContextConfiguration::guiScaling() const
{
    const QScreen* s = screen();
    return s ? s->devicePixelRatio() : 1;
}

double UiContextConfiguration::physicalDpi() const
{
    if (configuration()->customPhysicalDotsPerInch()) {
        return configuration()->customPhysicalDotsPerInch().value();
    }

    const QScreen* screen = this->screen();
    if (!screen) {
        return DEFAULT_DPI;
    }

    auto physicalSize = screen->physicalSize();
    // Work around xrandr reporting a 1x1mm size if
    // the screen doesn't have a valid physical size
    if (physicalSize.height() <= 1 && physicalSize.width() <= 1) {
        return DEFAULT_DPI;
    }

#ifdef Q_OS_WIN
    //! NOTE: copied from MU3, `MuseScore::MuseScore()`
    if (QOperatingSystemVersion::current() <= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 7)) {
        return screen->logicalDotsPerInch() * screen->devicePixelRatio();
    }
#endif
    return screen->physicalDotsPerInch();
}

double UiContextConfiguration::logicalDpi() const
{
    const QScreen* screen = this->screen();
    if (!screen) {
        return DEFAULT_DPI;
    }

    return screen->logicalDotsPerInch();
}
