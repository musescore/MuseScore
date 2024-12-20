/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "async/notifylist.h"
#include "types/translatablestring.h"

#include "context/shortcutcontext.h"
#include "internal/notationuiactions.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;

static const QString TOOLBAR_NAME("noteInput");

static const ActionCode ADD_ACTION_CODE("add");
static const TranslatableString ADD_ACTION_TITLE = TranslatableString("notation", "Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;

static const ActionCode CROSS_STAFF_BEAMING_CODE("cross-staff-beaming");
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
    uiConfiguration()->toolConfigChanged(TOOLBAR_NAME).onNotify(this, [this]() {
        load();
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });

    MenuItemList items;

    ToolConfig noteInputConfig = uiConfiguration()->toolConfig(TOOLBAR_NAME, NotationUiActions::defaultNoteInputBarConfig());

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
        } else if (citem.action == CROSS_STAFF_BEAMING_CODE) {
            subitems = makeCrossStaffBeamingItems();
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

bool NoteInputBarModel::isInputAllowed() const
{
    auto currentMasterNotation = masterNotation();
    return currentMasterNotation != nullptr && currentMasterNotation->hasParts();
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

        masterNotation()->hasPartsChanged().onNotify(this, [this]() {
            emit isInputAllowedChanged();
        });
    }

    updateState();

    emit isInputAllowedChanged();
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

void NoteInputBarModel::updateState()
{
    for (int i = 0; i < rowCount(); ++i) {
        MenuItem& item = this->item(i);
        UiActionState state = item.state();
        state.checked = false;
        item.setState(state);
    }

    if (isInputAllowed()) {
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
    updateLvState();
    updateSlurState();
    updateVoicesState();
    updateArticulationsState();
    updateRestState();
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
        if (note->laissezVib()) {
            checked = false;
            break;
        }
    }

    updateItemStateChecked(&findItem(codeFromQString("tie")), checked); // todo
}

void NoteInputBarModel::updateLvState()
{
    if (!selection()) {
        return;
    }

    std::vector<Note*> tiedNotes = selection()->notes(NoteFilter::WithTie);

    bool checked = !tiedNotes.empty();
    for (const Note* note: tiedNotes) {
        if (!note->laissezVib()) {
            checked = false;
            break;
        }
    }

    updateItemStateChecked(&findItem(codeFromQString("lv")), checked);
}

void NoteInputBarModel::updateSlurState()
{
    bool checked = notation() ? notation()->elements()->msScore()->inputState().slur() != nullptr : false;
    updateItemStateChecked(&findItem(codeFromQString("add-slur")), checked);
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

void NoteInputBarModel::updateAddState()
{
    findItem(ADD_ACTION_CODE).setSubitems(makeAddItems());
}

int NoteInputBarModel::resolveCurrentVoiceIndex() const
{
    constexpr int INVALID_VOICE = -1;

    if (!noteInput() || !selection()) {
        return INVALID_VOICE;
    }

    if (isNoteInputMode()) {
        return static_cast<int>(noteInputState().currentVoiceIndex);
    }

    if (selection()->isNone()) {
        return INVALID_VOICE;
    }

    const std::vector<EngravingItem*>& selectedElements = selection()->elements();
    if (selectedElements.empty()) {
        return INVALID_VOICE;
    }

    int voice = INVALID_VOICE;
    for (const EngravingItem* element : selectedElements) {
        if (element->hasVoiceAssignmentProperties()) {
            VoiceAssignment voiceAssignment = element->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
            if (voiceAssignment == VoiceAssignment::ALL_VOICE_IN_INSTRUMENT || voiceAssignment == VoiceAssignment::ALL_VOICE_IN_STAFF) {
                return INVALID_VOICE;
            }
        }
        int elementVoice = static_cast<int>(element->voice());
        if (elementVoice != voice && voice != INVALID_VOICE) {
            return INVALID_VOICE;
        }

        voice = static_cast<int>(element->voice());
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

        result = mu::engraving::flipArticulations(result, mu::engraving::PlacementV::ABOVE);
        return mu::engraving::splitArticulations(result);
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

    const std::vector<EngravingItem*>& selectedElements = selection()->elements();
    if (selectedElements.empty()) {
        return INVALID_DURATION_TYPE;
    }

    DurationType result = INVALID_DURATION_TYPE;
    bool isFirstElement = true;
    for (const EngravingItem* element: selectedElements) {
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

bool NoteInputBarModel::isNoteInputModeAction(const ActionCode& actionCode) const
{
    return actionCode == "note-input" || noteInputMethodForActionCode(actionCode) != NoteInputMethod::UNKNOWN;
}

UiAction NoteInputBarModel::currentNoteInputModeAction() const
{
    NoteInputMethod method = noteInputState().method;
    UiAction action = uiActionsRegister()->action(actionCodeForNoteInputMethod(method));
    std::vector<std::string> genericShortcuts = uiActionsRegister()->action("note-input").shortcuts;
    action.shortcuts = genericShortcuts;
    return action;
}

MenuItem* NoteInputBarModel::makeActionItem(const UiAction& action, const QString& section,
                                            const muse::uicomponents::MenuItemList& subitems)
{
    MenuItem* item = new MenuItem(action, this);
    item->setSection(section);
    item->setSubitems(subitems);
    return item;
}

MenuItem* NoteInputBarModel::makeAddItem(const QString& section)
{
    static const UiAction addAction(ADD_ACTION_CODE, UiCtxAny, mu::context::CTX_ANY, ADD_ACTION_TITLE, ADD_ACTION_ICON_CODE);
    return makeActionItem(addAction, section, makeAddItems());
}

MenuItemList NoteInputBarModel::makeSubitems(const ActionCode& actionCode)
{
    MenuItemList items;
    if (isNoteInputModeAction(actionCode)) {
        items = makeNoteInputMethodItems();
    } else if (actionCode == CROSS_STAFF_BEAMING_CODE) {
        items = makeCrossStaffBeamingItems();
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
    ActionCode currentInputMethod = currentNoteInputModeAction().code;

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

MenuItemList NoteInputBarModel::makeCrossStaffBeamingItems()
{
    MenuItemList items = {
        makeMenuItem("move-up"),
        makeMenuItem("move-down")
    };

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
        makeMenu(TranslatableString("notation", "Notes"), makeNotesItems()),
        makeMenu(TranslatableString("notation", "Intervals"), makeIntervalsItems()),
        makeMenu(TranslatableString("notation", "Measures"), makeMeasuresItems()),
        makeMenu(TranslatableString("notation", "Frames"), makeFramesItems()),
        makeMenu(TranslatableString("notation", "Text"), makeTextItems()),
        makeMenu(TranslatableString("notation", "Lines"), makeLinesItems())
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
        makeMenuItem("dynamics"),
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

IMasterNotationPtr NoteInputBarModel::masterNotation() const
{
    return context()->currentMasterNotation();
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
