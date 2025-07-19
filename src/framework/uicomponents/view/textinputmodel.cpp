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

#include "textinputmodel.h"

#include <QKeySequence>

#include "shortcuts/shortcutstypes.h"

using namespace muse::uicomponents;
using namespace muse::shortcuts;

TextInputModel::TextInputModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void TextInputModel::init()
{
    shortcutsRegister()->shortcutsChanged().onNotify(this, [this](){
        loadShortcuts();
    });

    loadShortcuts();
}

bool TextInputModel::isShortcutAllowedOverride(Qt::Key key, Qt::KeyboardModifiers modifiers) const
{
    auto [newKey, newModifiers] = correctKeyInput(key, modifiers);

    if (needIgnoreKey(newKey)) {
        return true;
    }

    const Shortcut& shortcut = this->shortcut(newKey, newModifiers);
    return !shortcut.isValid();
}

bool TextInputModel::handleShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    auto [newKey, newModifiers] = correctKeyInput(key, modifiers);

    if (needIgnoreKey(newKey)) {
        return false;
    }

    const Shortcut& shortcut = this->shortcut(newKey, newModifiers);
    bool found = shortcut.isValid();
    if (found) {
        dispatcher()->dispatch(shortcut.action);
    }

    return found;
}

void TextInputModel::loadShortcuts()
{
    //! NOTE: from navigation actions
    static const std::vector<std::string> actionCodes {
        "nav-next-section",
        "nav-prev-section",
        "nav-next-panel",
        "nav-prev-panel",
        "nav-next-tab",
        "nav-prev-tab",
        "nav-trigger-control",
        "nav-up",
        "nav-down",
        "nav-first-control",
        "nav-last-control",
        "nav-nextrow-control",
        "nav-prevrow-control"
    };

    for (const std::string& actionCode : actionCodes) {
        m_notAllowedForOverrideShortcuts.push_back(shortcutsRegister()->shortcut(actionCode));
    }
}

Shortcut TextInputModel::shortcut(Qt::Key key, Qt::KeyboardModifiers modifiers) const
{
    QKeySequence keySequence(modifiers | key);
    for (const Shortcut& shortcut : m_notAllowedForOverrideShortcuts) {
        for (const std::string& seq : shortcut.sequences) {
            QKeySequence shortcutSequence(QString::fromStdString(seq));
            if (shortcutSequence == keySequence) {
                return shortcut;
            }
        }
    }

    return Shortcut();
}
