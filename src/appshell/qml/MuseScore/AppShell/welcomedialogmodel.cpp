/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "welcomedialogmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::appshell;

static std::vector<QVariantMap> welcomeDialogData()
{
    QVariantMap item1;
    item1.insert("title", muse::qtrc("appshell/welcome", "Enjoy free cloud storage"));
    item1.insert("imageUrl", "qrc:/resources/welcomedialog/MuseScoreCom.png");
    item1.insert("description", muse::qtrc("appshell/welcome",
                                           "Save your scores privately on MuseScore.com to revisit past versions and invite others to view and comment – and when you’re ready, share your music with the world."));
    item1.insert("buttonText", muse::qtrc("appshell/welcome", "View my scores online"));
    item1.insert("destinationUrl",
                 "https://musescore.com/my-scores?utm_source=mss-app-welcome-musescore-com&utm_medium=mss-app-welcome-musescore-com&utm_campaign=mss-app-welcome-musescore-com");

    QVariantMap item2;
    item2.insert("title", muse::qtrc("appshell/welcome", "What’s new in MuseScore Studio"));
    item2.insert("imageUrl", "qrc:/resources/welcomedialog/WhatsNew.png");
    item2.insert("description", muse::qtrc("appshell/welcome",
                                           "Includes a new system for hiding empty staves, a new text editing widget, guitar notation improvements, engraving improvements and more."));
    item2.insert("buttonText", muse::qtrc("appshell/welcome", "Watch video"));
    item2.insert("destinationUrl",
                 "https://www.youtube.com/watch?v=J2gY9CbMuoI&utm_source=mss-app-yt-4.6-release&utm_medium=mss-app-yt-4.6-release&utm_campaign=mss-app-yt-4.6-release");

    QVariantMap item3;
    item3.insert("title", muse::qtrc("appshell/welcome", "Install our free MuseSounds libraries"));
    item3.insert("imageUrl", "qrc:/resources/welcomedialog/MuseSounds.png");
    item3.insert("description", muse::qtrc("appshell/welcome",
                                           "Explore our collection of realistic sample libraries, including solo instruments, marching percussion, and full orchestra - available for free on MuseHub."));
    item3.insert("buttonText", muse::qtrc("appshell/welcome", "Get it on MuseHub"));
    item3.insert("destinationUrl",
                 "https://www.musehub.com/free-musesounds?utm_source=mss-app-welcome-free-musesounds&utm_medium=mss-app-welcome-free-musesounds&utm_campaign=mss-app-welcome-free-musesounds&utm_id=mss-app-welcome-free-musesounds");

    QVariantMap item4;
    item4.insert("title", muse::qtrc("appshell/welcome", "Explore our tutorials"));
    item4.insert("imageUrl", "qrc:/resources/welcomedialog/ExploreTutorials.png");
    item4.insert("description", muse::qtrc("appshell/welcome",
                                           "We’ve put together a playlist of tutorials to help both beginners and experienced users get the most out of MuseScore Studio."));
    item4.insert("buttonText", muse::qtrc("appshell/welcome", "View tutorials"));
    item4.insert("destinationUrl",
                 "https://www.youtube.com/playlist?list=PLTYuWi2LmaPECOZrC6bkPHBkYY9_WEexT&utm_source=mss-app-welcome-tutorials&utm_medium=mss-app-welcome-tutorials&utm_campaign=mss-app-welcome-tutorials&utm_id=mss-app-welcome-tutorials");

    return { item1, item2, item3, item4 };
}

WelcomeDialogModel::WelcomeDialogModel()
    : muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void WelcomeDialogModel::init()
{
    IF_ASSERT_FAILED(configuration()) {
        return;
    }

    m_items = welcomeDialogData();

    m_currentIndex = configuration()->welcomeDialogLastShownIndex();
    nextItem();

    IF_ASSERT_FAILED(m_currentIndex != muse::nidx) {
        m_currentIndex = 0;
    }
    configuration()->setWelcomeDialogLastShownIndex(static_cast<int>(m_currentIndex));

    emit itemsChanged();
    emit currentItemChanged();
}

QVariantMap WelcomeDialogModel::currentItem() const
{
    if (m_items.empty()) {
        return QVariantMap();
    }
    return m_items.at(m_currentIndex);
}

void WelcomeDialogModel::nextItem()
{
    IF_ASSERT_FAILED(!m_items.empty()) {
        return;
    }

    if (hasNext()) {
        ++m_currentIndex;
    } else {
        // Cycle to first...
        m_currentIndex = 0;
    }
    configuration()->setWelcomeDialogLastShownIndex(static_cast<int>(m_currentIndex));

    emit currentItemChanged();
}

void WelcomeDialogModel::prevItem()
{
    IF_ASSERT_FAILED(!m_items.empty()) {
        return;
    }

    if (hasPrev()) {
        --m_currentIndex;
    } else {
        // Cycle to last....
        m_currentIndex = count() - 1;
    }
    configuration()->setWelcomeDialogLastShownIndex(static_cast<int>(m_currentIndex));

    emit currentItemChanged();
}

bool WelcomeDialogModel::showOnStartup() const
{
    return configuration()->welcomeDialogShowOnStartup();
}

void WelcomeDialogModel::setShowOnStartup(bool show)
{
    if (show == showOnStartup()) {
        return;
    }

    configuration()->setWelcomeDialogShowOnStartup(show);
    emit showOnStartupChanged();
}
