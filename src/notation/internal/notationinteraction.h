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
#pragma once

#include <memory>
#include <vector>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iinteractive.h"
#include "engraving/rendering/isinglerenderer.h"
#include "engraving/rendering/ieditmoderenderer.h"

#include "../inotationinteraction.h"
#include "../inotationconfiguration.h"
#include "../iselectinstrumentscenario.h"
#include "inotationundostack.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/elementgroup.h"
#include "engraving/types/symid.h"
#include "scorecallbacks.h"

namespace mu::engraving {
class Lasso;
}

class QDrag;

namespace mu::notation {
class Notation;
class NotationSelection;
class NotationSelectionFilter;
class NotationInteraction : public INotationInteraction, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<ISelectInstrumentsScenario> selectInstrumentScenario = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<engraving::rendering::ISingleRenderer> engravingRenderer = { this };
    muse::Inject<engraving::rendering::IEditModeRenderer> editModeRenderer = { this };

public:
    NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack);

    void paint(muse::draw::Painter* painter);

    // Put notes
    INotationNoteInputPtr noteInput() const override;

    // Shadow note
    mu::engraving::ShadowNote* shadowNote() const override;
    bool showShadowNote(const muse::PointF& pos) override;
    void hideShadowNote() override;
    muse::RectF shadowNoteRect() const override;
    muse::async::Notification shadowNoteChanged() const override;

    // Visibility
    void toggleVisible() override;

    // Hit
    EngravingItem* hitElement(const muse::PointF& pos, float width) const override;
    std::vector<EngravingItem*> hitElements(const muse::PointF& pos, float width) const override;
    Staff* hitStaff(const muse::PointF& pos) const override;
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
    muse::async::Notification selectionChanged() const override;
    muse::async::Notification playbackNotesChanged() const override;
    std::vector<Note*> playbackNotes() const override;
    void clearPlaybackNotes() override;
    void addPlaybackNote(Note*, int) override;
    int noteOttavaType(const mu::engraving::Note* note) override;

    void addGlissandoNote(mu::engraving::Note* note, int ticks, int duration_ticks, int ottavaType) override;
    void addGlissandoEndNote(mu::engraving::Note* note, int ottavaType) override;
    int glissandoNoteTicks() const override;
    int glissandoNoteDurationticks() const override;
    int glissandoCurrticks() const override;
    void glissandoEndNotesUpdate() override;
    muse::async::Notification glissandoEndNotesChanged() override;
    mu::engraving::Note* glissandoNote() const override;
    std::vector<mu::engraving::Note*> glissandoEndNotes() const override;
    void glissandoTick(int ticks) override;
    muse::async::Notification glissandoTickChanged() override;

    bool arpeggioNoteTicksExist(muse::PointF) const override;
    bool arpeggioPointEqual(muse::PointF) override;
    void addArpeggioPoint(muse::PointF) override;
    void arpeggioPointClear() override;
    void addArpeggioNote(mu::engraving::Note*, int, int, int) override;
    void updateArpeggioDuration(int) override;
    void addArpeggioNote(mu::engraving::Note*, int) override;
    int arpeggioNoteTicks() const override;
    int arpeggioNoteDurationticks() const override;
    int arpeggioCurrticks() const override;
    muse::async::Notification arpeggioNotesChanged() override;
    std::vector<mu::engraving::Note*> arpeggioNotes() const override;
    bool arpeggioIsDown() const override;
    void arpeggioNotesUpdate(bool) override;
    void arpeggioTick(int) override;
    muse::async::Notification arpeggioTickChanged() override;

    void addTrillNote(mu::engraving::Note*, int, int, int, int) override;
    void addTrillNote1(mu::engraving::Note*, int, int, int, int) override;
    int trillNoteTicks() const override;
    int trillNoteTicks1() const override;
    int trillNoteDurationticks() const override;
    int trillNoteDurationticks1() const override;
    int trillNoteTremolotype() const override;
    int trillNoteTremolotype1() const override;
    int trillCurrticks() const override;
    int trillCurrticks1() const override;
    void trillNoteUpdate() override;
    void trillNoteUpdate1() override;
    mu::engraving::Note *trillNote() const override;
    mu::engraving::Note *trillNote1() const override;
    bool trillTick(int) override;
    bool trillTick1(int) override;
    muse::async::Notification trillNoteChanged() override;
    muse::async::Notification trillNoteChanged1() override;
    muse::async::Notification trillTickChanged() override;
    muse::async::Notification trillTickChanged1() override;

    void notifyClefKeySigsKeysChanged() override;
    muse::async::Notification clefKeySigsKeysChanged() const override;
    void clearClefKeySigsKeys() override;
    std::set<uint> clefKeySigsKeys() const override;
    void addClefKeySigsKeys(uint) override;
    void notifyClefKeySigsKeysChange() override;
    void playingChang(bool is_playing) override;
    bool isPlaying() const override;
    void notifyPianoKeyboardNotesChanged() override;
    void selectTopOrBottomOfChord(MoveDirection d) override;
    void findAndSelectChordRest(const Fraction& tick) override;
    void moveSegmentSelection(MoveDirection d) override;

    EngravingItem* contextItem() const override;

    // SelectionFilter
    INotationSelectionFilterPtr selectionFilter() const override;

    // Drag
    bool isDragStarted() const override;
    void startDrag(const std::vector<EngravingItem*>& elems, const muse::PointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const muse::PointF& fromPos, const muse::PointF& toPos, DragMode mode) override;
    void endDrag() override;
    muse::async::Notification dragChanged() const override;

    bool isOutgoingDragElementAllowed(const EngravingItem* element) const override;
    void startOutgoingDragElement(const EngravingItem* element, QObject* dragSource) override;
    void startOutgoingDragRange(QObject* dragSource) override;
    bool isOutgoingDragStarted() const override;
    void endOutgoingDrag() override;

    // Drop
    bool startDropSingle(const QByteArray& edata) override;
    bool startDropRange(const QByteArray& data) override;
    bool startDropImage(const QUrl& url) override;
    bool updateDropSingle(const muse::PointF& pos, Qt::KeyboardModifiers modifiers) override;
    bool updateDropRange(const muse::PointF& pos) override;
    bool dropSingle(const muse::PointF& pos, Qt::KeyboardModifiers modifiers) override;
    bool dropRange(const QByteArray& data, const muse::PointF& pos, bool deleteSourceMaterial) override;
    void setDropTarget(EngravingItem* item, bool notify = true) override;
    void setDropRect(const muse::RectF& rect) override;
    void endDrop() override;
    muse::async::Notification dropChanged() const override;

    bool applyPaletteElement(mu::engraving::EngravingItem* element, Qt::KeyboardModifiers modifiers = {}) override;
    void undo() override;
    void redo() override;
    void undoRedoToIndex(size_t idx) override;

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
    void nudgeAnchors(MoveDirection) override;
    void moveChordRestToStaff(MoveDirection d) override;
    void moveLyrics(MoveDirection d) override;
    void swapChordRest(MoveDirection d) override;
    void toggleSnapToPrevious() override;
    void toggleSnapToNext() override;

    // Text edit
    bool isTextSelected() const override;
    bool isTextEditingStarted() const override;
    bool textEditingAllowed(const EngravingItem* element) const override;
    void startEditText(EngravingItem* element, const muse::PointF& cursorPos = muse::PointF()) override;
    void editText(QInputMethodEvent* event) override;
    void endEditText() override;
    void changeTextCursorPosition(const muse::PointF& newCursorPos) override;
    void selectText(mu::engraving::SelectTextType type) override;
    const TextBase* editedText() const override;
    muse::async::Notification textEditingStarted() const override;
    muse::async::Notification textEditingChanged() const override;
    muse::async::Channel<TextBase*> textEditingEnded() const override;

    // Grip edit
    bool isGripEditStarted() const override;
    bool isHitGrip(const muse::PointF& pos) const override;
    void startEditGrip(const muse::PointF& pos) override;
    void startEditGrip(EngravingItem* element, mu::engraving::Grip grip) override;
    void endEditGrip() override;

    bool isElementEditStarted() const override;
    void startEditElement(EngravingItem* element, bool editTextualProperties = true) override;
    void changeEditElement(EngravingItem* newElement) override;
    bool isEditAllowed(QKeyEvent* event) override;
    void editElement(QKeyEvent* event) override;
    void endEditElement() override;

    // Anchors edit
    void updateTimeTickAnchors(QKeyEvent* event) override;
    void moveElementAnchors(QKeyEvent* event) override;

    // Measure
    void splitSelectedMeasure() override;
    void joinSelectedMeasures() override;

    muse::Ret canAddBoxes() const override;
    void addBoxes(BoxType boxType, int count, AddBoxesTarget target) override;
    void addBoxes(BoxType boxType, int count, int beforeBoxIndex, bool moveSignaturesClef = true) override;

    void copySelection() override;
    void copyLyrics() override;
    muse::Ret repeatSelection() override;
    void pasteSelection(const Fraction& scale = Fraction(1, 1)) override;
    void swapSelection() override;
    void deleteSelection() override;
    void flipSelection() override;
    void flipSelectionHorizontally() override;
    void addTieToSelection() override;
    void addLaissezVibToSelection() override;
    void addTiedNoteToChord() override;
    void addSlurToSelection() override;
    void addHammerOnPullOffToSelection() override;
    void addOttavaToSelection(OttavaType type) override;
    void addHairpinOnGripDrag(engraving::EditData& ed, bool isLeftGrip) override;
    void addHairpinsToSelection(HairpinType type) override;
    void putRestToSelection() override;
    void putRest(Duration duration) override;
    void addBracketsToSelection(BracketsType type) override;
    void toggleAccidentalForSelection(AccidentalType type) override;
    void toggleArticulationForSelection(SymbolId articulationSymbolId) override;
    void toggleDotsForSelection(Pad dots) override;
    void addGraceNotesToSelectedNotes(GraceNoteType type) override;
    bool canAddTupletToSelectedChordRests() const override;
    void addTupletToSelectedChordRests(const TupletOptions& options) override;
    void addBeamToSelectedChordRests(BeamMode mode) override;

    void increaseDecreaseDuration(int steps, bool stepByDots) override;

    void autoFlipHairpinsType(engraving::Dynamic* selDyn) override;

    void toggleDynamicPopup() override;
    bool toggleLayoutBreakAvailable() const override;
    void toggleLayoutBreak(LayoutBreakType breakType) override;
    void moveMeasureToPrevSystem() override;
    void moveMeasureToNextSystem() override;
    void toggleSystemLock() override;
    void toggleScoreLock() override;
    void makeIntoSystem() override;
    void applySystemLock() override;

    void addRemoveSystemLocks(AddRemoveSystemLockType intervalType, int interval = 0) override;
    bool transpose(const TransposeOptions& options) override;
    void swapVoices(voice_idx_t voiceIndex1, voice_idx_t voiceIndex2) override;
    void addIntervalToSelectedNotes(int interval) override;
    void addFret(int fretIndex) override;
    void changeSelectedElementsVoice(voice_idx_t voiceIndex) override;
    void changeSelectedElementsVoiceAssignment(VoiceAssignment voiceAssignment) override;
    void addAnchoredLineToSelectedNotes() override;

    void addTextToTopFrame(TextStyleType type) override;

    muse::Ret canAddTextToItem(TextStyleType type, const EngravingItem* item) const override;
    void addTextToItem(TextStyleType type, EngravingItem* item) override;

    muse::Ret canAddImageToItem(const EngravingItem* item) const override;
    void addImageToItem(const muse::io::path_t& imagePath, EngravingItem* item) override;

    muse::Ret canAddFiguredBass() const override;
    void addFiguredBass() override;

    void addStretch(qreal value) override;

    Measure* selectedMeasure() const override;
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
    void resetToDefaultLayout() override;

    ScoreConfig scoreConfig() const override;
    void setScoreConfig(const ScoreConfig& config) override;
    muse::async::Channel<ScoreConfigType> scoreConfigChanged() const override;

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

    muse::Ret canAddGuitarBend() const override;
    void addGuitarBend(GuitarBendType bendType) override;

    muse::Ret canAddFretboardDiagram() const override;
    void addFretboardDiagram() override;

    void toggleBold() override;
    void toggleItalic() override;
    void toggleUnderline() override;
    void toggleStrike() override;
    void toggleSubScript() override;
    void toggleSuperScript() override;
    void toggleArticulation(mu::engraving::SymId) override;
    void toggleOrnament(mu::engraving::SymId) override;
    void toggleAutoplace(bool) override;

    bool canInsertClef(mu::engraving::ClefType) const override;
    void insertClef(mu::engraving::ClefType) override;

    void changeAccidental(mu::engraving::AccidentalType) override;
    void transposeSemitone(int) override;
    void transposeDiatonicAlterations(mu::engraving::TransposeDirection) override;
    void getLocation() override;
    void execute(void (mu::engraving::Score::*)(), const muse::TranslatableString& actionName) override;

    void showItem(const mu::engraving::EngravingItem* item, int staffIndex = -1) override;
    muse::async::Channel<ShowItemRequest> showItemRequested() const override;

    void setGetViewRectFunc(const std::function<muse::RectF()>& func) override;

private:
    mu::engraving::Score* score() const;
    void onScoreInited();
    void onViewModeChanged();

    void startEdit(const muse::TranslatableString& actionName);
    void apply();
    void rollback();

    struct ShadowNoteParams {
        mu::engraving::TDuration duration;
        mu::engraving::AccidentalType accidentalType = mu::engraving::AccidentalType::NONE;
        std::set<mu::engraving::SymId> articulationIds;
        mu::engraving::Position position;
    };

    bool showShadowNote(mu::engraving::ShadowNote& note, ShadowNoteParams& params);

    bool needStartEditGrip(QKeyEvent* event) const;
    bool handleKeyPress(QKeyEvent* event);

    void doEndEditElement();
    void doEndDrag();

    //! NOTE: Helper methods for applyPaletteElement
    void applyPaletteElementToList(EngravingItem* element, bool isMeasureAnchoredElement, mu::engraving::Score* score,
                                   const mu::engraving::Selection& sel, Qt::KeyboardModifiers modifiers = {});
    void applyPaletteElementToRange(EngravingItem* element, bool isMeasureAnchoredElement, mu::engraving::Score* score,
                                    const mu::engraving::Selection& sel, Qt::KeyboardModifiers modifiers = {});

    bool doDropStandard();
    bool doDropTextBaseAndSymbols(const muse::PointF& pos, bool applyUserOffset);

    void onElementDestroyed(EngravingItem* element);

    void doSelect(const std::vector<EngravingItem*>& elements, SelectType type, engraving::staff_idx_t staffIndex = 0);
    void selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment);
    void selectAndStartEditIfNeeded(EngravingItem* element);

    void notifyAboutDragChanged();
    void notifyAboutDropChanged();
    void notifyAboutSelectionChangedIfNeed();
    void notifyAboutPianoKeyboardNotesChanged();
    void notifyAboutNotationChanged();
    void notifyAboutTextEditingStarted();
    void notifyAboutTextEditingChanged();
    void notifyAboutTextEditingEnded(TextBase* text);
    void notifyAboutNoteInputStateChanged();
    void doDragLasso(const muse::PointF& p);
    void endLasso();
    void toggleFontStyle(mu::engraving::FontStyle);
    void toggleVerticalAlignment(mu::engraving::VerticalAlignment);
    void navigateToLyrics(bool, bool, bool);

    Harmony* editedHarmony() const;
    Segment* harmonySegment(const Harmony* harmony) const;
    mu::engraving::Harmony* findHarmonyInSegment(const mu::engraving::Segment* segment, engraving::track_idx_t track,
                                                 mu::engraving::TextStyleType textStyleType) const;
    mu::engraving::Harmony* createHarmony(mu::engraving::Segment* segment, engraving::track_idx_t track,
                                          mu::engraving::HarmonyType type) const;

    void addText(TextStyleType type, EngravingItem* item = nullptr);

    void startEditText(mu::engraving::TextBase* text);
    bool needEndTextEdit() const;

    mu::engraving::Page* point2page(const muse::PointF& p, bool useNearestPage = false) const;
    std::vector<EngravingItem*> elementsAt(const muse::PointF& p) const;
    EngravingItem* elementAt(const muse::PointF& p) const;

    // Sorting using this function will place the elements that are the most
    // interesting to be selected at the end of the list
    static bool elementIsLess(const mu::engraving::EngravingItem* e1, const mu::engraving::EngravingItem* e2);

    void updateGripAnchorLines();
    void updateDragAnchorLines();
    void setAnchorLines(const std::vector<muse::LineF>& anchorList);
    void resetAnchorLines();

    double currentScaling(muse::draw::Painter* painter) const;

    std::vector<ShadowNoteParams> previewNotes() const;

    bool shouldDrawInputPreview() const;
    void drawInputPreview(muse::draw::Painter* painter);

    void drawAnchorLines(muse::draw::Painter* painter);
    void drawTextEditMode(muse::draw::Painter* painter);
    void drawSelectionRange(muse::draw::Painter* painter);
    void drawGripPoints(muse::draw::Painter* painter);
    void drawLasso(muse::draw::Painter* painter);
    void drawDrop(muse::draw::Painter* painter);

    void moveElementSelection(MoveDirection d);
    void moveStringSelection(MoveDirection d);

    EngravingItem* dropTarget(mu::engraving::EditData& ed) const;
    bool prepareDropStandardElement(const muse::PointF& pos, Qt::KeyboardModifiers modifiers);
    bool prepareDropMeasureAnchorElement(const muse::PointF& pos);
    bool prepareDropTimeAnchorElement(const muse::PointF& pos);
    bool dropCanvas(EngravingItem* e);
    void resetDropData();

    void doFinishAddFretboardDiagram();

    bool selectInstrument(mu::engraving::InstrumentChange* instrumentChange);
    void cleanupDrumsetChanges(mu::engraving::InstrumentChange* instrumentChange) const;

    void applyDropPaletteElement(mu::engraving::Score* score, mu::engraving::EngravingItem* target, mu::engraving::EngravingItem* e,
                                 Qt::KeyboardModifiers modifiers, muse::PointF pt = muse::PointF(), bool pasteMode = false);

    void applyLineNoteToNote(engraving::Score* score, Note* note1, Note* note2, EngravingItem* line);

    void doAddSlur(const mu::engraving::Slur* slurTemplate = nullptr);
    void doAddSlur(EngravingItem* firstItem, EngravingItem* secondItem, const mu::engraving::Slur* slurTemplate);

    bool scoreHasMeasure() const;
    bool notesHaveActiculation(const std::vector<Note*>& notes, SymbolId articulationSymbolId) const;

    bool needEndTextEditing(const std::vector<EngravingItem*>& newSelectedElements) const;
    bool needEndElementEditing(const std::vector<EngravingItem*>& newSelectedElements) const;

    void resetGripEdit();
    void resetHitElementContext();

    template<typename P>
    void execute(void (mu::engraving::Score::* function)(P), P param, const muse::TranslatableString& actionName);

    struct HitMeasureData
    {
        Measure* measure = nullptr;
        Staff* staff = nullptr;
    };

    HitMeasureData hitMeasure(const muse::PointF& pos) const;

    struct DragData
    {
        muse::PointF beginMove;
        muse::PointF elementOffset;
        mu::engraving::EditData ed;
        std::vector<EngravingItem*> elements;
        std::vector<std::unique_ptr<mu::engraving::ElementGroup> > dragGroups;
        void reset();
    };

    struct ElementDropData
    {
        mu::engraving::EditData ed;
        EngravingItem* dropTarget = nullptr;
        muse::RectF dropRect;
    };

    struct RangeDropData
    {
        engraving::Fraction sourceTick;
        engraving::staff_idx_t sourceStaffIdx = muse::nidx;

        engraving::Fraction tickLength;
        size_t numStaves = 0;

        engraving::Segment* targetSegment = nullptr;
        engraving::staff_idx_t targetStaffIdx = 0;

        std::vector<muse::RectF> dropRects;
    };

    struct DropData
    {
        std::optional<ElementDropData> elementDropData;
        std::optional<RangeDropData> rangeDropData;
    };

    ScoreCallbacks m_scoreCallbacks;
    Notation* m_notation = nullptr;
    INotationUndoStackPtr m_undoStack;

    INotationNoteInputPtr m_noteInput = nullptr;

    muse::async::Notification m_shadowNoteChanged;

    std::shared_ptr<NotationSelection> m_selection = nullptr;
    std::shared_ptr<NotationSelection> m_playback_selection = nullptr;
    muse::async::Notification m_selectionChanged;

    muse::async::Notification m_playbackNotesChanged;
    std::vector<Note*> m_playback_notes;
    std::map<const Note*, int> m_ottava_map;
    
    Note* glissando_note = nullptr;
    int glissando_ticks = 0;
    int glissando_duration_ticks = 0;
    std::vector<Note*> glissando_endnotes;
    int glissando_curr_ticks = 0;
    muse::async::Notification m_glissandoEndNotesChanged;
    muse::async::Notification m_glissandoTickChanged;

    std::vector<muse::PointF> arpeggio_points;
    int arpeggio_ticks = 0;
    int arpeggio_duration_ticks = 0;
    std::vector<Note*> arpeggio_notes;
    bool arpeggio_is_down = false;
    int arpeggio_curr_ticks = 0;
    muse::async::Notification m_arpeggioNotesChanged;
    muse::async::Notification m_arpeggioTickChanged;

    Note* trill_note = nullptr;
    int trill_ticks = 0;
    int trill_duration_ticks = 0;
    int trill_tremolo_type = 0;
    int trill_curr_ticks = 0;
    Note* trill_note1 = nullptr;
    int trill_ticks1 = 0;
    int trill_duration_ticks1 = 0;
    int trill_tremolo_type1 = 0;
    int trill_curr_ticks1 = 0;
    muse::async::Notification m_trillNoteChanged;
    muse::async::Notification m_trillNoteChanged1;
    muse::async::Notification m_trillTickChanged;
    muse::async::Notification m_trillTickChanged1;

    muse::async::Notification m_clefKeySigsKeysChanged;
    std::set<uint> m_clefKeySigsKeys;
    bool m_isplaying = false;

    std::shared_ptr<NotationSelectionFilter> m_selectionFilter = nullptr;

    DragData m_dragData;
    muse::async::Notification m_dragChanged;
    std::vector<muse::LineF> m_anchorLines;

    QDrag* m_outgoingDrag = nullptr;

    mu::engraving::EditData m_editData;

    muse::async::Notification m_textEditingStarted;
    muse::async::Notification m_textEditingChanged;
    muse::async::Channel<TextBase*> m_textEditingEnded;
    muse::async::Channel<TextBase*> m_textAdded;

    DropData m_dropData;
    muse::async::Notification m_dropChanged;

    muse::async::Channel<ScoreConfigType> m_scoreConfigChanged;

    engraving::BspTree m_droppableTree;
    Page* m_currentDropPage = nullptr;

    mu::engraving::Lasso* m_lasso = nullptr;

    bool m_notifyAboutDropChanged = false;
    HitElementContext m_hitElementContext;

    muse::async::Channel<ShowItemRequest> m_showItemRequested;
};
}
