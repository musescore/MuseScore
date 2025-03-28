/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "editpercussionshortcutmodel.h"
#include "shortcuts/shortcutstypes.h"

using namespace muse;
using namespace mu::notation;

EditPercussionShortcutModel::EditPercussionShortcutModel(QObject* parent)
    : QObject(parent), Injectable(iocCtxForQmlObject(this))
{
}

void EditPercussionShortcutModel::load(const QVariant& originDrum, const QVariantList& drumsWithShortcut,
                                       const QVariantList& applicationShortcuts)
{
    m_originDrum = originDrum.toMap();
    m_applicationShortcuts = applicationShortcuts;

    for (const QVariant& drum : drumsWithShortcut) {
        if (drum == originDrum) {
            continue;
        }
        m_drumsWithShortcut << drum;
    }

    m_cleared = false;

    emit originShortcutTextChanged();
    emit clearedChanged();
}

void EditPercussionShortcutModel::inputKey(Qt::Key key)
{
    if (needIgnoreKey(key)) {
        return;
    }

    const QKeyCombination combination(key);
    const QKeySequence newShortcut = QKeySequence(combination);
    if (m_newShortcut == newShortcut) {
        return;
    }

    m_newShortcut = newShortcut;

    m_conflictInAppShortcuts = false;
    if (!checkDrumShortcutsForConflict()) {
        // Only need to do this if we haven't already found a conflict in the drums...
        m_conflictInAppShortcuts = checkApplicationShortcutsForConflict();
    }

    emit newShortcutTextChanged();
}

void EditPercussionShortcutModel::clear()
{
    m_newShortcut = QKeySequence();
    m_conflictShortcut.clear();

    m_cleared = true;

    emit newShortcutTextChanged();
    emit clearedChanged();
}

bool EditPercussionShortcutModel::trySave()
{
    const QString newShortcut = m_newShortcut.toString();
    const bool alreadyEmpty = originShortcutText().isEmpty() && m_cleared;
    if (alreadyEmpty || originShortcutText() == newShortcut) {
        return false;
    }

    if (m_conflictShortcut.isEmpty()) {
        // No conflicts to warn about, apply the changes...
        return true;
    }

    const QString originTitle = m_originDrum.value("title").toString();
    const QString headerText = qtrc("shortcuts", "Reassign shortcut for <b>%1</b>").arg(originTitle);
    const IInteractive::Text text(getConflictWarningText().toStdString(), IInteractive::TextFormat::RichText);

    const IInteractive::Button btn = interactive()->warning(headerText.toStdString(), text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        interactive()->buttonData(IInteractive::Button::Ok)
    }, (int)IInteractive::Button::Ok).standardButton();

    if (btn != IInteractive::Button::Ok) {
        return false;
    }

    return true;
}

int EditPercussionShortcutModel::conflictDrumPitch() const
{
    if (m_conflictShortcut.isEmpty() || m_conflictInAppShortcuts) {
        return -1;
    }
    return m_conflictShortcut.value("pitch").toInt();
}

QString EditPercussionShortcutModel::originShortcutText() const
{
    return m_originDrum.value("shortcut").toString();
}

QString EditPercussionShortcutModel::newShortcutText() const
{
    return m_newShortcut.toString(QKeySequence::NativeText);
}

QString EditPercussionShortcutModel::informationText() const
{
    QString title = m_conflictShortcut["title"].toString();
    if (title.isEmpty()) {
        return QString();
    }
    return qtrc("shortcuts", "This shortcut is already assigned to: <b>%1</b>").arg(title);
}

bool EditPercussionShortcutModel::checkDrumShortcutsForConflict()
{
    m_conflictShortcut.clear();
    const QString newDrumShortcut = m_newShortcut.toString();

    for (const QVariant& drum : m_drumsWithShortcut) {
        const QVariantMap otherDrum = drum.toMap();

        const QString otherDrumShortcut = otherDrum.value("shortcut").toString();
        if (newDrumShortcut == otherDrumShortcut) {
            m_conflictShortcut = otherDrum;
            return true;
        }
    }

    return false;
}

bool EditPercussionShortcutModel::checkApplicationShortcutsForConflict()
{
    m_conflictShortcut.clear();
    const QString newDrumShortcut = m_newShortcut.toString();

    for (const QVariant& shortcut : m_applicationShortcuts) {
        const QVariantMap otherShortcut = shortcut.toMap();

        const QString otherShortcutStr = otherShortcut.value("shortcut").toString();
        if (newDrumShortcut == otherShortcutStr) {
            m_conflictShortcut = otherShortcut;
            return true;
        }
    }

    return false;
}

QString EditPercussionShortcutModel::getConflictWarningText() const
{
    if (m_conflictInAppShortcuts) {
        const QString conflictTitle = m_conflictShortcut["title"].toString();
        const QString conflictShortcut = m_conflictShortcut["shortcut"].toString();
        return qtrc("shortcuts", "When this instrument is selected, <b>%1</b> will no longer run <b>%2</b>.")
               .arg(conflictShortcut, conflictTitle);
    }

    const QString originTitle = m_originDrum.value("title").toString();
    return informationText() + "<br><br>" + qtrc("shortcuts", "Are you sure you want to assign it to <b>%1</b> instead?")
           .arg(originTitle);
}

bool EditPercussionShortcutModel::needIgnoreKey(const Qt::Key& key) const
{
    static const std::set<Qt::Key> ignoredKeys {
        Qt::Key_Enter,
        Qt::Key_Return,
        Qt::Key_Delete,
        Qt::Key_Backspace,
        Qt::Key_Down,
        Qt::Key_Up,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Insert,
        Qt::Key_Home,
        Qt::Key_PageUp,
        Qt::Key_PageDown,
        Qt::Key_End,
        Qt::Key_0,
        Qt::Key_1,
        Qt::Key_2,
        Qt::Key_3,
        Qt::Key_4,
        Qt::Key_5,
        Qt::Key_6,
        Qt::Key_7,
        Qt::Key_8,
        Qt::Key_9,
    };

    const bool keyFound = ignoredKeys.find(key) != ignoredKeys.end();
    return keyFound || shortcuts::needIgnoreKey(key);
}
