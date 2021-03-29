//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "shortcutsmodel.h"

#include "ui/view/iconcodes.h"
#include "translation.h"
#include "log.h"

using namespace mu::shortcuts;
using namespace mu::actions;
using namespace mu::ui;

ShortcutsModel::ShortcutsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Shortcut shortcut = m_shortcuts[index.row()];
    QString title = actionTitle(shortcut.action);
    QString sequence = QString::fromStdString(shortcut.sequence);

    switch (role) {
    case RoleTitle: return actionTitle(shortcut.action);
    case RoleSequence: return sequence;
    case RoleSearchKey: return title + sequence;
    }

    return QVariant();
}

QString ShortcutsModel::actionTitle(const std::string& actionCode) const
{
    ActionItem action = actionsRegister()->action(actionCode);
    QString title = QString::fromStdString(action.title);

    if (action.iconCode == IconCode::Code::NONE) {
        return title;
    }

    return QString(iconCodeToChar(action.iconCode)) + " " + title;
}

int ShortcutsModel::rowCount(const QModelIndex&) const
{
    return m_shortcuts.size();
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleSequence, "sequence" },
        { RoleSearchKey, "searchKey" }
    };

    return roles;
}

void ShortcutsModel::load()
{
    beginResetModel();
    m_shortcuts.clear();

    for (const Shortcut& shortcut: shortcutsRegister()->shortcuts()) {
        if (actionTitle(shortcut.action).isEmpty()) {
            continue;
        }

        m_shortcuts << shortcut;
    }

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        load();
    });

    endResetModel();

    emit shortcutsChanged();
}

QItemSelection ShortcutsModel::selection() const
{
    return m_selection;
}

QString ShortcutsModel::currentSequence() const
{
    QModelIndex index = currentShortcutIndex();

    if (index.isValid()) {
        return QString::fromStdString(m_shortcuts[index.row()].sequence);
    }

    return QString();
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

QVariantList ShortcutsModel::shortcuts() const
{
    QVariantList result;

    for (const Shortcut& shortcut: m_shortcuts) {
        QVariantMap obj;
        obj["title"] = actionTitle(shortcut.action);
        obj["sequence"] = QString::fromStdString(shortcut.sequence);

        result << obj;
    }

    return result;
}

void ShortcutsModel::loadShortcutsFromFile()
{
    QString filter = qtrc("shortcuts", "MuseScore Shortcuts File") +  " (*.xml)";
    io::path selectedPath = interactive()->selectOpeningFile(qtrc("shortcuts", "Load Shortcuts"),
                                                             configuration()->shortcutsUserPath().val,
                                                             filter);
    configuration()->setShortcutsUserPath(selectedPath);
}

void ShortcutsModel::saveShortcutsToFile()
{
    NOT_IMPLEMENTED;
}

void ShortcutsModel::printShortcuts()
{
    NOT_IMPLEMENTED;
}

void ShortcutsModel::applySequenceToCurrentShortcut(const QString& newSequence)
{
    QModelIndex index = currentShortcutIndex();
    if (!index.isValid()) {
        return;
    }

    int row = index.row();
    m_shortcuts[row].sequence = newSequence.toStdString();

    notifyAboutShortcutChanged(index);
}

void ShortcutsModel::clearSelectedShortcuts()
{
    for (const QModelIndex& index : m_selection.indexes()) {
        Shortcut& shortcut = m_shortcuts[index.row()];
        shortcut.sequence.clear();
        shortcut.standardKey = QKeySequence::StandardKey::UnknownKey;

        notifyAboutShortcutChanged(index);
    }
}

void ShortcutsModel::notifyAboutShortcutChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
    emit shortcutsChanged();
}

void ShortcutsModel::resetToDefaultSelectedShortcuts()
{
    for (const QModelIndex& index : m_selection.indexes()) {
        Shortcut& shortcut = m_shortcuts[index.row()];
        shortcut = shortcutsRegister()->defaultShortcut(shortcut.action);

        notifyAboutShortcutChanged(index);
    }
}
