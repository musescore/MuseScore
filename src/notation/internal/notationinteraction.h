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
#ifndef MU_NOTATION_NOTATIONINTERACTION_H
#define MU_NOTATION_NOTATIONINTERACTION_H

#include <memory>
#include <vector>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "inotationinteraction.h"
#include "inotationconfiguration.h"
#include "inotationundostack.h"
#include "iinteractive.h"

#include "libmscore/element.h"
#include "libmscore/elementgroup.h"
#include "scorecallbacks.h"

namespace Ms {
class ShadowNote;
class Lasso;
}

namespace mu::notation {
class Notation;
class NotationInteraction : public INotationInteraction, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, framework::IInteractive, interactive)

public:
    NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack);
    ~NotationInteraction() override;

    void init();
    void paint(draw::Painter* painter);

    // Put notes
    INotationNoteInputPtr noteInput() const override;

    // Shadow note
    void showShadowNote(const PointF& pos) override;
    void hideShadowNote() override;

    // Visibility
    void toggleVisible() override;

    // Hit
    Element* hitElement(const PointF& pos, float width) const override;
    Staff* hitStaff(const PointF& pos) const override;
    const HitElementContext& hitElementContext() const override;
    void setHitElementContext(const HitElementContext& context) override;

    // Select
    void addChordToSelection(MoveDirection d) override;
    void moveChordNoteSelection(MoveDirection d) override;
    void select(const std::vector<Element*>& elements, SelectType type, int staffIndex = 0) override;
    void selectAll() override;
    void selectSection() override;
    void selectFirstElement() override;
    void selectLastElement() override;
    INotationSelectionPtr selection() const override;
    void clearSelection() override;
    async::Notification selectionChanged() const override;

    // Drag
    bool isDragStarted() const override;
    void startDrag(const std::vector<Element*>& elems, const PointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const PointF& fromPos, const PointF& toPos, DragMode mode) override;
    void endDrag() override;
    async::Notification dragChanged() const override;

    // Drop
    void startDrop(const QByteArray& edata) override;
    bool isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers) override;
    bool drop(const PointF& pos, Qt::KeyboardModifiers modifiers) override;
    void endDrop() override;
    async::Notification dropChanged() const override;

    bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) override;
    void undo() override;
    void redo() override;

    // Move
    //! NOTE Perform operations on selected elements
    void moveSelection(MoveDirection d, MoveSelectionType type) override;
    void movePitch(MoveDirection d, PitchMode mode) override; //! NOTE Requires a note to be selected
    void moveText(MoveDirection d, bool quickly) override;    //! NOTE Requires a text element to be selected

    // Text edit
    bool isTextEditingStarted() const override;
    void startEditText(Element* element, const PointF& cursorPos) override;
    void editText(QKeyEvent* event) override;
    void endEditText() override;
    void changeTextCursorPosition(const PointF& newCursorPos) override;
    const TextBase* editedText() const override;
    async::Notification textEditingStarted() const override;
    async::Notification textEditingChanged() const override;

    // Grip edit
    bool isGripEditStarted() const override;
    bool isHitGrip(const PointF& pos) const override;
    void startEditGrip(const PointF& pos) override;
    void endEditGrip() override;

    // Measure
    void splitSelectedMeasure() override;
    void joinSelectedMeasures() override;

    void addBoxes(BoxType boxType, int count, int beforeBoxIndex = -1) override;

    void copySelection() override;
    void copyLyrics() override;
    void pasteSelection(const Fraction& scale = Fraction(1, 1)) override;
    void swapSelection() override;
    void deleteSelection() override;
    void flipSelection() override;
    void addTieToSelection() override;
    void addTiedNoteToChord() override;
    void addSlurToSelection() override;
    void addOttavaToSelection(OttavaType type) override;
    void addHairpinToSelection(HairpinType type) override;
    void addAccidentalToSelection(AccidentalType type) override;
    void putRestToSelection() override;
    void putRest(DurationType duration) override;
    void addBracketsToSelection(BracketsType type) override;
    void changeSelectedNotesArticulation(SymbolId articulationSymbolId) override;
    void addGraceNotesToSelectedNotes(GraceNoteType type) override;
    void addTupletToSelectedChordRests(const TupletOptions& options) override;
    void addBeamToSelectedChordRests(BeamMode mode) override;

    void increaseDecreaseDuration(int steps, bool stepByDots) override;

    void toggleLayoutBreak(LayoutBreakType breakType) override;
    void setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval = 0) override;
    bool transpose(const TransposeOptions& options) override;
    void swapVoices(int voiceIndex1, int voiceIndex2) override;
    void addIntervalToSelectedNotes(int interval) override;
    void addFret(int fretIndex) override;
    void changeSelectedNotesVoice(int voiceIndex) override;
    void addAnchoredLineToSelectedNotes() override;

    void addText(TextType type) override;
    void addFiguredBass() override;

    void addStretch(qreal value) override;

    void addTimeSignature(Measure* measure, int staffIndex, TimeSignature* timeSignature) override;

    void explodeSelectedStaff() override;
    void implodeSelectedStaff() override;

    void realizeSelectedChordSymbols() override;
    void removeSelectedRange() override;
    void removeEmptyTrailingMeasures() override;

    void fillSelectionWithSlashes() override;
    void replaceSelectedNotesWithSlashes() override;

    void spellPitches() override;
    void regroupNotesAndRests() override;
    void resequenceRehearsalMarks() override;
    void unrollRepeats() override;

    void resetToDefault(ResettableValueType type) override;

    ScoreConfig scoreConfig() const override;
    void setScoreConfig(ScoreConfig config) override;
    async::Channel<ScoreConfigType> scoreConfigChanged() const override;

    void nextLyrics(bool = false, bool = false, bool = true) override;
    void nextLyricsVerse(bool = false) override;
    void nextSyllable() override;
    void addMelisma() override;
    void addLyricsVerse() override;

private:
    Ms::Score* score() const;

    void startEdit();
    void apply();

    void doSelect(const std::vector<Element*>& elements, SelectType type, int staffIndex);
    void notifyAboutDragChanged();
    void notifyAboutDropChanged();
    void notifyAboutSelectionChanged();
    void notifyAboutNotationChanged();
    void notifyAboutTextEditingStarted();
    void notifyAboutTextEditingChanged();
    void doDragLasso(const PointF& p);
    void endLasso();

    Ms::Page* point2page(const PointF& p) const;
    QList<Element*> hitElements(const PointF& p_in, float w) const;
    QList<Element*> elementsAt(const PointF& p) const;
    Element* elementAt(const PointF& p) const;
    static bool elementIsLess(const Ms::Element* e1, const Ms::Element* e2);

    void setAnchorLines(const std::vector<LineF>& anchorList);
    void resetAnchorLines();
    void drawAnchorLines(draw::Painter* painter);
    void drawTextEditMode(mu::draw::Painter* painter);
    void drawSelectionRange(mu::draw::Painter* painter);
    void drawGripPoints(mu::draw::Painter* painter);
    void moveElementSelection(MoveDirection d);

    Element* dropTarget(Ms::EditData& ed) const;
    bool dragMeasureAnchorElement(const PointF& pos);
    bool dragTimeAnchorElement(const PointF& pos);
    void setDropTarget(Element* el);
    bool dropCanvas(Element* e);

    void selectInstrument(Ms::InstrumentChange* instrumentChange);

    void applyDropPaletteElement(Ms::Score* score, Ms::Element* target, Ms::Element* e, Qt::KeyboardModifiers modifiers,
                                 PointF pt = PointF(), bool pasteMode = false);

    void doAddSlur(const Ms::Slur* slurTemplate = nullptr);
    void doAddSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Ms::Slur* slurTemplate);

    bool scoreHasMeasure() const;
    bool notesHaveActiculation(const std::vector<Note*>& notes, SymbolId articulationSymbolId) const;

    bool needEndTextEditing(const std::vector<Element*>& newSelectedElements) const;

    void updateGripEdit(const std::vector<Element*>& elements);
    void resetGripEdit();

    void resetStretch();
    void resetTextStyleOverrides();
    void resetBeamMode();
    void resetShapesAndPosition();

    struct HitMeasureData
    {
        Measure* measure = nullptr;
        Staff* staff = nullptr;
    };

    HitMeasureData hitMeasure(const PointF& pos) const;

    struct DragData
    {
        PointF beginMove;
        PointF elementOffset;
        Ms::EditData ed;
        std::vector<Element*> elements;
        std::vector<std::unique_ptr<Ms::ElementGroup> > dragGroups;
        DragMode mode { DragMode::BothXY };
        void reset();
    };

    struct DropData
    {
        Ms::EditData ed;
        Element* dropTarget = nullptr;
    };

    ScoreCallbacks m_scoreCallbacks;
    Notation* m_notation = nullptr;
    INotationUndoStackPtr m_undoStack;

    INotationNoteInputPtr m_noteInput = nullptr;
    Ms::ShadowNote* m_shadowNote = nullptr;

    INotationSelectionPtr m_selection = nullptr;
    async::Notification m_selectionChanged;

    DragData m_dragData;
    async::Notification m_dragChanged;
    std::vector<LineF> m_anchorLines;

    Ms::EditData m_textEditData;
    async::Notification m_textEditingStarted;
    async::Notification m_textEditingChanged;

    Ms::EditData m_gripEditData;

    DropData m_dropData;
    async::Notification m_dropChanged;

    async::Channel<ScoreConfigType> m_scoreConfigChanged;

    Ms::Lasso* m_lasso = nullptr;

    bool m_notifyAboutDropChanged = false;
    HitElementContext m_hitElementContext;
};
}

#endif // MU_NOTATION_NOTATIONINTERACTION_H
