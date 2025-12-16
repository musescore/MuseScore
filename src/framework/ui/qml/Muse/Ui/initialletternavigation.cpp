/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "initialletternavigation.h"
#include "log.h"

using namespace muse::ui;

static QSet<Qt::Key> NAVIGATION_KEYS = {
    Qt::Key_Enter,
    Qt::Key_Return,
    Qt::Key_Up,
    Qt::Key_Down,
    Qt::Key_Left,
    Qt::Key_Right,
    Qt::Key_Tab,
    Qt::Key_PageUp,
    Qt::Key_PageDown,
};

static QString normalizeForSearch(const QString& string)
{
    return string.normalized(QString::NormalizationForm_KD).toLower();
}

InitialLetterNavigation::InitialLetterNavigation(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_inputBufferTimer.setSingleShot(true);
    m_inputBufferTimer.setInterval(1000);

    QObject::connect(&m_inputBufferTimer, &QTimer::timeout, this, [this]() {
        clearBuffer();
    });

    qApp->installEventFilter(this);
}

QStringList InitialLetterNavigation::stringList() const
{
    return m_stringList;
}

void InitialLetterNavigation::setStringList(const QStringList& stringList)
{
    if (stringList == m_stringList) {
        return;
    }
    m_stringList = stringList;
    emit stringListChanged();
}

NavigationPanel* InitialLetterNavigation::panel() const
{
    return m_panel;
}

void InitialLetterNavigation::setPanel(NavigationPanel* panel)
{
    if (panel == m_panel) {
        return;
    }
    m_panel = panel;
    emit panelChanged();
}

int InitialLetterNavigation::controlColumn() const
{
    return m_controlColumn;
}

void InitialLetterNavigation::setControlColumn(int controlColumn)
{
    if (controlColumn == m_controlColumn) {
        return;
    }
    m_controlColumn = controlColumn;
    emit controlColumnChanged();
}

bool InitialLetterNavigation::eventFilter(QObject* watched, QEvent* event)
{
    if (!event || event->type() != QEvent::ShortcutOverride) {
        return QObject::eventFilter(watched, event);
    }

    const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    const Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
    const bool eventKeyIsSpace = key == Qt::Key::Key_Space;
    const bool isNavKey = NAVIGATION_KEYS.contains(key);
    if (isNavKey || (eventKeyIsSpace && m_inputString.isEmpty())) {
        // Ignore all navigation keys apart from space (unless space is the first thing we type)...
        clearBuffer();
        return QObject::eventFilter(watched, event);
    }

    const QString eventText = normalizeForSearch(keyEvent->text());

    static const QRegularExpression controlCharsExceptSpace("[\\x00-\\x1F\\x7F]");
    if (eventText.isEmpty() || eventText.contains(controlCharsExceptSpace)) {
        // Ignore text containing ASCII control characters...
        clearBuffer();
        return QObject::eventFilter(watched, event);
    }

    event->accept();
    m_inputBufferTimer.start();

    // If the last input was the same as this one we "cycle" the entries starting at the active control...
    if (!m_prevEventText.isEmpty() && eventText != m_prevEventText) {
        m_cycling = false; // Keep it `true` on the first keypress.
    }
    m_inputString.append(eventText);
    m_prevEventText = eventText;

    const int indexOfMatch = indexForInput(eventText);
    if (indexOfMatch < 0) {
        return !eventKeyIsSpace; // Ignore space if it's the last character typed and there are no matches...
    }

    //! NOTE: This has to happen before navigateToIndex in some cases - the view might be
    //! deleting navigation controls when they're not visible...
    emit requestVisible(indexOfMatch);

    const INavigation::Index navIndex = { m_controlColumn, indexOfMatch };
    navigateToIndex(navIndex);

    return true;
}

int InitialLetterNavigation::indexForInput(const QString& eventText) const
{
    const INavigationControl* control = navigationController()->activeControl();
    const int activeIndex = control ? control->index().row : 0;
    const int startIndex = activeIndex + (m_cycling ? 1 : 0);

    const auto indexHasMatch = [this, eventText](int index) -> bool {
        const QString& itemString = normalizeForSearch(m_stringList.at(index));
        const QString& searchString = m_cycling ? eventText : m_inputString;
        return itemString.startsWith(searchString);
    };

    for (int i = startIndex; i < m_stringList.size(); ++i) {
        if (indexHasMatch(i)) {
            return i;
        }
    }

    for (int i = 0; i < startIndex; ++i) {
        if (indexHasMatch(i)) {
            return i;
        }
    }

    return -1;
}

void InitialLetterNavigation::navigateToIndex(const INavigation::Index& index)
{
    const INavigationSection* section = m_panel ? m_panel->section() : nullptr;
    IF_ASSERT_FAILED(section) {
        return;
    }
    const std::string sectionName = section->name().toStdString();
    const std::string panelName = m_panel->name().toStdString();
    const bool ok = navigationController()->requestActivateByIndex(sectionName, panelName, index);
    IF_ASSERT_FAILED(ok) {
        LOGE() << "Index not found in this panel/section - make sure nav control indices match m_stringList indices";
        return;
    }

    if (!navigationController()->isHighlight()) {
        navigationController()->setIsHighlight(true);
    }
}

void InitialLetterNavigation::clearBuffer()
{
    m_cycling = true;
    m_inputString.clear();
    m_prevEventText.clear();
}
