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

using namespace muse::shortcuts;

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
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

    m_cleared = false;

    emit originSequenceChanged();
    emit clearedChanged();
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

void EditShortcutModel::inputKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    std::tie(key, modifiers) = correctKeyInput(key, modifiers);

    if (needIgnoreKey(key)) {
        return;
    }

    // remove shift-modifier for non-letter keys, except a few keys
    if ((modifiers & Qt::ShiftModifier) && !isShiftAllowed(key)) {
        modifiers &= ~Qt::ShiftModifier;
    }

    QKeyCombination combination(modifiers, key);

    for (int i = 0; i < m_newSequence.count(); i++) {
        if (m_newSequence[i] == combination) {
            return;
        }
    }

    QKeySequence newSequence = QKeySequence(combination);
    if (m_newSequence == newSequence) {
        return;
    }

    m_newSequence = newSequence;
    checkNewSequenceForConflicts();

    emit newSequenceChanged();
}

void EditShortcutModel::clear()
{
    clearNewSequence();
    m_cleared = true;
    emit originSequenceChanged();
    emit clearedChanged();
}

bool EditShortcutModel::isShiftAllowed(Qt::Key key)
{
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        return true;
    }

    // keys where Shift should not be removed
    switch (key) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Insert:
    case Qt::Key_Delete:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
    case Qt::Key_Escape:
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12:
    case Qt::Key_F13:
    case Qt::Key_F14:
    case Qt::Key_F15:
    case Qt::Key_F16:
    case Qt::Key_F17:
    case Qt::Key_F18:
    case Qt::Key_F19:
    case Qt::Key_F20:
    case Qt::Key_F21:
    case Qt::Key_F22:
    case Qt::Key_F23:
    case Qt::Key_F24:
    case Qt::Key_F25:
    case Qt::Key_F26:
    case Qt::Key_F27:
    case Qt::Key_F28:
    case Qt::Key_F29:
    case Qt::Key_F30:
    case Qt::Key_F31:
    case Qt::Key_F32:
    case Qt::Key_F33:
    case Qt::Key_F34:
    case Qt::Key_F35:
        return true;
    default:
        return false;
    }
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

    return muse::qtrc("shortcuts", "This shortcut is already assigned to: <b>%1</b>").arg(title);
}

void EditShortcutModel::trySave()
{
    QString newSequence = this->newSequence();
    const bool alreadyEmpty = originSequenceInNativeFormat().isEmpty() && m_cleared;
    if (alreadyEmpty || m_originSequence == newSequence) {
        return;
    }

    m_originSequence = newSequence;

    QString conflictWarn = conflictWarning();

    if (conflictWarn.isEmpty()) {
        emit applyNewSequenceRequested(m_originSequence);
        return;
    }

    QString str = conflictWarn + "<br><br>" + muse::qtrc("shortcuts", "Are you sure you want to assign it to <b>%1</b> instead?")
                  .arg(m_originShortcutTitle);

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);

    auto promise = interactive()->warningAsync(muse::trc("shortcuts", "Reassign shortcut"), text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        interactive()->buttonData(IInteractive::Button::Ok)
    }, (int)IInteractive::Button::Ok);

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Ok)) {
            int conflictShortcutIndex = m_allShortcuts.indexOf(m_conflictShortcut);
            emit applyNewSequenceRequested(m_originSequence, conflictShortcutIndex);
        }
    });
}

QString EditShortcutModel::newSequence() const
{
    return m_newSequence.toString();
}
