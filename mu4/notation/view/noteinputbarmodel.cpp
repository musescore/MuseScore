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
#include "noteinputbarmodel.h"

#include "log.h"
#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::workspace;
using namespace mu::framework;

static const std::string TOOLBAR_TAG("Toolbar");
static const std::string NOTE_INPUT_TOOLBAR_NAME("noteInput");

static const std::string ADD_ACTION_NAME("add");
static const std::string ADD_ACTION_TITLE("Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;

static QList<mu::actions::ActionName> noteInputModeActionNames = {
    { "note-input" },
    { "note-input-rhythm" },
    { "note-input-repitch" },
    { "note-input-realtime-auto" },
    { "note-input-realtime-manual" },
    { "note-input-timewise" },
};

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    const ActionItem& item = m_items.at(index.row());
    switch (role) {
    case IconRole: return static_cast<int>(item.action.iconCode);
    case SectionRole: return item.section;
    case NameRole: return QString::fromStdString(item.action.name);
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int NoteInputBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> NoteInputBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { IconRole, "iconRole" },
        { SectionRole, "sectionRole" },
        { NameRole, "nameRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void NoteInputBarModel::load()
{
    m_items.clear();

    beginResetModel();

    std::vector<std::string> noteInputActions = currentWorkspaceActions();

    int section = 0;
    for (const std::string& actionName: noteInputActions) {
        if (actionName.empty()) {
            section++;
            continue;
        }

        m_items << makeActionItem(actionsRegister()->action(actionName), QString::number(section));
    }

    m_items << makeAddItem(QString::number(++section));

    endResetModel();

    emit countChanged(rowCount());

    RetValCh<std::shared_ptr<IWorkspace> > workspace = workspaceManager()->currentWorkspace();

    if (workspace.ret) {
        workspace.ch.onReceive(this, [this](std::shared_ptr<IWorkspace>) {
            load();
        });

        workspace.val->dataChanged().onReceive(this, [this](const AbstractDataPtr data) {
            if (data->name == NOTE_INPUT_TOOLBAR_NAME) {
                load();
            }
        });
    }

    onNotationChanged();

    m_notationChanged = context()->currentNotationChanged();
    m_notationChanged.onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });
}

NoteInputBarModel::ActionItem& NoteInputBarModel::item(const actions::ActionName& actionName)
{
    for (ActionItem& item : m_items) {
        if (item.action.name == actionName) {
            return item;
        }
    }

    LOGE() << "item not found with name: " << actionName;
    static ActionItem null;
    return null;
}

int NoteInputBarModel::findNoteInputModeItemIndex() const
{
    for (int i = 0; i < m_items.size(); i++) {
        if (noteInputModeActionNames.contains(m_items[i].action.name)) {
            return i;
        }
    }

    return -1;
}

void NoteInputBarModel::onNotationChanged()
{
    INotationPtr notation = context()->currentNotation();

    //! NOTE Unsubscribe from previous notation, if it was
    m_notationChanged.resetOnNotify(this);
    m_noteInputStateChanged.resetOnNotify(this);

    if (notation) {
        m_noteInputStateChanged = noteInput()->stateChanged();
        m_noteInputStateChanged.onNotify(this, [this]() {
            updateState();
        });

        interaction()->selectionChanged().onNotify(this, [this]() {
            updateState();
        });
    }

    updateState();
}

void NoteInputBarModel::toggleNoteInput()
{
    if (!noteInput()) {
        return;
    }

    if (isNoteInputMode()) {
        noteInput()->endNoteInput();
    } else {
        noteInput()->startNoteInput();
    }
}

void NoteInputBarModel::updateState()
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

        updateNoteInputState();
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

void NoteInputBarModel::updateNoteInputState()
{
    updateNoteInputModeState();

    updateNoteDotState();
    updateNoteDurationState();
    updateNoteAccidentalState();
    updateTieState();
    updateSlurState();
    updateVoicesState();
}

void NoteInputBarModel::updateNoteInputModeState()
{
    int noteInputModeIndex = findNoteInputModeItemIndex();
    if (noteInputModeIndex == -1) {
        return;
    }

    m_items[noteInputModeIndex].action = currentNoteInputModeAction();
    m_items[noteInputModeIndex].checked = isNoteInputMode();

    emit dataChanged(index(noteInputModeIndex), index(noteInputModeIndex));
}

void NoteInputBarModel::updateNoteDotState()
{
    static QMap<actions::ActionName, int> noteInputDots = {
        { "pad-dot", 1 },
        { "pad-dotdot", 2 },
        { "pad-dot3", 3 },
        { "pad-dot4", 4 }
    };

    int durationDots = noteInputState().duration.dots();

    for (const actions::ActionName& actionName: noteInputDots.keys()) {
        item(actionName).checked = durationDots == noteInputDots[actionName];
    }
}

void NoteInputBarModel::updateNoteDurationState()
{
    static QMap<actions::ActionName, DurationType> noteInputDurations = {
        { "note-longa", DurationType::V_LONG },
        { "note-breve", DurationType::V_BREVE },
        { "pad-note-1", DurationType::V_WHOLE },
        { "pad-note-2", DurationType::V_HALF },
        { "pad-note-4", DurationType::V_QUARTER },
        { "pad-note-8", DurationType::V_EIGHTH },
        { "pad-note-16", DurationType::V_16TH },
        { "pad-note-32", DurationType::V_32ND },
        { "pad-note-64", DurationType::V_64TH },
        { "pad-note-128", DurationType::V_128TH },
        { "pad-note-256", DurationType::V_256TH },
        { "pad-note-512", DurationType::V_512TH },
        { "pad-note-1024", DurationType::V_1024TH }
    };

    DurationType durationType = noteInputState().duration.type();

    for (const actions::ActionName& actionName: noteInputDurations.keys()) {
        item(actionName).checked = durationType == noteInputDurations[actionName];
    }
}

void NoteInputBarModel::updateNoteAccidentalState()
{
    static QMap<actions::ActionName, AccidentalType> noteInputAccidentals = {
        { "flat2", AccidentalType::FLAT2 },
        { "flat", AccidentalType::FLAT },
        { "nat", AccidentalType::NATURAL },
        { "sharp", AccidentalType::SHARP },
        { "sharp2", AccidentalType::SHARP2 }
    };

    AccidentalType accidentalType = noteInputState().accidentalType;

    for (const actions::ActionName& actionName: noteInputAccidentals.keys()) {
        item(actionName).checked = accidentalType == noteInputAccidentals[actionName];
    }
}

void NoteInputBarModel::updateTieState()
{
    std::vector<Note*> tiedNotes = selection()->notes(NoteFilter::WithTie);

    bool checked = !tiedNotes.empty();
    for (const Note* note: tiedNotes) {
        if (!note->tieFor()) {
            checked = false;
            break;
        }
    }

    item("tie").checked = checked;
}

void NoteInputBarModel::updateSlurState()
{
    bool checked = false;

    if (isNoteInputMode()) {
        checked = noteInputState().withSlur;
    } else {
        NOT_IMPLEMENTED;
    }

    item("add-slur").checked = noteInputState().withSlur;
}

void NoteInputBarModel::updateVoicesState()
{
    QMap<actions::ActionName, int> voices {
        { "voice-1", 0 },
        { "voice-2", 1 },
        { "voice-3", 2 },
        { "voice-4", 3 }
    };

    int currentVoice = resolveCurrentVoiceIndex();

    for (const actions::ActionName& actionName: voices.keys()) {
        item(actionName).checked = currentVoice == voices[actionName];
    }
}

int NoteInputBarModel::resolveCurrentVoiceIndex() const
{
    constexpr int INVALID_VOICE = -1;

    if (!noteInput() || !selection()) {
        return INVALID_VOICE;
    }

    if (isNoteInputMode()) {
        return noteInputState().currentVoiceIndex;
    }

    if (selection()->isNone()) {
        return INVALID_VOICE;
    }

    for (const Element* element: selection()->elements()) {
        if (element->isNote()) {
            return element->voice();
        }
    }

    return INVALID_VOICE;
}

bool NoteInputBarModel::isNoteInputModeAction(const ActionName& actionName) const
{
    return noteInputModeActionNames.contains(actionName);
}

Action NoteInputBarModel::currentNoteInputModeAction() const
{
    static QMap<NoteInputMethod, actions::ActionName> noteInputActionNames = {
        { NoteInputMethod::STEPTIME, "note-input" },
        { NoteInputMethod::RHYTHM, "note-input-rhythm" },
        { NoteInputMethod::REPITCH, "note-input-repitch" },
        { NoteInputMethod::REALTIME_AUTO, "note-input-realtime-auto" },
        { NoteInputMethod::REALTIME_MANUAL, "note-input-realtime-manual" },
        { NoteInputMethod::TIMEWISE, "note-input-timewise" },
    };

    NoteInputMethod method = noteInputState().method;
    return actionsRegister()->action(noteInputActionNames[method]);
}

NoteInputBarModel::ActionItem NoteInputBarModel::makeActionItem(const Action& action, const QString& section)
{
    ActionItem item;
    item.action = action;
    item.section = section;
    return item;
}

NoteInputBarModel::ActionItem NoteInputBarModel::makeAddItem(const QString& section)
{
    Action addAction(ADD_ACTION_NAME, ADD_ACTION_TITLE, shortcuts::ShortcutContext::Undefined, ADD_ACTION_ICON_CODE);
    return makeActionItem(addAction, section);
}

std::vector<std::string> NoteInputBarModel::currentWorkspaceActions() const
{
    RetValCh<std::shared_ptr<IWorkspace> > workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret) {
        LOGE() << workspace.ret.toString();
        return {};
    }

    AbstractDataPtr abstractData = workspace.val->data(TOOLBAR_TAG, NOTE_INPUT_TOOLBAR_NAME);
    ToolbarDataPtr toolbarData = std::dynamic_pointer_cast<ToolbarData>(abstractData);
    if (!toolbarData) {
        LOGE() << "Failed to get data of actions for " << NOTE_INPUT_TOOLBAR_NAME;
        return {};
    }

    return toolbarData->actions;
}

void NoteInputBarModel::handleAction(const QString& action)
{
    if (isNoteInputModeAction(action.toStdString())) {
        toggleNoteInput();
        return;
    }

    dispatcher()->dispatch(actions::namefromQString(action));
}

QVariantMap NoteInputBarModel::get(int index)
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

INotationPtr NoteInputBarModel::notation() const
{
    return context()->currentNotation();
}

INotationInteractionPtr NoteInputBarModel::interaction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

INotationSelectionPtr NoteInputBarModel::selection() const
{
    return interaction() ? interaction()->selection() : nullptr;
}

INotationNoteInputPtr NoteInputBarModel::noteInput() const
{
    return interaction() ? interaction()->noteInput() : nullptr;
}

bool NoteInputBarModel::isNoteInputMode() const
{
    return noteInput() ? noteInput()->isNoteInputMode() : false;
}

NoteInputState NoteInputBarModel::noteInputState() const
{
    return noteInput() ? noteInput()->state() : NoteInputState();
}
