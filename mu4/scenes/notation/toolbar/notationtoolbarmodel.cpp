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
#include "notationtoolbarmodel.h"

#include "log.h"
#include "domain/notation/notationactions.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;
using namespace mu::actions;

NotationToolBarModel::NotationToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant NotationToolBarModel::data(const QModelIndex& index, int role) const
{
    const ActionItem& item = m_items.at(index.row());
    switch (role) {
    case TitleRole: return QString::fromStdString(item.action.title);
    case NameRole: return QString::fromStdString(item.action.name);
    case EnabledRole: return item.enabled;
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int NotationToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> NotationToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "nameRole" },
        { TitleRole, "titleRole" },
        { EnabledRole, "enabledRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void NotationToolBarModel::load()
{
    auto makeItem = [](const Action& action) {
                        ActionItem item;
                        item.action = action;
                        return item;
                    };

    beginResetModel();

    m_items << makeItem(NotationActions::action("domain/notation/file-open"))
            << makeItem(NotationActions::action("domain/notation/note-input"))
            << makeItem(NotationActions::action("domain/notation/pad-note-16"))
            << makeItem(NotationActions::action("domain/notation/pad-note-8"))
            << makeItem(NotationActions::action("domain/notation/pad-note-4"));

    endResetModel();

    onNotationChanged();
    m_notationChanged = globalContext()->currentNotationChanged();
    m_notationChanged.onNotify(this, [this]() {
        onNotationChanged();
    });
}

NotationToolBarModel::ActionItem& NotationToolBarModel::item(const actions::ActionName& name)
{
    for (ActionItem& item : m_items) {
        if (item.action.name == name) {
            return item;
        }
    }

    LOGE() << "item not found with name: " << name;
    static ActionItem null;
    return null;
}

void NotationToolBarModel::onNotationChanged()
{
    updateState();
    std::shared_ptr<INotation> notation = globalContext()->currentNotation();
    if (notation) {
        m_inputStateChanged = notation->inputStateChanged();
        m_inputStateChanged.onNotify(this, [this]() {
            updateState();
        });
    } else {
        m_notationChanged.resetOnNotify(this);
        m_inputStateChanged.resetOnNotify(this);
    }
}

void NotationToolBarModel::updateState()
{
    std::shared_ptr<INotation> notation = globalContext()->currentNotation();
    if (!notation) {
        for (ActionItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (ActionItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        auto is = notation->inputState();
        if (is->isNoteEnterMode()) {
            item("domain/notation/note-input").checked = true;

            item("domain/notation/pad-note-4").checked = is->duration() == DurationType::V_QUARTER;
            item("domain/notation/pad-note-8").checked = is->duration() == DurationType::V_EIGHTH;
            item("domain/notation/pad-note-16").checked = is->duration() == DurationType::V_16TH;
        } else {
            for (ActionItem& item : m_items) {
                item.checked = false;
            }
        }
    }

    item("domain/notation/file-open").enabled = true;

    emit dataChanged(index(0), index(rowCount() - 1));
}

void NotationToolBarModel::click(const QString& action)
{
    dispatcher()->dispatch(actions::namefromQString(action));
}
