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
#ifndef MU_NOTATION_INOTATIONINTERACTION_H
#define MU_NOTATION_INOTATIONINTERACTION_H

#include <functional>

#include "async/notification.h"

#include "notationtypes.h"
#include "inotationnoteinput.h"
#include "inotationselection.h"
#include "actions/actiontypes.h"

class QKeyEvent;
class QInputMethodEvent;

namespace mu::notation {
class INotationInteraction
{
public:
    virtual ~INotationInteraction() = default;

    // Put notes
    virtual INotationNoteInputPtr noteInput() const = 0;

    // Shadow note
    virtual void showShadowNote(const PointF& pos) = 0;
    virtual void hideShadowNote() = 0;

    // Visibility
    virtual void toggleVisible() = 0;

    // Hit
    virtual EngravingItem* hitElement(const PointF& pos, float width) const = 0;
    virtual Staff* hitStaff(const PointF& pos) const = 0;

    struct HitElementContext
    {
        notation::EngravingItem* element = nullptr;
        notation::Staff* staff = nullptr;
    };

    virtual const HitElementContext& hitElementContext() const = 0;
    virtual void setHitElementContext(const HitElementContext& context) = 0;

    // Select
    virtual void moveChordNoteSelection(MoveDirection d) = 0;
    virtual void select(const std::vector<EngravingItem*>& elements, SelectType type = SelectType::REPLACE,
                        engraving::staff_idx_t staffIndex = 0) = 0;
    virtual void selectAll() = 0;
    virtual void selectSection() = 0;
    virtual void selectFirstElement(bool frame = true) = 0;
    virtual void selectLastElement() = 0;
    virtual INotationSelectionPtr selection() const = 0;
    virtual void clearSelection() = 0;
    virtual async::Notification selectionChanged() const = 0;
    virtual void selectTopOrBottomOfChord(MoveDirection d) = 0;

    // SelectionFilter
    virtual bool isSelectionTypeFiltered(SelectionFilterType type) const = 0;
    virtual void setSelectionTypeFiltered(SelectionFilterType type, bool filtered) = 0;

    // Drag
    using IsDraggable = std::function<bool (const EngravingItem*)>;
    virtual bool isDragStarted() const = 0;
    virtual void startDrag(const std::vector<EngravingItem*>& elems, const PointF& eoffset, const IsDraggable& isDrag) = 0;
    virtual void drag(const PointF& fromPos, const PointF& toPos, DragMode mode) = 0;
    virtual void endDrag() = 0;
    virtual async::Notification dragChanged() const = 0;

    virtual bool isDragCopyStarted() const = 0;
    virtual void startDragCopy(const EngravingItem* element, QObject* dragSource) = 0;
    virtual void endDragCopy() = 0;

    // Drop
    //! TODO Change KeyboardModifiers to modes
    virtual void startDrop(const QByteArray& edata) = 0;
    virtual bool startDrop(const QUrl& url) = 0;
    virtual bool isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers) = 0; //! NOTE Also may set drop target
    virtual bool drop(const PointF& pos, Qt::KeyboardModifiers modifiers) = 0;
    virtual void setDropTarget(const EngravingItem* item, bool notify = true) = 0;
    virtual void setDropRect(const RectF& rect) = 0;
    virtual void endDrop() = 0;
    virtual async::Notification dropChanged() const = 0;

    virtual bool applyPaletteElement(mu::engraving::EngravingItem* element, Qt::KeyboardModifiers modifiers = {}) = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;

    // Change selection
    virtual bool moveSelectionAvailable(MoveSelectionType type) const = 0;
    virtual void moveSelection(MoveDirection d, MoveSelectionType type) = 0;

    virtual void moveLyrics(MoveDirection d) = 0;
    virtual void expandSelection(ExpandSelectionMode mode) = 0;
    virtual void addToSelection(MoveDirection d, MoveSelectionType type) = 0;
    virtual void selectTopStaff() = 0;
    virtual void selectEmptyTrailingMeasure() = 0;
    virtual void moveSegmentSelection(MoveDirection d) = 0;

    // Move/nudge elements
    virtual void movePitch(MoveDirection d, PitchMode mode) = 0;
    virtual void nudge(MoveDirection d, bool quickly) = 0;
    virtual void moveChordRestToStaff(MoveDirection d) = 0;
    virtual void swapChordRest(MoveDirection d) = 0;

    // Text edit
    virtual bool isTextSelected() const = 0;
    virtual bool isTextEditingStarted() const = 0;
    virtual bool textEditingAllowed(const EngravingItem* element) const = 0;
    virtual void startEditText(EngravingItem* element, const PointF& elementPos = PointF()) = 0;
    virtual void editText(QInputMethodEvent* event) = 0;
    virtual void endEditText() = 0;
    virtual void changeTextCursorPosition(const PointF& newCursorPos) = 0;
    virtual void selectText(mu::engraving::SelectTextType type) = 0;
    virtual const TextBase* editedText() const = 0;
    virtual async::Notification textEditingStarted() const = 0;
    virtual async::Notification textEditingChanged() const = 0;
    virtual async::Channel<TextBase*> textEditingEnded() const = 0;

    // Display
    virtual async::Channel<ScoreConfigType> scoreConfigChanged() const = 0;

    // Grip edit
    virtual bool isGripEditStarted() const = 0;
    virtual bool isHitGrip(const PointF& pos) const = 0;
    virtual void startEditGrip(const PointF& pos) = 0;
    virtual void startEditGrip(EngravingItem* element, mu::engraving::Grip grip) = 0;
    virtual void endEditGrip() = 0;

    virtual bool isElementEditStarted() const = 0;
    virtual void startEditElement(EngravingItem* element) = 0;
    virtual void changeEditElement(EngravingItem* newElement) = 0;
    virtual bool isEditAllowed(QKeyEvent* event) = 0;
    virtual void editElement(QKeyEvent* event) = 0;
    virtual void endEditElement() = 0;

    virtual void splitSelectedMeasure() = 0;
    virtual void joinSelectedMeasures() = 0;

    virtual Ret canAddBoxes() const = 0;
    virtual void addBoxes(BoxType boxType, int count, AddBoxesTarget target) = 0;
    virtual void addBoxes(BoxType boxType, int count, int beforeBoxIndex) = 0;

    virtual void copySelection() = 0;
    virtual mu::Ret repeatSelection() = 0;
    virtual void copyLyrics() = 0;
    virtual void pasteSelection(const Fraction& scale = Fraction(1, 1)) = 0;
    virtual void swapSelection() = 0;
    virtual void deleteSelection() = 0;
    virtual void flipSelection() = 0;
    virtual void addTieToSelection() = 0;
    virtual void addTiedNoteToChord() = 0;
    virtual void addSlurToSelection() = 0;
    virtual void addOttavaToSelection(OttavaType type) = 0;
    virtual void addHairpinsToSelection(HairpinType type) = 0;
    virtual void addAccidentalToSelection(AccidentalType type) = 0;
    virtual void putRestToSelection() = 0;
    virtual void putRest(Duration duration) = 0;
    virtual void addBracketsToSelection(BracketsType type) = 0;
    virtual void changeSelectedNotesArticulation(SymbolId articulationSymbolId) = 0;
    virtual void addGraceNotesToSelectedNotes(GraceNoteType type) = 0;
    virtual bool canAddTupletToSelectedChordRests() const = 0;
    virtual void addTupletToSelectedChordRests(const TupletOptions& options) = 0;
    virtual void addBeamToSelectedChordRests(BeamMode mode) = 0;

    virtual void increaseDecreaseDuration(int steps, bool stepByDots) = 0;

    virtual bool toggleLayoutBreakAvailable() const = 0;
    virtual void toggleLayoutBreak(LayoutBreakType breakType) = 0;

    virtual void setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval = 0) = 0;
    virtual bool transpose(const TransposeOptions& options) = 0;
    virtual void swapVoices(int voiceIndex1, int voiceIndex2) = 0;
    virtual void addIntervalToSelectedNotes(int interval) = 0;
    virtual void addFret(int fretIndex) = 0;
    virtual void changeSelectedNotesVoice(int voiceIndex) = 0;
    virtual void addAnchoredLineToSelectedNotes() = 0;

    virtual void addTextToTopFrame(TextStyleType type) = 0;

    virtual Ret canAddTextToItem(TextStyleType type, const EngravingItem* item) const = 0;
    virtual void addTextToItem(TextStyleType type, EngravingItem* item) = 0;

    virtual Ret canAddImageToItem(const EngravingItem* item) const = 0;
    virtual void addImageToItem(const io::path_t& imagePath, EngravingItem* item) = 0;

    virtual Ret canAddFiguredBass() const = 0;
    virtual void addFiguredBass() = 0;

    virtual void addStretch(qreal value) = 0;

    virtual void addTimeSignature(Measure* measure, engraving::staff_idx_t staffIndex, TimeSignature* timeSignature) = 0;

    virtual void explodeSelectedStaff() = 0;
    virtual void implodeSelectedStaff() = 0;

    virtual void realizeSelectedChordSymbols(bool literal, Voicing voicing, HarmonyDurationType durationType) = 0;
    virtual void removeSelectedMeasures() = 0;
    virtual void removeSelectedRange() = 0;
    virtual void removeEmptyTrailingMeasures() = 0;

    virtual void fillSelectionWithSlashes() = 0;
    virtual void replaceSelectedNotesWithSlashes() = 0;

    virtual void changeEnharmonicSpelling(bool both) = 0;
    virtual void spellPitches() = 0;
    virtual void regroupNotesAndRests() = 0;
    virtual void resequenceRehearsalMarks() = 0;

    virtual void resetStretch() = 0;
    virtual void resetTextStyleOverrides() = 0;
    virtual void resetBeamMode() = 0;
    virtual void resetShapesAndPosition() = 0;

    virtual ScoreConfig scoreConfig() const = 0;
    virtual void setScoreConfig(const ScoreConfig& config) = 0;

    virtual void addMelisma() = 0;
    virtual void addLyricsVerse() = 0;

    // Text navigation
    virtual void navigateToLyrics(MoveDirection direction, bool moveOnly = false) = 0;
    virtual void navigateToLyricsVerse(MoveDirection direction) = 0;

    virtual void navigateToNextSyllable() = 0;

    virtual void navigateToNearHarmony(MoveDirection direction, bool nearNoteOrRest) = 0;
    virtual void navigateToHarmonyInNearMeasure(MoveDirection direction) = 0;
    virtual void navigateToHarmony(const Fraction& ticks) = 0;

    virtual void navigateToNearFiguredBass(MoveDirection direction) = 0;
    virtual void navigateToFiguredBassInNearMeasure(MoveDirection direction) = 0;
    virtual void navigateToFiguredBass(const Fraction& ticks) = 0;

    virtual void navigateToNearText(MoveDirection direction) = 0;

    // Text style
    virtual void toggleBold() = 0;
    virtual void toggleItalic() = 0;
    virtual void toggleUnderline() = 0;
    virtual void toggleStrike() = 0;

    virtual bool canInsertClef(mu::engraving::ClefType) const = 0;
    virtual void insertClef(mu::engraving::ClefType) = 0;

    virtual void toggleArticulation(mu::engraving::SymId) = 0;
    virtual void changeAccidental(mu::engraving::AccidentalType) = 0;
    virtual void transposeSemitone(int) = 0;
    virtual void transposeDiatonicAlterations(mu::engraving::TransposeDirection) = 0;
    virtual void toggleAutoplace(bool all) = 0;
    virtual void getLocation() = 0;
    virtual void execute(void (mu::engraving::Score::*)()) = 0;

    struct ShowItemRequest {
        const EngravingItem* item = nullptr;
        RectF showRect;
    };

    virtual void showItem(const mu::engraving::EngravingItem* item, int staffIndex = -1) = 0;
    virtual async::Channel<ShowItemRequest> showItemRequested() const = 0;

    virtual void setGetViewRectFunc(const std::function<RectF()>& func) = 0;
};

using INotationInteractionPtr = std::shared_ptr<INotationInteraction>;

EngravingItem* contextItem(INotationInteractionPtr);
}

#endif // MU_NOTATION_INOTATIONINTERACTION_H
