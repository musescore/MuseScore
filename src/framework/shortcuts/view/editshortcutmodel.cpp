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
#include <private/qkeymapper_p.h>

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

    qApp->installEventFilter(this);
}

void EditShortcutModel::clearNewSequence()
{
    if (m_newSequences.empty() && m_conflictShortcut.isEmpty()) {
        return;
    }

    m_newSequences = std::vector<QKeySequence>();
    m_conflictShortcut.clear();

    emit newSequenceChanged();
}

void EditShortcutModel::setWindow(QObject* window)
{
    m_window = window;
}

void EditShortcutModel::newShortcutFieldFocusChanged(bool focused)
{
    m_newShortcutFieldFocused = focused;
}

void EditShortcutModel::currentShortcutAcceptInProgress()
{
    m_currentShortcutAcceptInProgress = true;
}

void EditShortcutModel::inputKey(QKeyEvent* keyEvent)
{
    Qt::Key key = (Qt::Key)keyEvent->key();

    if (m_newShortcutFieldFocused || needIgnoreKey(key) || key == Qt::Key_Tab || key == Qt::Key_Escape) {
        return;
    }

    if (m_currentShortcutAcceptInProgress) {
        m_currentShortcutAcceptInProgress = false;
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            return;
        }
    }

    std::vector<QKeySequence> newSequences;
    QVector<int> possibleKeys = QKeyMapper::possibleKeys(keyEvent).toVector();
    for (int i = possibleKeys.size() - 1; i >= 0; --i) {
        if ((possibleKeys[i] & Qt::KeypadModifier) != 0) {
            possibleKeys.push_back(possibleKeys[i] & ~Qt::KeypadModifier);
        }
    }
    std::sort(possibleKeys.begin(), possibleKeys.end());
    std::reverse(possibleKeys.begin(), possibleKeys.end()); // we start with the combinations with the most modifiers, those are the greater values
    int indexOfLastPressedShortcut = possibleKeys.indexOf(m_lastPressedShortcut);
    int indexOfPossibleKeyToUse = indexOfLastPressedShortcut >= 0 ? indexOfLastPressedShortcut + 1 : 0;
    if (indexOfPossibleKeyToUse >= possibleKeys.size()) {
        indexOfPossibleKeyToUse = 0;
    }
    m_lastPressedShortcut = possibleKeys[indexOfPossibleKeyToUse];
    m_conflictShortcut.clear();
    for (int i = 0; i < possibleKeys.size(); ++i) {
        QKeyCombination combination = QKeyCombination::fromCombined(possibleKeys[i]);

        bool exists = false;
        for (const QKeySequence& sequence : newSequences) {
            for (int j = 0; j < sequence.count(); ++j) {
                if (sequence[j] == combination) {
                    exists = true;
                    break;
                }
            }
        }

        if (exists) {
            continue;
        }

        QKeySequence newSequence = QKeySequence(combination);
        //if (m_newSequence == newSequence) {
        //    return;
        //}

        if (i == indexOfPossibleKeyToUse) {
            newSequences.push_back(newSequence);
        }

        if (m_conflictShortcut.isEmpty()) {
            checkNewSequenceForConflicts(newSequence);
        }
    }

    m_newSequences.clear();
    m_newSequences.insert(m_newSequences.end(), newSequences.begin(), newSequences.end());

    if (possibleKeys.size() > 1) {
        m_alternatives = muse::qtrc("shortcuts", "Shortcut has alternative spellings. Keep pressing the same keys to switch between them.");
    } else {
        m_alternatives = QString();
    }

    emit newSequenceChanged();
}

void EditShortcutModel::clear()
{
    clearNewSequence();
    m_cleared = true;
    emit originSequenceChanged();
    emit clearedChanged();
}

void EditShortcutModel::editShortcut(const QString& shortcutText)
{
    std::vector<std::string> seqs;
    muse::strings::split(shortcutText.toStdString(), seqs, "; ");

    std::vector<QKeySequence> newSequences;
    for (const std::string& seq : seqs) {
        if (seq.empty()) {
            continue;
        }
        QKeySequence newSequence(QString::fromStdString(seq), QKeySequence::SequenceFormat::NativeText);
        if (!newSequence.toString().isEmpty()) { // valid?
            newSequences.push_back(newSequence);
        }
    }

    m_conflictShortcut.clear();
    m_newSequences.clear();
    for (const QKeySequence& newSequence : newSequences) {
        m_newSequences.push_back(newSequence);
        if (m_conflictShortcut.isEmpty()) {
            checkNewSequenceForConflicts(newSequence);
        }
    }

    emit newSequenceChanged();
}

void EditShortcutModel::checkNewSequenceForConflicts(QKeySequence newSequence)
{
    m_conflictShortcut.clear();
    const std::string input = newSequence.toString().toStdString();

    for (const QVariant& shortcut : m_potentialConflictShortcuts) {
        QVariantMap map = shortcut.toMap();
        if (map["title"].toString() == m_originShortcutTitle) {
            continue;
        }

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
    QString str;
    bool first = true;
    for (const QKeySequence& seq : m_newSequences) {
        if (!first) {
            str += "; ";
        } else {
            first = false;
        }
        str += seq.toString(QKeySequence::NativeText);
    }
    return str;
}

QString EditShortcutModel::conflictWarning() const
{
    QString title = m_conflictShortcut["title"].toString();
    if (title.isEmpty()) {
        return QString();
    }

    return muse::qtrc("shortcuts", "This shortcut is already assigned to: <b>%1</b>").arg(title);
}

QString EditShortcutModel::alternatives() const
{
    return m_alternatives;
}

void EditShortcutModel::trySave()
{
    QString newSequence = this->newSequence();
    const bool alreadyEmpty = originSequenceInNativeFormat().isEmpty() && m_cleared;
    if (alreadyEmpty /*|| m_originSequence == newSequence*/) {
        return;
    }

    newSequence = newSequence.replace("; ", ", ");
    m_originSequence = newSequence;

    QString conflictWarn = conflictWarning();

    if (conflictWarn.isEmpty()) {
        emit applyNewSequenceRequested(m_originSequence);
        return;
    }

    QString str = conflictWarn + "<br><br>" + muse::qtrc("shortcuts", "Are you sure you want to assign it to <b>%1</b> instead?")
                  .arg(m_originShortcutTitle);

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);

    IInteractive::Button btn = interactive()->warning(muse::trc("shortcuts", "Reassign shortcut"), text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        interactive()->buttonData(IInteractive::Button::Ok)
    }, (int)IInteractive::Button::Ok).standardButton();

    if (btn != IInteractive::Button::Ok) {
        return;
    }

    int conflictShortcutIndex = m_allShortcuts.indexOf(m_conflictShortcut);
    emit applyNewSequenceRequested(m_originSequence, conflictShortcutIndex);
}

QString EditShortcutModel::newSequence() const
{
    QString str;
    bool first = true;
    for (const QKeySequence& seq : m_newSequences) {
        if (!first) {
            str += "; ";
        } else {
            first = false;
        }
        str += seq.toString();
    }
    return str;
}

bool EditShortcutModel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_window) {
        if (event->type() == QEvent::KeyPress) {
            inputKey((QKeyEvent*)event);
        } else if (event->type() == QEvent::ShortcutOverride) {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (!keyEvent) {
                return false;
            }
            // ???
        }
    }

    return false;
}
