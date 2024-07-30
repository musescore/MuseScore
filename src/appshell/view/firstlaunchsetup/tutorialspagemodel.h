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
#ifndef MU_APPSHELL_TUTORIALSPAGEMODEL_H
#define MU_APPSHELL_TUTORIALSPAGEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "update/iupdateconfiguration.h"

class QUrl;

namespace mu::appshell {
class TutorialsPageModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Inject<muse::update::IUpdateConfiguration> updateConfiguration = { this };

public:
    explicit TutorialsPageModel(QObject* parent = nullptr);

    Q_INVOKABLE QUrl museScorePrivacyPolicyUrl() const;
};
}

#endif // MU_APPSHELL_TUTORIALSPAGEMODEL_H
