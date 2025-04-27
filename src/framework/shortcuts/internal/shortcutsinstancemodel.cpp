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
#include "shortcutsinstancemodel.h"

#include <private/qkeymapper_p.h>
#include "log.h"
#include "shortcutstypes.h"

using namespace muse::shortcuts;

ShortcutsInstanceModel::ShortcutsInstanceModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void ShortcutsInstanceModel::init()
{
    shortcutsRegister()->shortcutsChanged().onNotify(this, [this](){
        doLoadShortcuts();
    });

    shortcutsRegister()->activeChanged().onNotify(this, [this](){
        emit activeChanged();
    });

    doLoadShortcuts();
    qApp->installEventFilter(this);
}

QVariantMap ShortcutsInstanceModel::shortcuts() const
{
    return m_shortcuts;
}

bool ShortcutsInstanceModel::active() const
{
    return shortcutsRegister()->active();
}

void ShortcutsInstanceModel::activate(const QString& seq)
{
    std::vector<QString> sequences{ seq };
    doActivate(sequences);
}

void ShortcutsInstanceModel::activateAmbiguous(const QString& seq)
{
    std::vector<QString> sequences;
    sequences.push_back(seq);

    QKeySequence ks(seq);
    QKeyCombination kc = ks[0];
    int kcKey = kc.toCombined();
    int key = m_shortcutSequences.find(kcKey)->second;

    for (auto it = m_shortcutSequences.begin(); it != m_shortcutSequences.end(); ++it) {
        if (it->second == key) {
            if (it->first == kcKey) {
                continue;
            }
            QKeyCombination match = QKeyCombination::fromCombined(it->first);
            QString matchSeq = QKeySequence(match).toString();
            sequences.push_back(matchSeq);
        }
    }

    doActivate(sequences);
}

void ShortcutsInstanceModel::doLoadShortcuts()
{
    m_shortcuts.clear();

    const ShortcutList& shortcuts = shortcutsRegister()->shortcuts();
    for (const Shortcut& sc : shortcuts) {
        for (const std::string& seq : sc.sequences) {
            QString seqStr = QString::fromStdString(seq);

            // RULE: If a sequence is used for several shortcuts but the values for autoRepeat vary depending on
            // the context, then we should force autoRepeat to false for all shortcuts sharing the sequence in
            // question. This prevents the creation of ambiguous shortcuts (see QShortcutEvent::isAmbiguous)
            auto search = m_shortcuts.find(seqStr);
            if (search == m_shortcuts.end()) {
                // Sequence not found, add it...
                m_shortcuts.insert(seqStr, QVariant(sc.autoRepeat));
            } else if (search.value().toBool() && !sc.autoRepeat) {
                // Sequence already exists, but we need to enforce the above rule...
                search.value() = false;
            }
        }
    }

    emit shortcutsChanged();
}

bool ShortcutsInstanceModel::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() != QEvent::ShortcutOverride || watched != mainWindow()->qWindow()) {
        return false;
    }

    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    if (!keyEvent) {
        return false;
    }

    if (needIgnoreKey((Qt::Key)keyEvent->key())) {
        return false;
    }

    m_shortcutSequences.clear(); // Is this safe? Multiple events very close in time?
    QList<int> possibleKeys = QKeyMapper::possibleKeys(keyEvent);
    if (possibleKeys.size() < 2) {
        return false;
    }

    for (auto it = possibleKeys.cbegin(); it != possibleKeys.cend(); ++it) {
        if (m_shortcutSequences.find(*it) == m_shortcutSequences.end()) {
            m_shortcutSequences.insert({ *it, 0 });
        }
    }
    int key = QKeyCombination(keyEvent->modifiers(), (Qt::Key)keyEvent->key()).toCombined();
    for (auto it = m_shortcutSequences.begin(); it != m_shortcutSequences.end(); ++it) {
        it->second = possibleKeys.contains(it->first) ? key : 0;
    }

    return false;
}

void ShortcutsInstanceModel::doActivate(std::vector<QString> sequences)
{
    std::vector<std::string> translatedSequences;
    translatedSequences.reserve(sequences.size());
    for (const QString& seq : sequences) {
        translatedSequences.push_back(seq.toStdString());
    }
    controller()->activate(translatedSequences);
}
