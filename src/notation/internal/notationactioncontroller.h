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
#ifndef MU_NOTATION_NOTATIONACTIONCONTROLLER_H
#define MU_NOTATION_NOTATIONACTIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "inotation.h"
#include "iinteractive.h"
#include "audio/isequencer.h"

namespace mu::notation {
class NotationActionController : public actions::Actionable
{
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, framework::IInteractive, interactive)
    INJECT(notation, audio::ISequencer, sequencer)

public:
    void init();

private:
    bool canReceiveAction(const actions::ActionCode& actionCode) const override;

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;
    INotationElementsPtr currentNotationElements() const;
    INotationSelectionPtr currentNotationSelection() const;
    INotationNoteInputPtr currentNotationNoteInput() const;

    void resetState();

    void toggleNoteInputMethod(NoteInputMethod method);
    void addNote(NoteName note, NoteAddingMode addingMode);
    void addText(TextType type);
    void padNote(const Pad& pad);
    void putNote(const actions::ActionData& data);

    void toggleAccidental(AccidentalType type);
    void addArticulation(SymbolId articulationSymbolId);

    void putTuplet(int tupletCount);

    void moveAction(const actions::ActionCode& actionCode);
    void moveText(INotationInteractionPtr interaction, const actions::ActionCode& actionCode);

    void swapVoices(int voiceIndex1, int voiceIndex2);
    void changeVoice(int voiceIndex);

    void cutSelection();
    void copySelection();
    void deleteSelection();
    void swapSelection();
    void flipSelection();
    void addTie();
    void addSlur();
    void addInterval(int interval);

    void undo();
    void redo();

    void selectAllSimilarElements();
    void selectAllSimilarElementsInStaff();
    void selectAllSimilarElementsInRange();
    void openSelectionMoreOptions();
    void selectAll();

    void splitMeasure();
    void joinSelectedMeasures();
    void selectMeasuresCountAndInsert();
    void selectMeasuresCountAndAppend();

    void insertBox(BoxType boxType);
    void appendBox(BoxType boxType);
    void insertBoxes(BoxType boxType, int count);
    void appendBoxes(BoxType boxType, int count);
    int firstSelectedBoxIndex() const;

    void addOttava(OttavaType type);
    void addHairpin(HairpinType type);
    void addAnchoredNoteLine();

    void openPageStyle();
    void openStaffProperties();
    void openBreaksDialog();
    void openScoreProperties();
    void openTransposeDialog();
    void openPartsDialog();
    void openTupletOtherDialog();

    bool isTextEditting() const;

    enum class PastingType {
        Default,
        Half,
        Double,
        Special
    };

    void pasteSelection(PastingType type = PastingType::Default);
    Fraction resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const;

    FilterElementsOptions elementsFilterOptions(const Element* element) const;

    void startNoteInputIfNeed();
};
}

#endif // MU_NOTATION_NOTATIONACTIONCONTROLLER_H
