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
#include "iinteractive.h"

#include "inotationinteraction.h"
#include "inotationconfiguration.h"
#include "inotationundostack.h"
#include "iselectinstrumentscenario.h"

#include "libmscore/engravingitem.h"
#include "libmscore/elementgroup.h"
#include "scorecallbacks.h"

namespace mu::engraving {
class Lasso;
}

class QDrag;

namespace mu::notation {
class Notation;
class NotationSelection;
class NotationInteraction : public INotationInteraction, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, ISelectInstrumentsScenario, selectInstrumentScenario)
    INJECT(notation, framework::IInteractive, interactive)

public:
    NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack);

    void paint(draw::Painter* painter);

    // Put notes
    INotationNoteInputPtr noteInput() const override;

    // Shadow note
    void showShadowNote(const PointF& pos) override;
    void hideShadowNote() override;

    // Visibility
    void toggleVisible() override;

    // Hit
    EngravingItem* hitElement(const PointF& pos, float width) const override;
    Staff* hitStaff(const PointF& pos) const override;
    const HitElementContext& hitElementContext() const override;
    void setHitElementContext(const HitElementContext& context) override;

    // Select
    void moveChordNoteSelection(MoveDirection d) override;
    void select(const std::vector<EngravingItem*>& elements, SelectType type = SelectType::REPLACE,
                engraving::staff_idx_t staffIndex = 0) override;
    void selectAll() override;
    void selectSection() override;
    void selectFirstElement(bool frame = false) override;
    void selectLastElement() override;
    INotationSelectionPtr selection() const override;
    void clearSelection() override;
    async::Notification selectionChanged() const override;
    void selectTopOrBottomOfChord(MoveDirection d) override;
    void moveSegmentSelection(MoveDirection d) override;

    // SelectionFilter
    bool isSelectionTypeFiltered(SelectionFilterType type) const override;
    void setSelectionTypeFiltered(SelectionFilterType type, bool filtered) override;

    // Drag
    bool isDragStarted() const override;
    void startDrag(const std::vector<EngravingItem*>& elems, const PointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const PointF& fromPos, const PointF& toPos, DragMode mode) override;
    void endDrag() override;
    async::Notification dragChanged() const override;

    bool isDragCopyStarted() const override;
    void startDragCopy(const EngravingItem* element, QObject* dragSource) override;
    void endDragCopy() override;

    // Drop
    void startDrop(const QByteArray& edata) override;
    bool startDrop(const QUrl& url) override;
    bool isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers) override;
    bool drop(const PointF& pos, Qt::KeyboardModifiers modifiers) override;
    void setDropTarget(const EngravingItem* item, bool notify = true) override;
    void setDropRect(const RectF& rect) override;
    void endDrop() override;
    async::Notification dropChanged() const override;

    bool applyPaletteElement(mu::engraving::EngravingItem* element, Qt::KeyboardModifiers modifiers = {}) override;
    void undo() override;
    void redo() override;

    // Change selection
    bool moveSelectionAvailable(MoveSelectionType type) const override;
    void moveSelection(MoveDirection d, MoveSelectionType type) override;
    void expandSelection(ExpandSelectionMode mode) override;
    void addToSelection(MoveDirection d, MoveSelectionType type) override;
    void selectTopStaff() override;
    void selectEmptyTrailingMeasure() override;

    // Move
    void movePitch(MoveDirection d, PitchMode mode) override;
    void nudge(MoveDirection d, bool quickly) override;
    void moveChordRestToStaff(MoveDirection d) override;
    void moveLyrics(MoveDirection d) override;
    void swapChordRest(MoveDirection d) override;

    // Text edit
    bool isTextSelected() const override;
    bool isTextEditingStarted() const override;
    bool textEditingAllowed(const EngravingItem* element) const override;
    void startEditText(EngravingItem* element, const PointF& cursorPos = PointF()) override;
    void editText(QInputMethodEvent* event) override;
    void endEditText() override;
    void changeTextCursorPosition(const PointF& newCursorPos) override;
    void selectText(mu::engraving::SelectTextType type) override;
    const TextBase* editedText() const override;
    async::Notification textEditingStarted() const override;
    async::Notification textEditingChanged() const override;
    async::Channel<TextBase*> textEditingEnded() const override;

    // Grip edit
    bool isGripEditStarted() const override;
    bool isHitGrip(const PointF& pos) const override;
    void startEditGrip(const PointF& pos) override;
    void startEditGrip(EngravingItem* element, mu::engraving::Grip grip) override;
    void endEditGrip() override;

    bool isElementEditStarted() const override;
    void startEditElement(EngravingItem* element) override;
    void changeEditElement(EngravingItem* newElement) override;
    bool isEditAllowed(QKeyEvent* event) override;
    void editElement(QKeyEvent* event) override;
    void endEditElement() override;

    // Measure
    void splitSelectedMeasure() override;
    void joinSelectedMeasures() override;

    Ret canAddBoxes() const override;
    void addBoxes(BoxType boxType, int count, AddBoxesTarget target) override;
    void addBoxes(BoxType boxType, int count, int beforeBoxIndex) override;

    void copySelection() override;
    void copyLyrics() override;
    Ret repeatSelection() override;
    void pasteSelection(const Fraction& scale = Fraction(1, 1)) override;
    void swapSelection() override;
    void deleteSelection() override;
    void flipSelection() override;
    void addTieToSelection() override;
    void addTiedNoteToChord() override;
    void addSlurToSelection() override;
    void addOttavaToSelection(OttavaType type) override;
    void addHairpinsToSelection(HairpinType type) override;
    void addAccidentalToSelection(AccidentalType type) override;
    void putRestToSelection() override;
    void putRest(Duration duration) override;
    void addBracketsToSelection(BracketsType type) override;
    void changeSelectedNotesArticulation(SymbolId articulationSymbolId) override;
    void addGraceNotesToSelectedNotes(GraceNoteType type) override;
    bool canAddTupletToSelectedChordRests() const override;
    void addTupletToSelectedChordRests(const TupletOptions& options) override;
    void addBeamToSelectedChordRests(BeamMode mode) override;

    void increaseDecreaseDuration(int steps, bool stepByDots) override;

    bool toggleLayoutBreakAvailable() const override;
    void toggleLayoutBreak(LayoutBreakType breakType) override;

    void setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval = 0) override;
    bool transpose(const TransposeOptions& options) override;
    void swapVoices(int voiceIndex1, int voiceIndex2) override;
    void addIntervalToSelectedNotes(int interval) override;
    void addFret(int fretIndex) override;
    void changeSelectedNotesVoice(int voiceIndex) override;
    void addAnchoredLineToSelectedNotes() override;

    void addTextToTopFrame(TextStyleType type) override;

    Ret canAddTextToItem(TextStyleType type, const EngravingItem* item) const override;
    void addTextToItem(TextStyleType type, EngravingItem* item) override;

    Ret canAddImageToItem(const EngravingItem* item) const override;
    void addImageToItem(const io::path_t& imagePath, EngravingItem* item) override;

    Ret canAddFiguredBass() const override;
    void addFiguredBass() override;

    void addStretch(qreal value) override;

    void addTimeSignature(Measure* measure, engraving::staff_idx_t staffIndex, TimeSignature* timeSignature) override;

    void explodeSelectedStaff() override;
    void implodeSelectedStaff() override;

    void realizeSelectedChordSymbols(bool literal, Voicing voicing, HarmonyDurationType durationType) override;
    void removeSelectedMeasures() override;
    void removeSelectedRange() override;
    void removeEmptyTrailingMeasures() override;

    void fillSelectionWithSlashes() override;
    void replaceSelectedNotesWithSlashes() override;
    void changeEnharmonicSpelling(bool) override;
    void spellPitches() override;
    void regroupNotesAndRests() override;
    void resequenceRehearsalMarks() override;

    void resetStretch() override;
    void resetTextStyleOverrides() override;
    void resetBeamMode() override;
    void resetShapesAndPosition() override;

    ScoreConfig scoreConfig() const override;
    void setScoreConfig(const ScoreConfig& config) override;
    async::Channel<ScoreConfigType> scoreConfigChanged() const override;

    void navigateToLyrics(MoveDirection direction, bool moveOnly = true) override;
    void navigateToLyricsVerse(MoveDirection direction) override;

    void navigateToNextSyllable() override;

    void navigateToNearHarmony(MoveDirection direction, bool nearNoteOrRest) override;
    void navigateToHarmonyInNearMeasure(MoveDirection direction) override;
    void navigateToHarmony(const Fraction& ticks) override;

    void navigateToNearFiguredBass(MoveDirection direction) override;
    void navigateToFiguredBassInNearMeasure(MoveDirection direction) override;
    void navigateToFiguredBass(const Fraction& ticks) override;

    void navigateToNearText(MoveDirection direction) override;

    void addMelisma() override;
    void addLyricsVerse() override;

    void toggleBold() override;
    void toggleItalic() override;
    void toggleUnderline() override;
    void toggleStrike() override;
    void toggleArticulation(mu::engraving::SymId) override;
    void toggleAutoplace(bool) override;

    void insertClef(mu::engraving::ClefType) override;
    void changeAccidental(mu::engraving::AccidentalType) override;
    void transposeSemitone(int) override;
    void transposeDiatonicAlterations(mu::engraving::TransposeDirection) override;
    void toggleGlobalOrLocalInsert() override;
    void getLocation() override;
    void execute(void (mu::engraving::Score::*)()) override;

    void showItem(const mu::engraving::EngravingItem* item, int staffIndex = -1) override;
    async::Channel<ShowItemRequest> showItemRequested() const override;

    void setGetViewRectFunc(const std::function<RectF()>& func) override;

private:
    mu::engraving::Score* score() const;
    void onScoreInited();

    void startEdit();
    void apply();
    void rollback();

    void checkAndShowMScoreError() const;

    bool needStartEditGrip(QKeyEvent* event) const;
    bool handleKeyPress(QKeyEvent* event);

    void doEndEditElement();
    void doEndDrag();

    void onElementDestroyed(EngravingItem* element);

    void doSelect(const std::vector<EngravingItem*>& elements, SelectType type, engraving::staff_idx_t staffIndex = 0);
    void selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment);

    void notifyAboutDragChanged();
    void notifyAboutDropChanged();
    void notifyAboutSelectionChangedIfNeed();
    void notifyAboutNotationChanged();
    void notifyAboutTextEditingStarted();
    void notifyAboutTextEditingChanged();
    void notifyAboutTextEditingEnded(TextBase* text);
    void notifyAboutNoteInputStateChanged();
    void doDragLasso(const PointF& p);
    void endLasso();
    void toggleFontStyle(mu::engraving::FontStyle);
    void navigateToLyrics(bool, bool, bool);

    mu::engraving::Harmony* editedHarmony() const;
    mu::engraving::Harmony* findHarmonyInSegment(const mu::engraving::Segment* segment, engraving::track_idx_t track,
                                                 mu::engraving::TextStyleType textStyleType) const;
    mu::engraving::Harmony* createHarmony(mu::engraving::Segment* segment, engraving::track_idx_t track,
                                          mu::engraving::HarmonyType type) const;

    void addText(TextStyleType type, EngravingItem* item = nullptr);

    void startEditText(mu::engraving::TextBase* text);
    bool needEndTextEdit() const;

    mu::engraving::Page* point2page(const PointF& p) const;
    std::vector<EngravingItem*> hitElements(const PointF& p_in, float w) const;
    std::vector<EngravingItem*> elementsAt(const PointF& p) const;
    EngravingItem* elementAt(const PointF& p) const;

    // Sorting using this function will place the elements that are the most
    // interesting to be selected at the end of the list
    static bool elementIsLess(const mu::engraving::EngravingItem* e1, const mu::engraving::EngravingItem* e2);

    void updateAnchorLines();
    void setAnchorLines(const std::vector<LineF>& anchorList);
    void resetAnchorLines();
    double currentScaling(draw::Painter* painter) const;
    void drawAnchorLines(draw::Painter* painter);
    void drawTextEditMode(mu::draw::Painter* painter);
    void drawSelectionRange(mu::draw::Painter* painter);
    void drawGripPoints(mu::draw::Painter* painter);
    void moveElementSelection(MoveDirection d);
    void moveStringSelection(MoveDirection d);

    EngravingItem* dropTarget(mu::engraving::EditData& ed) const;
    bool dragMeasureAnchorElement(const PointF& pos);
    bool dragTimeAnchorElement(const PointF& pos);
    bool dropCanvas(EngravingItem* e);
    void resetDropElement();

    bool selectInstrument(mu::engraving::InstrumentChange* instrumentChange);

    void applyDropPaletteElement(mu::engraving::Score* score, mu::engraving::EngravingItem* target, mu::engraving::EngravingItem* e,
                                 Qt::KeyboardModifiers modifiers, PointF pt = PointF(), bool pasteMode = false);

    void doAddSlur(const mu::engraving::Slur* slurTemplate = nullptr);
    void doAddSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const mu::engraving::Slur* slurTemplate);

    bool scoreHasMeasure() const;
    bool notesHaveActiculation(const std::vector<Note*>& notes, SymbolId articulationSymbolId) const;

    bool needEndTextEditing(const std::vector<EngravingItem*>& newSelectedElements) const;
    bool needEndElementEditing(const std::vector<EngravingItem*>& newSelectedElements) const;

    void resetGripEdit();
    void resetHitElementContext();

    bool elementsSelected(const std::set<ElementType>& elementsTypes) const;

    template<typename P>
    void execute(void (mu::engraving::Score::* function)(P), P param);

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
        mu::engraving::EditData ed;
        std::vector<EngravingItem*> elements;
        std::vector<std::unique_ptr<mu::engraving::ElementGroup> > dragGroups;
        void reset();
    };

    struct DropData
    {
        mu::engraving::EditData ed;
        const EngravingItem* dropTarget = nullptr;
        RectF dropRect;
    };

    ScoreCallbacks m_scoreCallbacks;
    Notation* m_notation = nullptr;
    INotationUndoStackPtr m_undoStack;

    INotationNoteInputPtr m_noteInput = nullptr;

    std::shared_ptr<NotationSelection> m_selection = nullptr;
    async::Notification m_selectionChanged;

    DragData m_dragData;
    async::Notification m_dragChanged;
    std::vector<LineF> m_anchorLines;

    QDrag* m_drag = nullptr;

    mu::engraving::EditData m_editData;

    async::Notification m_textEditingStarted;
    async::Notification m_textEditingChanged;
    async::Channel<TextBase*> m_textEditingEnded;

    DropData m_dropData;
    async::Notification m_dropChanged;

    async::Channel<ScoreConfigType> m_scoreConfigChanged;

    mu::engraving::Lasso* m_lasso = nullptr;

    bool m_notifyAboutDropChanged = false;
    HitElementContext m_hitElementContext;

    async::Channel<ShowItemRequest> m_showItemRequested;
};
}

#endif // MU_NOTATION_NOTATIONINTERACTION_H
