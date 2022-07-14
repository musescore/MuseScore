/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_APPSHELL_SPLASHSCREEN_H
#define MU_APPSHELL_SPLASHSCREEN_H

#include <QSplashScreen>

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"

class QSvgRenderer;

namespace mu::appshell {
class SplashScreen : public QSplashScreen
{
    Q_OBJECT

    //! The uiConfiguration has not yet been initialized, so we can't use user settings.
    //! We only use it for querying the default system font
    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)

public:
    SplashScreen();

private:
    void drawContents(QPainter* painter) override;

    QSvgRenderer* m_backgroundRenderer = nullptr;
};
}

#endif // MU_APPSHELL_SPLASHSCREEN_H
