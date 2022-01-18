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
#include "noteinputbarmodel.h"

#include "log.h"
#include "translation.h"

#include "internal/notationuiactions.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::ui;
using namespace mu::uicomponents;

static const QString TOOLBAR_NAME("noteInput");

static const std::string ADD_ACTION_CODE("add");
static const char* ADD_ACTION_TITLE("Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;

static const ActionCode TUPLET_ACTION_CODE("tuplet");

static const std::vector<std::pair<ActionCode, NoteInputMethod> > noteInputModeActions = {
    { "note-input-steptime", NoteInputMethod::STEPTIME },
    { "note-input-rhythm", NoteInputMethod::RHYTHM },
    { "note-input-repitch", NoteInputMethod::REPITCH },
    { "note-input-realtime-auto", NoteInputMethod::REALTIME_AUTO },
    { "note-input-realtime-manual", NoteInputMethod::REALTIME_MANUAL },
    { "note-input-timewise", NoteInputMethod::TIMEWISE },
};

static NoteInputMethod noteInputMethodForActionCode(const ActionCode& code)
{
    for (const auto& pair : noteInputModeActions) {
        if (pair.first == code) {
            return pair.second;
        }
    }

    return NoteInputMethod::UNKNOWN;
}

static ActionCode actionCodeForNoteInputMethod(NoteInputMethod method)
{
    for (const auto& pair : noteInputModeActions) {
        if (pair.second == method) {
            return pair.first;
        }
    }

    return {};
}

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : AbstractMenuModel(parent)
{
    uiConfiguration()->toolConfigChanged(TOOLBAR_NAME).onNotify(this, [this]() {
        load();
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    TRACEFUNC;
    int row = index.row();

    if (!isIndexValid(row)) {
        return QVariant();
    }

    const MenuItem* item = items()[row];
    switch (role) {
    case IsMenuSecondaryRole: return isMenuSecondary(item->action().code);
    case OrderRole: return row;
    case SectionRole: return item->section();
    default: return AbstractMenuModel::data(index, role);
    }
}

QHash<int, QByteArray> NoteInputBarModel::roleNames() const
{
    QHash<int, QByteArray> roles = AbstractMenuModel::roleNames();
    roles[IsMenuSecondaryRole] = "isMenuSecondary";
    roles[OrderRole] = "order";
    roles[SectionRole] = "section";

    return roles;
}

void NoteInputBarModel::load()
{
    MenuItemList items;

    ToolConfig noteInputConfig = uiConfiguration()->toolConfig(TOOLBAR_NAME);
    if (!noteInputConfig.isValid()) {
        noteInputConfig = NotationUiActions::defaultNoteInputBarConfig();
    }

    int section = 0;
    for (const ToolConfig::Item& citem : noteInputConfig.items) {
        if (!citem.show) {
            continue;
        }

        if (citem.action.empty()) {
            section++;
            continue;
        }

        MenuItemList subitems;
        if (isNoteInputModeAction(citem.action)) {
            subitems = makeNoteInputMethodItems();
        } else if (citem.action == TUPLET_ACTION_CODE) {
            subitems = makeTupletItems();
        }

        MenuItem* item = makeActionItem(uiActionsRegister()->action(citem.action), QString::number(section), subitems);

        items << item;
    }

    items << makeAddItem(QString::number(++section));
    setItems(items);

    onNotationChanged();
    AbstractMenuModel::load();
}

int NoteInputBarModel::findNoteInputModeItemIndex() const
{
    const MenuItemList& items = this->items();

    for (int i = 0; i < items.size(); i++) {
        if (isNoteInputModeAction(items[i]->action().code)) {
            return i;
        }
    }

    return INVALID_ITEM_INDEX;
}

void NoteInputBarModel::onNotationChanged()
{
    if (context()->currentNotation()) {
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

void NoteInputBarModel::updateItemStateChecked(MenuItem* item, bool checked)
{
    if (!item) {
        return;
    }

    UiActionState state = item->state();
    state.checked = checked;
    item->setState(state);
}

void NoteInputBarModel::updateItemStateEnabled(MenuItem* item, bool enabled)
{
    if (!item) {
        return;
    }

    UiActionState state = item->state();
    state.enabled = enabled;
    item->setState(state);
}

void NoteInputBarModel::updateState()
{
    bool enabled = notation() && !playbackController()->isPlaying();

    for (int i = 0; i < rowCount(); ++i) {
        MenuItem& item = this->item(i);
        UiActionState state = item.state();
        state.checked = false;
        state.enabled = enabled;
        item.setState(state);
    }

    if (enabled) {
        updateNoteInputState();
    }
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
    updateTupletState();
    updateAddState();
}

void NoteInputBarModel::updateNoteInputModeState()
{
    int noteInputModeIndex = findNoteInputModeItemIndex();
    if (noteInputModeIndex == INVALID_ITEM_INDEX) {
        return;
    }

    MenuItem& item = this->item(noteInputModeIndex);

    QString currentSection = item.section();
    MenuItemList subitems = makeNoteInputMethodItems();

    item.setAction(currentNoteInputModeAction());
    item.setSection(currentSection);
    item.setSubitems(subitems);

    updateItemStateChecked(&item, isNoteInputMode());
}

void NoteInputBarModel::updateNoteDotState()
{
    static const ActionCodeList dotActions = {
        "pad-dot",
        "pad-dot2",
        "pad-dot3",
        "pad-dot4"
    };

    int durationDots = noteInputState().duration.dots();

    for (const ActionCode& actionCode: dotActions) {
        updateItemStateChecked(&findItem(actionCode), durationDots == NotationUiActions::actionDotCount(actionCode));
    }
}

void NoteInputBarModel::updateNoteDurationState()
{
    static const ActionCodeList noteActions = {
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
        updateItemStateChecked(&findItem(actionCode), durationType == NotationUiActions::actionDurationType(actionCode));
    }
}

void NoteInputBarModel::updateNoteAccidentalState()
{
    static const ActionCodeList accidentalActions = {
        "flat2",
        "flat",
        "nat",
        "sharp",
        "sharp2"
    };

    AccidentalType accidentalType = noteInputState().accidentalType;

    for (const ActionCode& actionCode: accidentalActions) {
        updateItemStateChecked(&findItem(actionCode), accidentalType == NotationUiActions::actionAccidentalType(actionCode));
    }
}

void NoteInputBarModel::updateTieState()
{
    if (!selection()) {
        return;
    }

    std::vector<Note*> tiedNotes = selection()->notes(NoteFilter::WithTie);

    bool checked = !tiedNotes.empty();
    for (const Note* note: tiedNotes) {
        if (!note->tieFor()) {
            checked = false;
            break;
        }
    }

    updateItemStateChecked(&findItem(codeFromQString("tie")), checked); // todo
}

void NoteInputBarModel::updateSlurState()
{
    updateItemStateChecked(&findItem(codeFromQString("add-slur")), notation()->elements()->msScore()->inputState().slur() != nullptr);
}

void NoteInputBarModel::updateVoicesState()
{
    static const ActionCodeList voiceActions {
        "voice-1",
        "voice-2",
        "voice-3",
        "voice-4"
    };

    int currentVoice = resolveCurrentVoiceIndex();

    for (const ActionCode& actionCode: voiceActions) {
        updateItemStateChecked(&findItem(actionCode), currentVoice == NotationUiActions::actionVoice(actionCode));
    }
}

void NoteInputBarModel::updateArticulationsState()
{
    static const ActionCodeList articulationActions {
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
        updateItemStateChecked(&findItem(actionCode), isArticulationSelected(NotationUiActions::actionArticulationSymbolId(actionCode)));
    }
}

void NoteInputBarModel::updateRestState()
{
    updateItemStateChecked(&findItem(ActionCode("pad-rest")), resolveRestSelected());
}

void NoteInputBarModel::updateTupletState()
{
    updateItemStateEnabled(&findItem(ActionCode(TUPLET_ACTION_CODE)), resolveTupletEnabled());
}

void NoteInputBarModel::updateAddState()
{
    findItem(ActionCode(ADD_ACTION_CODE)).setSubitems(makeAddItems());
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

    std::vector<EngravingItem*> selectedElements = selection()->elements();
    if (selectedElements.empty()) {
        return INVALID_VOICE;
    }

    int voice = INVALID_VOICE;
    for (const EngravingItem* element : selection()->elements()) {
        int elementVoice = element->voice();
        if (elementVoice != voice && voice != INVALID_VOICE) {
            return INVALID_VOICE;
        }

        voice = element->voice();
    }

    return voice;
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

        result = Ms::flipArticulations(result, Ms::PlacementV::ABOVE);
        return Ms::splitArticulations(result);
    };

    std::set<SymbolId> result;
    bool isFirstNote = true;
    for (const EngravingItem* element: selection()->elements()) {
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

    for (const EngravingItem* element: selection()->elements()) {
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
    for (const EngravingItem* element: selection()->elements()) {
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

bool NoteInputBarModel::resolveTupletEnabled() const
{
    if (isNoteInputMode()) {
        return true;
    }

    if (!selection()) {
        return false;
    }

    for (const EngravingItem* element: selection()->elements()) {
        if (element->isRest() || element->isNote()) {
            return true;
        }
    }

    return false;
}

bool NoteInputBarModel::isNoteInputModeAction(const ActionCode& actionCode) const
{
    return actionCode == "note-input" || noteInputMethodForActionCode(actionCode) != NoteInputMethod::UNKNOWN;
}

UiAction NoteInputBarModel::currentNoteInputModeAction() const
{
    NoteInputMethod method = noteInputState().method;
    return uiActionsRegister()->action(actionCodeForNoteInputMethod(method));
}

MenuItem* NoteInputBarModel::makeActionItem(const UiAction& action, const QString& section, const uicomponents::MenuItemList& subitems)
{
    MenuItem* item = new MenuItem(action, this);
    item->setSection(section);
    item->setSubitems(subitems);
    return item;
}

MenuItem* NoteInputBarModel::makeAddItem(const QString& section)
{
    UiAction addAction(ADD_ACTION_CODE, UiCtxAny, ADD_ACTION_TITLE, ADD_ACTION_ICON_CODE);
    return makeActionItem(addAction, section, makeAddItems());
}

MenuItemList NoteInputBarModel::makeSubitems(const ActionCode& actionCode)
{
    MenuItemList items;
    if (isNoteInputModeAction(actionCode)) {
        items = makeNoteInputMethodItems();
    } else if (actionCode == TUPLET_ACTION_CODE) {
        items = makeTupletItems();
    } else if (actionCode == ADD_ACTION_CODE) {
        items = makeAddItems();
    }

    return items;
}

MenuItemList NoteInputBarModel::makeNoteInputMethodItems()
{
    MenuItemList items;
    actions::ActionCode currentInputMethod = currentNoteInputModeAction().code;

    for (const auto& pair : noteInputModeActions) {
        ActionCode actionCode = pair.first;
        MenuItem* item = makeMenuItem(actionCode);
        item->setSelectable(true);

        if (actionCode == currentInputMethod) {
            item->setSelected(true);
        }

        items.push_back(item);
    }

    return items;
}

MenuItemList NoteInputBarModel::makeTupletItems()
{
    MenuItemList items = {
        makeMenuItem("duplet"),
        makeMenuItem("triplet"),
        makeMenuItem("quadruplet"),
        makeMenuItem("quintuplet"),
        makeMenuItem("sextuplet"),
        makeMenuItem("septuplet"),
        makeMenuItem("octuplet"),
        makeMenuItem("nonuplet"),
        makeMenuItem("tuplet-dialog")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeAddItems()
{
    MenuItemList items = {
        makeMenu(qtrc("notation", "Notes"), makeNotesItems()),
        makeMenu(qtrc("notation", "Intervals"), makeIntervalsItems()),
        makeMenu(qtrc("notation", "Measures"), makeMeasuresItems()),
        makeMenu(qtrc("notation", "Frames"), makeFramesItems()),
        makeMenu(qtrc("notation", "Text"), makeTextItems()),
        makeMenu(qtrc("notation", "Lines"), makeLinesItems())
    };

    return items;
}

MenuItemList NoteInputBarModel::makeNotesItems()
{
    MenuItemList items {
        makeMenuItem("note-c"),
        makeMenuItem("note-d"),
        makeMenuItem("note-e"),
        makeMenuItem("note-f"),
        makeMenuItem("note-g"),
        makeMenuItem("note-a"),
        makeMenuItem("note-b"),
        makeSeparator(),
        makeMenuItem("chord-c"),
        makeMenuItem("chord-d"),
        makeMenuItem("chord-e"),
        makeMenuItem("chord-f"),
        makeMenuItem("chord-g"),
        makeMenuItem("chord-a"),
        makeMenuItem("chord-b")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem("interval1"),
        makeMenuItem("interval2"),
        makeMenuItem("interval3"),
        makeMenuItem("interval4"),
        makeMenuItem("interval5"),
        makeMenuItem("interval6"),
        makeMenuItem("interval7"),
        makeMenuItem("interval8"),
        makeMenuItem("interval9"),
        makeSeparator(),
        makeMenuItem("interval-2"),
        makeMenuItem("interval-3"),
        makeMenuItem("interval-4"),
        makeMenuItem("interval-5"),
        makeMenuItem("interval-6"),
        makeMenuItem("interval-7"),
        makeMenuItem("interval-8"),
        makeMenuItem("interval-9")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measure"),
        makeMenuItem("insert-measures"),
        makeSeparator(),
        makeMenuItem("append-measure"),
        makeMenuItem("append-measures")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem("insert-hbox"),
        makeMenuItem("insert-vbox"),
        makeMenuItem("insert-textframe"),
        makeSeparator(),
        makeMenuItem("append-hbox"),
        makeMenuItem("append-vbox"),
        makeMenuItem("append-textframe")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem("title-text"),
        makeMenuItem("subtitle-text"),
        makeMenuItem("composer-text"),
        makeMenuItem("poet-text"),
        makeMenuItem("part-text"),
        makeSeparator(),
        makeMenuItem("system-text"),
        makeMenuItem("staff-text"),
        makeMenuItem("expression-text"),
        makeMenuItem("rehearsalmark-text"),
        makeMenuItem("instrument-change-text"),
        makeMenuItem("fingering-text"),
        makeSeparator(),
        makeMenuItem("sticking-text"),
        makeMenuItem("chord-text"),
        makeMenuItem("roman-numeral-text"),
        makeMenuItem("nashville-number-text"),
        makeMenuItem("lyrics"),
        makeMenuItem("figured-bass"),
        makeMenuItem("tempo")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem("add-slur"),
        makeMenuItem("add-hairpin"),
        makeMenuItem("add-hairpin-reverse"),
        makeMenuItem("add-8va"),
        makeMenuItem("add-8vb"),
        makeMenuItem("add-noteline")
    };

    return items;
}

bool NoteInputBarModel::isMenuSecondary(const ActionCode& actionCode) const
{
    if (isNoteInputModeAction(actionCode)) {
        return true;
    }

    return false;
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

const ChordRest* NoteInputBarModel::elementToChordRest(const EngravingItem* element) const
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
