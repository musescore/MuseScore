/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <memory>

#include <qqmlintegration.h>

#include "uicomponents/qml/Muse/UiComponents/pagetabsmodel.h"

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu::project {
class NotationPageInfo;
}

namespace mu::appshell {
class MainToolBarModel : public muse::uicomponents::PageTabsModel, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    QML_ELEMENT

    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;

public:
    explicit MainToolBarModel(QObject* parent = nullptr);
    ~MainToolBarModel() override;

    void load() override;

private:
    muse::uicomponents::PageTab notationTab() const;

    std::unique_ptr<project::NotationPageInfo> m_notationPage;
    int m_notationTabIndex = -1;
};
}
