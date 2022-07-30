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

#include "shortcutsmodel.h"

#include "ui/view/iconcodes.h"
#include "translation.h"
#include "types/translatablestring.h"
#include "log.h"

using namespace mu::shortcuts;
using namespace mu::ui;

static QString shortcutsFileFilter()
{
    return mu::qtrc("shortcuts", "MuseScore shortcuts file") + " (*.xml)";
}

ShortcutsModel::ShortcutsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Shortcut& shortcut = m_shortcuts.at(index.row());

    switch (role) {
    case RoleTitle: return actionText(shortcut.action);
    case RoleIcon: return static_cast<int>(this->action(shortcut.action).iconCode);
    case RoleSequence: return sequencesToNativeText(shortcut.sequences);
    case RoleSearchKey: {
        const UiAction& action = this->action(shortcut.action);
        return QString::fromStdString(action.code) + action.title.qTranslated() + action.description.qTranslated()
               + sequencesToNativeText(shortcut.sequences);
    }
    }

    return QVariant();
}

const UiAction& ShortcutsModel::action(const std::string& actionCode) const
{
    return uiactionsRegister()->action(actionCode);
}

QString ShortcutsModel::actionText(const std::string& actionCode) const
{
    const UiAction& action = this->action(actionCode);

    if (action.description.isEmpty()) {
        return action.title.qTranslated();
    }

    return action.description.qTranslated();
}

int ShortcutsModel::rowCount(const QModelIndex&) const
{
    return m_shortcuts.size();
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIcon, "icon" },
        { RoleSequence, "sequence" },
        { RoleSearchKey, "searchKey" }
    };

    return roles;
}

void ShortcutsModel::load()
{
    beginResetModel();
    m_shortcuts.clear();

    for (const UiAction& action : uiactionsRegister()->getActions()) {
        if (action.title.isEmpty() || action.description.isEmpty()) {
            continue;
        }

        Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
        if (shortcut.action != action.code) {
            shortcut.action = action.code;
            shortcut.context = action.scCtx;
        }

        m_shortcuts << shortcut;
    }

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        load();
    });

    std::sort(m_shortcuts.begin(), m_shortcuts.end(), [this](const Shortcut& s1, const Shortcut& s2) {
        return actionText(s1.action) < actionText(s2.action);
    });

    endResetModel();
}

bool ShortcutsModel::apply()
{
    ShortcutList shortcuts;

    for (const Shortcut& shortcut : qAsConst(m_shortcuts)) {
        shortcuts.push_back(shortcut);
    }

    Ret ret = shortcutsRegister()->setShortcuts(shortcuts);

    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

void ShortcutsModel::reset()
{
    shortcutsRegister()->resetShortcuts();
}

QItemSelection ShortcutsModel::selection() const
{
    return m_selection;
}

QVariant ShortcutsModel::currentShortcut() const
{
    QModelIndex index = currentShortcutIndex();
    if (!index.isValid()) {
        return QVariant();
    }

    const Shortcut& sc = m_shortcuts.at(index.row());
    return shortcutToObject(sc);
}

QVariant ShortcutsModel::getShortcut(QString action) const
{
    for (const Shortcut& shortcut : m_shortcuts) {
        if (action == QString::fromStdString(shortcut.action)) {
            return shortcutToObject(shortcut);
        }
    }

    LOGE() << "No action found in shortcutsmodel.cpp!";
    return QVariant();
}

QModelIndex ShortcutsModel::currentShortcutIndex() const
{
    if (m_selection.size() == 1) {
        return m_selection.indexes().first();
    }

    return QModelIndex();
}

void ShortcutsModel::setSelection(const QItemSelection& selection)
{
    if (m_selection == selection) {
        return;
    }

    m_selection = selection;
    emit selectionChanged();
}

void ShortcutsModel::importShortcutsFromFile()
{
    io::path_t path = interactive()->selectOpeningFile(
        qtrc("shortcuts", "Import shortcuts"),
        globalConfiguration()->homePath(),
        shortcutsFileFilter());

    if (!path.empty()) {
        shortcutsRegister()->importFromFile(path);
    }
}

void ShortcutsModel::exportShortcutsToFile()
{
    io::path_t path = interactive()->selectSavingFile(
        qtrc("shortcuts", "Export shortcuts"),
        globalConfiguration()->homePath(),
        shortcutsFileFilter());

    if (path.empty()) {
        return;
    }

    Ret ret = shortcutsRegister()->exportToFile(path);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ShortcutsModel::applySequenceToCurrentShortcut(const QString& newSequence, int conflictShortcutIndex)
{
    QModelIndex currIndex = currentShortcutIndex();
    if (!currIndex.isValid()) {
        return;
    }

    int row = currIndex.row();
    m_shortcuts[row].sequences = Shortcut::sequencesFromString(newSequence.toStdString());

    LOGE() << "Conflict at: " << conflictShortcutIndex;
    if (conflictShortcutIndex >= 0 && conflictShortcutIndex < m_shortcuts.size()) {
        m_shortcuts[conflictShortcutIndex].clear();
        notifyAboutShortcutChanged(index(conflictShortcutIndex));
    }

    notifyAboutShortcutChanged(currIndex);
}

void ShortcutsModel::applySequenceToPalette(QString action, const QString& newSequence, QModelIndex cellIdx, int conflictShortcutIndex)
{
    int i = 0;
    LOGE() << "prev size:" << m_shortcuts.size();

    LOGE() << "Applying shortcut to: " + action;

    for (Shortcut& shortcut : m_shortcuts) {
        if (action == QString::fromStdString(shortcut.action)) {
            shortcut.sequences = Shortcut::sequencesFromString(newSequence.toStdString());
            notifyAboutShortcutChanged(index(i));
        }
        i += 1;
    }

    LOGE() << "after size:" << m_shortcuts.size();

    if (conflictShortcutIndex >= 0 && conflictShortcutIndex < m_shortcuts.size()) {
        m_shortcuts[conflictShortcutIndex].sequences.clear();
        notifyAboutShortcutChanged(index(conflictShortcutIndex));
    }

    ShortcutList shortcuts;

    for (const Shortcut& shortcut : qAsConst(m_shortcuts)) {
        shortcuts.push_back(shortcut);
    }

    Ret ret = shortcutsRegister()->setShortcuts(shortcuts);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ShortcutsModel::clearSelectedShortcuts()
{
    for (const QModelIndex& index : m_selection.indexes()) {
        Shortcut& shortcut = m_shortcuts[index.row()];
        shortcut.clear();

        notifyAboutShortcutChanged(index);
    }
}

void ShortcutsModel::clearSequenceOfShortcut(QString action)
{
    int i = 0;
    for (Shortcut& shortcut : m_shortcuts) {
        if (action == QString::fromStdString(shortcut.action)) {
            shortcut.sequences.clear();
            shortcut.standardKey = QKeySequence::StandardKey::UnknownKey;
            notifyAboutShortcutChanged(index(i));
            return;
        }
        i += 1;
    }
}

void ShortcutsModel::notifyAboutShortcutChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

void ShortcutsModel::resetToDefaultSelectedShortcuts()
{
    auto resolveConflicts = [this](const Shortcut& shortcut) {
        for (int i = 0; i < m_shortcuts.size(); ++i) {
            Shortcut& sc = m_shortcuts[i];

            if (shortcut == sc) {
                continue;
            }

            if (!areContextPrioritiesEqual(shortcut.context, sc.context)) {
                continue;
            }

            if (shortcut.sequences == sc.sequences) {
                sc.clear();
                notifyAboutShortcutChanged(index(i));
            }
        }
    };

    for (const QModelIndex& index : m_selection.indexes()) {
        Shortcut& shortcut = m_shortcuts[index.row()];

        const Shortcut& defaultShortcut = shortcutsRegister()->defaultShortcut(shortcut.action);
        if (defaultShortcut.isValid()) {
            shortcut = defaultShortcut;
        } else {
            shortcut.sequences = {};
        }

        resolveConflicts(shortcut);

        notifyAboutShortcutChanged(index);
    }
}

QVariantList ShortcutsModel::shortcuts() const
{
    QVariantList result;

    for (const Shortcut& shortcut : qAsConst(m_shortcuts)) {
        result << shortcutToObject(shortcut);
    }

    return result;
}

QVariant ShortcutsModel::shortcutToObject(const Shortcut& shortcut) const
{
    QVariantMap obj;
    obj["title"] = actionText(shortcut.action);
    obj["action"] = QString::fromStdString(shortcut.action);
    obj["sequence"] = QString::fromStdString(shortcut.sequencesAsString());
    obj["context"] = QString::fromStdString(shortcut.context);

    return obj;
}
