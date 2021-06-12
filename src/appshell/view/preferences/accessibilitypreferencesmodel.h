/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_ACCESSIBILITYPREFERENCESMODEL_H
#define MU_APPSHELL_ACCESSIBILITYPREFERENCESMODEL_H

#include <QObject>
//#include "appearancepreferencesmodel.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
//#include "async/asyncable.h"

namespace mu::appshell {
class AccessibilityPreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)

    Q_PROPERTY(QVariantList highContrastThemes READ highContrastThemes NOTIFY themesChanged)
    Q_PROPERTY(int currentThemeIndex READ currentThemeIndex WRITE setCurrentThemeIndex NOTIFY themesChanged)

public:
    explicit AccessibilityPreferencesModel(QObject* parent = nullptr);

    QVariantList highContrastThemes() const;
    int currentThemeIndex() const;

public slots:
    void setCurrentThemeIndex(int index);

signals:
    void themesChanged();

};
}


#endif // MU_APPSHELL_ACCESSIBILITYPREFERENCESMODEL_H
