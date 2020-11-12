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
#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::workspace;
using namespace mu::framework;

static const std::string NOTE_INPUT_TOOLBAR_NAME("noteInput");

static const std::string ADD_ACTION_NAME("add");
static const std::string ADD_ACTION_TITLE("Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;
static const QString ADD_ITEM_SECTION = QString::number(1000);

NotationToolBarModel::NotationToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant NotationToolBarModel::data(const QModelIndex& index, int role) const
{
    const ActionItem& item = m_items.at(index.row());
    switch (role) {
    case IconRole: return static_cast<int>(item.action.iconCode);
    case SectionRole: return item.section;
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
        { IconRole, "iconRole" },
        { SectionRole, "sectionRole" },
        { NameRole, "nameRole" },
        { TitleRole, "titleRole" },
        { EnabledRole, "enabledRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void NotationToolBarModel::load()
{
    RetValCh<std::shared_ptr<IWorkspace> > workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret) {
        LOGW() << workspace.ret.toString();
        return;
    }

    beginResetModel();

    m_items.clear();

    std::vector<std::string> noteInputActions = workspace.val->toolbarActions(NOTE_INPUT_TOOLBAR_NAME);

    auto areg = aregister();

    int section = 0;
    for (const std::string& actionName: noteInputActions) {
        if (actionName == "") {
            section++;
            continue;
        }

        m_items << makeItem(areg->action(actionName), QString::number(section));
    }

    m_items << makeAddItem();

    endResetModel();

    emit countChanged(rowCount());

    workspace.ch.onReceive(this, [this](std::shared_ptr<IWorkspace>) {
        load();
    });

    onNotationChanged();
    m_notationChanged = globalContext()->currentNotationChanged();
    m_notationChanged.onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
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
    std::shared_ptr<INotation> notation = globalContext()->currentNotation();

    //! NOTE Unsubscribe from previous notation, if it was
    m_notationChanged.resetOnNotify(this);
    m_inputStateChanged.resetOnNotify(this);

    if (notation) {
        m_inputStateChanged = notation->interaction()->inputStateChanged();
        m_inputStateChanged.onNotify(this, [this]() {
            updateState();
        });
    }

    updateState();
}

void NotationToolBarModel::updateState()
{
    bool isPlaying = playbackController()->isPlaying();
    if (!notation() || isPlaying) {
        for (ActionItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (ActionItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        updateInputState();
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

void NotationToolBarModel::updateInputState()
{
    auto is = notation()->interaction()->inputState();
    if (!is->isNoteEnterMode()) {
        for (ActionItem& item : m_items) {
            item.checked = false;
        }

        return;
    }

    item("note-input").checked = true;

    item("note-longa").checked = is->duration() == DurationType::V_LONG;
    item("note-breve").checked = is->duration() == DurationType::V_BREVE;
    item("pad-note-1").checked = is->duration() == DurationType::V_WHOLE;
    item("pad-note-2").checked = is->duration() == DurationType::V_HALF;
    item("pad-note-4").checked = is->duration() == DurationType::V_QUARTER;
    item("pad-note-8").checked = is->duration() == DurationType::V_EIGHTH;
    item("pad-note-16").checked = is->duration() == DurationType::V_16TH;
    item("pad-note-32").checked = is->duration() == DurationType::V_32ND;
    item("pad-note-64").checked = is->duration() == DurationType::V_64TH;
    item("pad-note-128").checked = is->duration() == DurationType::V_128TH;
    item("pad-note-256").checked = is->duration() == DurationType::V_256TH;
    item("pad-note-512").checked = is->duration() == DurationType::V_512TH;
    item("pad-note-1024").checked = is->duration() == DurationType::V_1024TH;
}

NotationToolBarModel::ActionItem NotationToolBarModel::makeItem(const Action& action, const QString& section)
{
    ActionItem item;
    item.action = action;
    item.section = section;
    return item;
}

NotationToolBarModel::ActionItem NotationToolBarModel::makeAddItem()
{
    Action addAction(ADD_ACTION_NAME, ADD_ACTION_TITLE, shortcuts::ShortcutContext::Undefined, ADD_ACTION_ICON_CODE);
    return makeItem(addAction, ADD_ITEM_SECTION);
}

void NotationToolBarModel::click(const QString& action)
{
    dispatcher()->dispatch(actions::namefromQString(action));
}

QVariantMap NotationToolBarModel::get(int index)
{
    QVariantMap result;

    QHash<int,QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
}

INotationPtr NotationToolBarModel::notation() const
{
    return globalContext()->currentNotation();
}
