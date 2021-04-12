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
#ifndef MU_NOTATION_INOTATIONINTERACTION_H
#define MU_NOTATION_INOTATIONINTERACTION_H

#include <QPointF>
#include <QKeyEvent>
#include <functional>

#include "async/notification.h"

#include "notationtypes.h"
#include "inotationnoteinput.h"
#include "inotationselection.h"

namespace mu::notation {
class INotationInteraction
{
public:
    virtual ~INotationInteraction() = default;

    // Put notes
    virtual INotationNoteInputPtr noteInput() const = 0;

    // Shadow note
    virtual void showShadowNote(const QPointF& pos) = 0;
    virtual void hideShadowNote() = 0;

    // Visibility
    virtual void toggleVisible() = 0;

    // Select
    virtual Element* hitElement(const QPointF& pos, float width) const = 0;
    virtual int hitStaffIndex(const QPointF& pos) const = 0;
    virtual void addChordToSelection(MoveDirection d) = 0;
    virtual void select(const std::vector<Element*>& elements, SelectType type, int staffIndex = 0) = 0;
    virtual void selectAll() = 0;
    virtual void selectSection() = 0;
    virtual INotationSelectionPtr selection() const = 0;
    virtual void clearSelection() = 0;
    virtual async::Notification selectionChanged() const = 0;

    // Drag
    using IsDraggable = std::function<bool (const Element*)>;
    virtual bool isDragStarted() const = 0;
    virtual void startDrag(const std::vector<Element*>& elems, const QPointF& eoffset, const IsDraggable& isDrag) = 0;
    virtual void drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode) = 0;
    virtual void endDrag() = 0;
    virtual async::Notification dragChanged() const = 0;

    // Drop
    //! TODO Change KeyboardModifiers to modes
    virtual void startDrop(const QByteArray& edata) = 0;
    virtual bool isDropAccepted(const QPointF& pos, Qt::KeyboardModifiers modifiers) = 0; //! NOTE Also may set drop target
    virtual bool drop(const QPointF& pos, Qt::KeyboardModifiers modifiers) = 0;
    virtual void endDrop() = 0;
    virtual async::Notification dropChanged() const = 0;

    virtual bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) = 0;

    // Move
    //! NOTE Perform operations on selected elements
    virtual void moveSelection(MoveDirection d, MoveSelectionType type) = 0;
    virtual void movePitch(MoveDirection d, PitchMode mode) = 0;  //! NOTE Requires a note to be selected
    virtual void moveText(MoveDirection d, bool quickly) = 0;     //! NOTE Requires a text element to be selected

    // Text edit
    virtual bool isTextEditingStarted() const = 0;
    virtual void startEditText(Element* element, const QPointF& elementPos) = 0;
    virtual void editText(QKeyEvent* event) = 0;
    virtual void endEditText() = 0;
    virtual void changeTextCursorPosition(const QPointF& newCursorPos) = 0;
    virtual async::Notification textEditingStarted() const = 0;
    virtual async::Notification textEditingChanged() const = 0;

    // Grip edit
    virtual bool isGripEditStarted() const = 0;
    virtual bool isHitGrip(const QPointF& pos) const = 0;
    virtual void startEditGrip(const QPointF& pos) = 0;
    virtual void endEditGrip() = 0;

    virtual void splitSelectedMeasure() = 0;
    virtual void joinSelectedMeasures() = 0;

    virtual void addBoxes(BoxType boxType, int count, int beforeBoxIndex = -1) = 0;

    virtual void copySelection() = 0;
    virtual void copyLyrics() = 0;
    virtual void pasteSelection(const Fraction& scale = Fraction(1, 1)) = 0;
    virtual void swapSelection() = 0;
    virtual void deleteSelection() = 0;
    virtual void flipSelection() = 0;
    virtual void addTieToSelection() = 0;
    virtual void addSlurToSelection() = 0;
    virtual void addOttavaToSelection(OttavaType type) = 0;
    virtual void addHairpinToSelection(HairpinType type) = 0;
    virtual void addAccidentalToSelection(AccidentalType type) = 0;
    virtual void addBracketsToSelection(BracketsType type) = 0;
    virtual void changeSelectedNotesArticulation(SymbolId articulationSymbolId) = 0;
    virtual void addGraceNotesToSelectedNotes(GraceNoteType type) = 0;
    virtual void addTupletToSelectedChordRests(const TupletOptions& options) = 0;
    virtual void addBeamToSelectedChordRests(BeamMode mode) = 0;

    virtual void setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval = 0) = 0;
    virtual void transpose(const TransposeOptions& options) = 0;
    virtual void swapVoices(int voiceIndex1, int voiceIndex2) = 0;
    virtual void addIntervalToSelectedNotes(int interval) = 0;
    virtual void changeSelectedNotesVoice(int voiceIndex) = 0;
    virtual void addAnchoredLineToSelectedNotes() = 0;

    virtual void addText(TextType type) = 0;
    virtual void addFiguredBass() = 0;

    virtual void addStretch(qreal value) = 0;

    virtual void explodeSelectedStaff() = 0;
    virtual void implodeSelectedStaff() = 0;

    virtual void realizeSelectedChordSymbols() = 0;
    virtual void removeSelectedRange() = 0;
    virtual void removeEmptyTrailingMeasures() = 0;

    virtual void fillSelectionWithSlashes() = 0;
    virtual void replaceSelectedNotesWithSlashes() = 0;

    virtual void spellPitches() = 0;
    virtual void regroupNotesAndRests() = 0;
    virtual void resequenceRehearsalMarks() = 0;
    virtual void unrollRepeats() = 0;

    virtual void resetToDefault(ResettableValueType type) = 0;

    virtual ScoreConfig scoreConfig() const = 0;
    virtual void setScoreConfig(ScoreConfig config) = 0;
};

using INotationInteractionPtr = std::shared_ptr<INotationInteraction>;
}

#endif // MU_NOTATION_INOTATIONINTERACTION_H
