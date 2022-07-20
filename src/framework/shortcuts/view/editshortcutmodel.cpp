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

#include "editshortcutmodel.h"

#include <QKeySequence>

#include "translation.h"
#include "shortcutstypes.h"
#include "log.h"

using namespace mu::shortcuts;
using namespace mu::framework;

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent)
{
}

void EditShortcutModel::load(const QVariant& originShortcut, const QVariantList& allShortcuts)
{
    TRACEFUNC;

    clearNewSequence();

    m_allShortcuts = allShortcuts;
    m_potentialConflictShortcuts.clear();

    QVariantMap originShortcutMap = originShortcut.toMap();
    std::string originCtx = originShortcutMap.value("context").toString().toStdString();

    for (const QVariant& shortcut : allShortcuts) {
        if (shortcut == originShortcut) {
            continue;
        }

        QVariantMap map = shortcut.toMap();
        std::string ctx = map.value("context").toString().toStdString();

        if (areContextPrioritiesEqual(originCtx, ctx)) {
            m_potentialConflictShortcuts << shortcut;
        }
    }

    m_originSequence = originShortcutMap.value("sequence").toString();
    m_originShortcutTitle = originShortcutMap.value("title").toString();

    emit originSequenceChanged();
}

void EditShortcutModel::clearNewSequence()
{
    if (m_newSequence.isEmpty() && m_conflictShortcut.isEmpty()) {
        return;
    }

    m_newSequence = QKeySequence();
    m_conflictShortcut.clear();

    emit newSequenceChanged();
}

void EditShortcutModel::inputKey(int key, Qt::KeyboardModifiers modifiers)
{
    std::pair<int, Qt::KeyboardModifiers> correctedKeyInput = correctKeyInput(key, modifiers);
    int newKey = correctedKeyInput.first;
    int newModifiers = correctedKeyInput.second;

    if (needIgnoreKey(newKey)) {
        return;
    }

    newKey += newModifiers;

    // remove shift-modifier for keys that don't need it: letters and special keys
    if ((newKey & Qt::ShiftModifier) && ((key < 0x41) || (key > 0x5a) || (key >= 0x01000000))) {
        newKey -= Qt::ShiftModifier;
    }

    for (int i = 0; i < m_newSequence.count(); i++) {
        if (m_newSequence[i] == key) {
            return;
        }
    }

    QKeySequence newSequence = QKeySequence(newKey);
    if (m_newSequence == newSequence) {
        return;
    }

    m_newSequence = newSequence;
    checkNewSequenceForConflicts();

    emit newSequenceChanged();
}

void EditShortcutModel::checkNewSequenceForConflicts()
{
    m_conflictShortcut.clear();
    const std::string input = newSequence().toStdString();

    for (const QVariant& shortcut : m_potentialConflictShortcuts) {
        QVariantMap map = shortcut.toMap();

        std::vector<std::string> toCheckSequences = Shortcut::sequencesFromString(map.value("sequence").toString().toStdString());

        for (const std::string& toCheckSequence : toCheckSequences) {
            if (input == toCheckSequence) {
                m_conflictShortcut = map;
                return;
            }
        }
    }
}

QString EditShortcutModel::originSequenceInNativeFormat() const
{
    std::vector<std::string> sequences = Shortcut::sequencesFromString(m_originSequence.toStdString());

    return sequencesToNativeText(sequences);
}

QString EditShortcutModel::newSequenceInNativeFormat() const
{
    return m_newSequence.toString(QKeySequence::NativeText);
}

QString EditShortcutModel::conflictWarning() const
{
    QString title = m_conflictShortcut["title"].toString();
    if (title.isEmpty()) {
        return QString();
    }

    return qtrc("shortcuts", "This shortcut is already assigned to: <b>%1</b>").arg(title);
}

void EditShortcutModel::applyNewSequence()
{
    QString newSequence = this->newSequence();

    if (m_originSequence == newSequence) {
        return;
    }

    m_originSequence = newSequence;

    QString conflictWarn = conflictWarning();

    if (conflictWarn.isEmpty()) {
        emit applyNewSequenceRequested(m_originSequence);
        return;
    }

    QString str = conflictWarn + "<br><br>" + mu::qtrc("shortcuts", "Are you sure you want to assign it to <b>%2</b> instead?")
                  .arg(m_originShortcutTitle);

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    IInteractive::Button btn = interactive()->warning(mu::trc("shortcuts", "Reassign shortcut"), text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        okBtn,
    }, (int)IInteractive::Button::Ok, IInteractive::Option::WithIcon).standardButton();

    if (btn != IInteractive::Button::Ok) {
        return;
    }

    int conflictShortcutIndex = m_allShortcuts.indexOf(m_conflictShortcut);
    emit applyNewSequenceRequested(m_originSequence, conflictShortcutIndex);
}

QString EditShortcutModel::newSequence() const
{
    return m_newSequence.toString();
}
