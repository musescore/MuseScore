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
#include "translation.h"
#include "ui/view/iconcodes.h"
#include "internal/notationactions.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::workspace;
using namespace mu::ui;
using namespace mu::uicomponents;

static const std::string TOOLBAR_TAG("Toolbar");
static const std::string TOOLBAR_NAME("noteInput");

static const std::string ADD_ACTION_CODE("add");
static const std::string ADD_ACTION_TITLE("Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;

static const ActionCode TUPLET_ACTION_CODE("tuplet");

static int INVALID_INDEX = -1;

static QMap<ActionCode, NoteInputMethod> noteInputModeActions = {
    { "note-input", NoteInputMethod::STEPTIME },
    { "note-input-rhythm", NoteInputMethod::RHYTHM },
    { "note-input-repitch", NoteInputMethod::REPITCH },
    { "note-input-realtime-auto", NoteInputMethod::REALTIME_AUTO },
    { "note-input-realtime-manual", NoteInputMethod::REALTIME_MANUAL },
    { "note-input-timewise", NoteInputMethod::TIMEWISE },
};

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    const MenuItem& item = m_items.at(index.row());
    switch (role) {
    case IconRole: return static_cast<int>(item.iconCode);
    case SectionRole: return QString::fromStdString(item.section);
    case CodeRole: return QString::fromStdString(item.code);
    case CheckedRole: return item.checked;
    case HintRole: return QString::fromStdString(item.description);
    case SubitemsRole: return subitems(item.code);
    case ShowSubitemsByPressAndHoldRole: return isNeedShowSubitemsByPressAndHold(item.code);
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
        { CodeRole, "codeRole" },
        { CheckedRole, "checkedRole" },
        { HintRole, "hintRole" },
        { SubitemsRole, "subitemsRole" },
        { ShowSubitemsByPressAndHoldRole, "showSubitemsByPressAndHoldRole" },
    };
    return roles;
}

ActionState NoteInputBarModel::actionState(const ActionCode& actionCode) const
{
    IMenuControllerPtr addMenuController = menuControllersRegister()->controller(MenuType::Add);
    if (addMenuController && addMenuController->contains(actionCode)) {
        return addMenuController->actionState(actionCode);
    }

    return AbstractMenuModel::actionState(actionCode);
}

void NoteInputBarModel::load()
{
    m_items.clear();

    beginResetModel();

    std::vector<std::string> noteInputActions = currentWorkspaceActions();

    int section = 0;
    for (const ActionCode& actionCode: noteInputActions) {
        if (actionCode.empty()) {
            section++;
            continue;
        }

        m_items << makeActionItem(actionsRegister()->action(actionCode), std::to_string(section));
    }

    m_items << makeAddItem(std::to_string(++section));

    endResetModel();

    emit countChanged(rowCount());

    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();

    if (workspace.ret) {
        workspace.ch.onReceive(this, [this](IWorkspacePtr) {
            load();
        });

        workspace.val->dataChanged().onReceive(this, [this](const AbstractDataPtr data) {
            if (data->name == TOOLBAR_NAME) {
                load();
            }
        });
    }

    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });

    IMenuControllerPtr addMenuController = menuControllersRegister()->controller(MenuType::Add);
    if (addMenuController) {
        addMenuController->actionsAvailableChanged().onReceive(this, [this](const ActionCodeList&) {
            notifyAboutTupletItemChanged();
            notifyAboutAddItemChanged();
        });
    }
}

MenuItem& NoteInputBarModel::item(const ActionCode& actionCode)
{
    for (MenuItem& item : m_items) {
        if (item.code == actionCode) {
            return item;
        }
    }

    static MenuItem null;
    return null;
}

int NoteInputBarModel::findNoteInputModeItemIndex() const
{
    for (int i = 0; i < m_items.size(); i++) {
        if (noteInputModeActions.contains(m_items[i].code)) {
            return i;
        }
    }

    return INVALID_INDEX;
}

void NoteInputBarModel::onNotationChanged()
{
    INotationPtr notation = context()->currentNotation();

    if (notation) {
        noteInput()->stateChanged().onNotify(this, [this]() {
            updateState();
        });

        interaction()->selectionChanged().onNotify(this, [this]() {
            updateState();
        });

        undoStack()->stackChanged().onNotify(this, [this]() {
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

void NoteInputBarModel::toggleNoteInputMethod(const ActionCode& actionCode)
{
    if (!noteInput()) {
        return;
    }

    if (!isNoteInputMode()) {
        noteInput()->startNoteInput();
    }

    noteInput()->toggleNoteInputMethod(noteInputModeActions[actionCode]);
}

void NoteInputBarModel::updateState()
{
    bool isPlaying = playbackController()->isPlaying();
    if (!notation() || isPlaying) {
        for (MenuItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (MenuItem& item : m_items) {
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
    updateArticulationsState();
    updateRestState();
}

void NoteInputBarModel::updateNoteInputModeState()
{
    int noteInputModeIndex = findNoteInputModeItemIndex();
    if (noteInputModeIndex == INVALID_INDEX) {
        return;
    }

    std::string currentSection = m_items[noteInputModeIndex].section;

    m_items[noteInputModeIndex] = makeActionItem(currentNoteInputModeAction(), currentSection);
    m_items[noteInputModeIndex].checked = isNoteInputMode();

    emit dataChanged(index(noteInputModeIndex), index(noteInputModeIndex));
}

void NoteInputBarModel::updateNoteDotState()
{
    static ActionCodeList dotActions = {
        "pad-dot",
        "pad-dotdot",
        "pad-dot3",
        "pad-dot4"
    };

    int durationDots = noteInputState().duration.dots();

    for (const ActionCode& actionCode: dotActions) {
        item(actionCode).checked = durationDots == NotationActions::actionDotCount(actionCode);
    }
}

void NoteInputBarModel::updateNoteDurationState()
{
    static ActionCodeList noteActions = {
        "note-longa",
        "note-breve",
        "pad-note-1",
        "pad-note-2",
        "pad-note-4",
        "pad-note-8",
        "pad-note-16",
        "pad-note-32",
        "pad-note-64",
        "pad-note-128",
        "pad-note-256",
        "pad-note-512",
        "pad-note-1024"
    };

    DurationType durationType = resolveCurrentDurationType();

    for (const ActionCode& actionCode: noteActions) {
        item(actionCode).checked = durationType == NotationActions::actionDurationType(actionCode);
    }
}

void NoteInputBarModel::updateNoteAccidentalState()
{
    static ActionCodeList accidentalActions = {
        "flat2",
        "flat",
        "nat",
        "sharp",
        "sharp2"
    };

    AccidentalType accidentalType = noteInputState().accidentalType;

    for (const ActionCode& actionCode: accidentalActions) {
        item(actionCode).checked = accidentalType == NotationActions::actionAccidentalType(actionCode);
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
    item("add-slur").checked = resolveSlurSelected();
}

void NoteInputBarModel::updateVoicesState()
{
    static ActionCodeList voiceActions {
        "voice-1",
        "voice-2",
        "voice-3",
        "voice-4"
    };

    int currentVoice = resolveCurrentVoiceIndex();

    for (const ActionCode& actionCode: voiceActions) {
        item(actionCode).checked = currentVoice == NotationActions::actionVoice(actionCode);
    }
}

void NoteInputBarModel::updateArticulationsState()
{
    static ActionCodeList articulationActions {
        "add-marcato",
        "add-sforzato",
        "add-tenuto",
        "add-staccato"
    };

    std::set<SymbolId> currentArticulations = resolveCurrentArticulations();

    auto isArticulationSelected = [&currentArticulations](SymbolId articulationSymbolId) {
        return std::find(currentArticulations.begin(), currentArticulations.end(),
                         articulationSymbolId) != currentArticulations.end();
    };

    for (const ActionCode& actionCode: articulationActions) {
        item(actionCode).checked = isArticulationSelected(NotationActions::actionArticulationSymbolId(actionCode));
    }
}

void NoteInputBarModel::updateRestState()
{
    item("pad-rest").checked = resolveRestSelected();
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
        return element->voice();
    }

    return INVALID_VOICE;
}

std::set<SymbolId> NoteInputBarModel::resolveCurrentArticulations() const
{
    if (!noteInput() || !selection()) {
        return {};
    }

    if (isNoteInputMode()) {
        return noteInputState().articulationIds;
    }

    if (selection()->isNone()) {
        return {};
    }

    auto chordArticulations = [](const Chord* chord) {
        std::set<SymbolId> result;
        for (Articulation* articulation: chord->articulations()) {
            result.insert(articulation->symId());
        }

        result = Ms::flipArticulations(result, Ms::Placement::ABOVE);
        return Ms::splitArticulations(result);
    };

    std::set<SymbolId> result;
    bool isFirstNote = true;
    for (const Element* element: selection()->elements()) {
        if (!element->isNote()) {
            continue;
        }

        const Note* note = dynamic_cast<const Note*>(element);
        if (isFirstNote) {
            result = chordArticulations(note->chord());
            isFirstNote = false;
        } else {
            std::set<SymbolId> currentNoteArticulations = chordArticulations(note->chord());
            for (auto it = result.begin(); it != result.end();) {
                if (std::find(currentNoteArticulations.begin(), currentNoteArticulations.end(),
                              *it) == currentNoteArticulations.end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    return result;
}

bool NoteInputBarModel::resolveRestSelected() const
{
    if (!noteInput() || !selection()) {
        return false;
    }

    if (isNoteInputMode()) {
        return noteInputState().isRest;
    }

    if (selection()->isNone() || selection()->isRange()) {
        return false;
    }

    for (const Element* element: selection()->elements()) {
        if (!element->isRest()) {
            return false;
        }
    }

    return true;
}

DurationType NoteInputBarModel::resolveCurrentDurationType() const
{
    constexpr DurationType INVALID_DURATION_TYPE = DurationType::V_INVALID;

    if (!noteInput() || !selection()) {
        return INVALID_DURATION_TYPE;
    }

    if (isNoteInputMode()) {
        return noteInputState().duration.type();
    }

    if (selection()->isNone() || selection()->isRange()) {
        return INVALID_DURATION_TYPE;
    }

    if (selection()->elements().empty()) {
        return INVALID_DURATION_TYPE;
    }

    DurationType result = INVALID_DURATION_TYPE;
    bool isFirstElement = true;
    for (const Element* element: selection()->elements()) {
        const ChordRest* chordRest = elementToChordRest(element);
        if (!chordRest) {
            continue;
        }

        if (isFirstElement) {
            result = chordRest->durationType().type();
            isFirstElement = false;
        } else if (result != chordRest->durationType().type()) {
            return INVALID_DURATION_TYPE;
        }
    }

    return result;
}

bool NoteInputBarModel::resolveSlurSelected() const
{
    if (!noteInput() || !selection()) {
        return false;
    }

    if (isNoteInputMode()) {
        return noteInputState().withSlur;
    }

    if (selection()->isNone() || selection()->isRange()) {
        return false;
    }

    if (selection()->elements().empty()) {
        return false;
    }

    for (const Element* element: selection()->elements()) {
        const ChordRest* chordRest = elementToChordRest(element);
        if (!chordRest) {
            continue;
        }

        Ms::Slur* slur = chordRest->slur();
        if (slur) {
            return true;
        }
    }

    return false;
}

bool NoteInputBarModel::isNoteInputModeAction(const ActionCode& actionCode) const
{
    return noteInputModeActions.contains(actionCode);
}

ActionItem NoteInputBarModel::currentNoteInputModeAction() const
{
    NoteInputMethod method = noteInputState().method;
    return actionsRegister()->action(noteInputModeActions.key(method));
}

int NoteInputBarModel::itemIndex(const ActionCode& actionCode) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].code == actionCode) {
            return i;
        }
    }

    return INVALID_INDEX;
}

MenuItem NoteInputBarModel::makeActionItem(const ActionItem& action, const std::string& section)
{
    MenuItem item = action;
    item.section = section;

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItem NoteInputBarModel::makeAddItem(const std::string& section)
{
    ActionItem addAction(ADD_ACTION_CODE, shortcuts::ShortcutContext::Undefined, ADD_ACTION_TITLE, ADD_ACTION_ICON_CODE);
    return makeActionItem(addAction, section);
}

QVariantList NoteInputBarModel::subitems(const ActionCode& actionCode) const
{
    MenuItemList items;
    if (isNoteInputModeAction(actionCode)) {
        items = noteInputMethodItems();
    } else if (actionCode == TUPLET_ACTION_CODE) {
        items = tupletItems();
    } else if (actionCode == ADD_ACTION_CODE) {
        items = addItems();
    }

    QVariantList result;
    for (const MenuItem& item: items) {
        result << item.toMap();
    }

    return result;
}

MenuItemList NoteInputBarModel::noteInputMethodItems() const
{
    MenuItemList items;

    for (const ActionCode& actionCode: noteInputModeActions.keys()) {
        MenuItem item = makeAction(actionCode);
        item.checkable = false;
        item.selectable = true;
        if (actionCode == currentNoteInputModeAction().code) {
            item.selected = true;
        }
        items.push_back(item);
    }

    return items;
}

MenuItemList NoteInputBarModel::tupletItems() const
{
    MenuItemList items = {
        makeAction("duplet"),
        makeAction("triplet"),
        makeAction("quadruplet"),
        makeAction("quintuplet"),
        makeAction("sextuplet"),
        makeAction("septuplet"),
        makeAction("octuplet"),
        makeAction("nonuplet"),
        makeAction("tuplet-dialog")
    };

    return items;
}

MenuItemList NoteInputBarModel::addItems() const
{
    MenuItemList items = {
        makeMenu(trc("notation", "Notes"), notesItems()),
        makeMenu(trc("notation", "Intervals"), intervalsItems()),
        makeMenu(trc("notation", "Measures"), measuresItems()),
        makeMenu(trc("notation", "Frames"), framesItems()),
        makeMenu(trc("notation", "Text"), textItems()),
        makeMenu(trc("notation", "Lines"), linesItems())
    };

    return items;
}

MenuItemList NoteInputBarModel::notesItems() const
{
    MenuItemList items {
        makeAction("note-input"),
        makeSeparator(),
        makeAction("note-c"),
        makeAction("note-d"),
        makeAction("note-e"),
        makeAction("note-f"),
        makeAction("note-g"),
        makeAction("note-a"),
        makeAction("note-b"),
        makeSeparator(),
        makeAction("chord-c"),
        makeAction("chord-d"),
        makeAction("chord-e"),
        makeAction("chord-f"),
        makeAction("chord-g"),
        makeAction("chord-a"),
        makeAction("chord-b")
    };

    return items;
}

MenuItemList NoteInputBarModel::intervalsItems() const
{
    MenuItemList items {
        makeAction("interval1"),
        makeAction("interval2"),
        makeAction("interval3"),
        makeAction("interval4"),
        makeAction("interval5"),
        makeAction("interval6"),
        makeAction("interval7"),
        makeAction("interval8"),
        makeAction("interval9"),
        makeSeparator(),
        makeAction("interval-2"),
        makeAction("interval-3"),
        makeAction("interval-4"),
        makeAction("interval-5"),
        makeAction("interval-6"),
        makeAction("interval-7"),
        makeAction("interval-8"),
        makeAction("interval-9")
    };

    return items;
}

MenuItemList NoteInputBarModel::measuresItems() const
{
    MenuItemList items {
        makeAction("insert-measure"),
        makeAction("insert-measures"),
        makeSeparator(),
        makeAction("append-measure"),
        makeAction("append-measures")
    };

    return items;
}

MenuItemList NoteInputBarModel::framesItems() const
{
    MenuItemList items {
        makeAction("insert-hbox"),
        makeAction("insert-vbox"),
        makeAction("insert-textframe"),
        makeSeparator(),
        makeAction("append-hbox"),
        makeAction("append-vbox"),
        makeAction("append-textframe")
    };

    return items;
}

MenuItemList NoteInputBarModel::textItems() const
{
    MenuItemList items {
        makeAction("title-text"),
        makeAction("subtitle-text"),
        makeAction("composer-text"),
        makeAction("poet-text"),
        makeAction("part-text"),
        makeSeparator(),
        makeAction("system-text"),
        makeAction("staff-text"),
        makeAction("expression-text"),
        makeAction("rehearsalmark-text"),
        makeAction("instrument-change-text"),
        makeAction("fingering-text"),
        makeSeparator(),
        makeAction("sticking-text"),
        makeAction("chord-text"),
        makeAction("roman-numeral-text"),
        makeAction("nashville-number-text"),
        makeAction("lyrics"),
        makeAction("figured-bass"),
        makeAction("tempo")
    };

    return items;
}

MenuItemList NoteInputBarModel::linesItems() const
{
    MenuItemList items {
        makeAction("add-slur"),
        makeAction("add-hairpin"),
        makeAction("add-hairpin-reverse"),
        makeAction("add-8va"),
        makeAction("add-8vb"),
        makeAction("add-noteline")
    };

    return items;
}

bool NoteInputBarModel::isNeedShowSubitemsByPressAndHold(const ActionCode& actionCode) const
{
    if (isNoteInputModeAction(actionCode)) {
        return true;
    }

    return false;
}

void NoteInputBarModel::notifyAboutTupletItemChanged()
{
    int tupletItemIndex = itemIndex(TUPLET_ACTION_CODE);
    if (tupletItemIndex == INVALID_INDEX) {
        return;
    }

    emit dataChanged(index(tupletItemIndex), index(tupletItemIndex));
}

void NoteInputBarModel::notifyAboutAddItemChanged()
{
    int addItemIndex = itemIndex(ADD_ACTION_CODE);
    if (addItemIndex == INVALID_INDEX) {
        return;
    }

    emit dataChanged(index(addItemIndex), index(addItemIndex));
}

std::vector<std::string> NoteInputBarModel::currentWorkspaceActions() const
{
    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret) {
        LOGE() << workspace.ret.toString();
        return {};
    }

    AbstractDataPtr abstractData = workspace.val->data(WorkspaceTag::Toolbar, TOOLBAR_NAME);
    ToolbarDataPtr toolbarData = std::dynamic_pointer_cast<ToolbarData>(abstractData);
    if (!toolbarData) {
        LOGE() << "Failed to get data of actions for " << TOOLBAR_NAME;
        return {};
    }

    return toolbarData->actions;
}

void NoteInputBarModel::handleAction(const QString& action, int actionIndex)
{
    ActionCode actionCode = codeFromQString(action);
    if (isNoteInputModeAction(actionCode)) {
        if (actionIndex != INVALID_INDEX) {
            toggleNoteInputMethod(actionCode);
        } else {
            toggleNoteInput();
        }
        return;
    }

    dispatcher()->dispatch(actionCode);
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

INotationUndoStackPtr NoteInputBarModel::undoStack() const
{
    return notation() ? notation()->undoStack() : nullptr;
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

const ChordRest* NoteInputBarModel::elementToChordRest(const Element* element) const
{
    if (!element) {
        return nullptr;
    }
    if (element->isChordRest()) {
        return toChordRest(element);
    }
    if (element->isNote()) {
        return toNote(element)->chord();
    }
    if (element->isStem()) {
        return toStem(element)->chord();
    }
    if (element->isHook()) {
        return toHook(element)->chord();
    }
    return nullptr;
}
