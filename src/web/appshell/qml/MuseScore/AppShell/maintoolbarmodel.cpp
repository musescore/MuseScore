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

#include "maintoolbarmodel.h"

#include "translation.h"

#include "project/notationpageinfo.h"

using namespace mu::appshell;
using namespace muse::uicomponents;

static const QString DEVTOOLS_PAGE("musescore://devtools");

MainToolBarModel::MainToolBarModel(QObject* parent)
    : PageTabsModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

MainToolBarModel::~MainToolBarModel() = default;

void MainToolBarModel::load()
{
    m_notationPage = std::make_unique<project::NotationPageInfo>(iocContext());

    QList<PageTab> tabs;
    m_notationTabIndex = static_cast<int>(tabs.size());
    tabs << notationTab();

    if (globalConfiguration()->devModeEnabled()) {
        tabs << PageTab { muse::qtrc("appshell", "DevTools"), DEVTOOLS_PAGE };
    }

    setTabs(tabs);

    m_notationPage->changed().onNotify(this, [this]() {
        setTab(m_notationTabIndex, notationTab());
    });
}

PageTab MainToolBarModel::notationTab() const
{
    PageTab tab;
    tab.title = muse::qtrc("appshell", "Score");
    tab.uri = m_notationPage->uri();
    tab.alsoCheckedOnUris = project::NotationPageInfo::pageUris();
    tab.isTitleBold = m_notationPage->isProjectOpened();

    return tab;
}
