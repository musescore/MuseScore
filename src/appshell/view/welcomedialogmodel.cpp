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
#include "log.h"

using namespace mu::appshell;

WelcomeDialogModel::WelcomeDialogModel()
{
}

void WelcomeDialogModel::init()
{
    // TODO: This is entirely placeholder code - figure out where to store/pull this information...
    QVariantMap item1;
    item1.insert("title", muse::qtrc("appshell", "What’s new in MuseScore Studio"));
    item1.insert("imageUrl", "https://i.ytimg.com/vi/-I-InDHIzdQ/hq720.jpg");
    item1.insert("description", muse::qtrc("appshell",
                                           "Includes a new system for dynamics, a new input mode, the ability to add system markings anywhere on a score and much more."));
    item1.insert("buttonText", muse::qtrc("appshell", "Watch video"));
    item1.insert("destinationUrl", "https://www.youtube.com/watch?v=-I-InDHIzdQ");

    QVariantMap item2;
    item2.insert("title", "Get amazing playback with MuseSounds!");
    item2.insert("imageUrl", "https://i.ytimg.com/vi/n7UgN69e2Y8/hq720.jpg");
    item2.insert("description", muse::qtrc("appshell",
                                           "Get free MuseSounds for orchestra, guitars & drumline, plus pro libraries from Spitfire, VSL, Berlin, Audio Imperia & more."));
    item2.insert("buttonText", muse::qtrc("appshell", "Get it on MuseHub"));
    item2.insert("destinationUrl", "https://www.musehub.com/");

    QVariantMap item3;
    item3.insert("title",
                 "Placeholder title on two lines - an example where the source text is is extremely long and gets truncated before it can finish.");
    item3.insert("imageUrl", "https://i.ytimg.com/vi/-I-InDHIzdQ/hq720.jpg");
    item3.insert("description",
                 "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    item3.insert("buttonText", muse::qtrc("appshell", "Button text"));

    QVariantMap item4;
    item4.insert("title", muse::qtrc("appshell", "Never lose your work with free cloud storage!"));
    item4.insert("imageUrl", "qrc:/SaveToCloud/images/Cloud.png");
    item4.insert("description", muse::qtrc("appshell",
                                           "While working on your score, your progress is backed up on MuseScore.com"));
    item4.insert("buttonText", muse::qtrc("appshell", "Get free cloud storage now"));
    item4.insert("destinationUrl", "https://musescore.com/");

    m_items.emplaceBack(item1);
    m_items.emplaceBack(item2);
    m_items.emplaceBack(item3);
    m_items.emplaceBack(item4);

    m_currentIndex = configuration()->welcomeDialogLastShownIndex();
    ++m_currentIndex;
    if (m_currentIndex >= count()) {
        // Cycle back to first item...
        m_currentIndex = 0;
    }
    IF_ASSERT_FAILED(m_currentIndex >= 0) {
        m_currentIndex = 0;
    }
    configuration()->setWelcomeDialogLastShownIndex(m_currentIndex);

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
    IF_ASSERT_FAILED(hasNext()) {
        return;
    }
    ++m_currentIndex;
    configuration()->setWelcomeDialogLastShownIndex(m_currentIndex);
    emit currentItemChanged();
}

void WelcomeDialogModel::prevItem()
{
    IF_ASSERT_FAILED(hasPrev()) {
        return;
    }
    --m_currentIndex;
    configuration()->setWelcomeDialogLastShownIndex(m_currentIndex);
    emit currentItemChanged();
}

bool WelcomeDialogModel::showOnStartup() const
{
    return configuration()->welcomeDialogShowOnStartup();
}

void WelcomeDialogModel::setShowOnStartup(bool show)
{
    configuration()->setWelcomeDialogShowOnStartup(show);
    emit showOnStartupChanged();
}
