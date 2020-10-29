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
#ifndef MU_NOTATION_NOTATIONINTERACTION_H
#define MU_NOTATION_NOTATIONINTERACTION_H

#include <memory>
#include <vector>
#include <QPointF>
#include <QLineF>

#include "modularity/ioc.h"

#include "../inotationinteraction.h"
#include "../inotationconfiguration.h"

#include "scorecallbacks.h"
#include "notationinputstate.h"
#include "notationselection.h"

#include "libmscore/element.h"
#include "libmscore/elementgroup.h"

namespace Ms {
class ShadowNote;
}

namespace mu {
namespace notation {
class Notation;
class NotationInteraction : public INotationInteraction
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    NotationInteraction(Notation* notation);
    ~NotationInteraction() override;

    void init();
    void paint(QPainter* p);

    // Put notes
    void startNoteEntry() override;
    void endNoteEntry() override;
    void padNote(const Pad& pad) override;
    void putNote(const QPointF& pos, bool replace, bool insert) override;
    async::Notification noteAdded() const override;
    INotationInputStatePtr inputState() const override;
    async::Notification inputStateChanged() const override;

    // Shadow note
    void showShadowNote(const QPointF& p) override;
    void hideShadowNote() override;
    void paintShadowNote(QPainter* p) override;

    // Select
    Element* hitElement(const QPointF& pos, float width) const override;
    void select(Element* element, SelectType type, int staffIndex = 0) override;
    INotationSelectionPtr selection() const override;
    void clearSelection() override;
    async::Notification selectionChanged() const override;

    // Drag
    bool isDragStarted() const override;
    void startDrag(const std::vector<Element*>& elems, const QPointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode) override;
    void endDrag() override;
    async::Notification dragChanged() const override;

    // Drop
    void startDrop(const QByteArray& edata) override;
    bool isDropAccepted(const QPointF& pos, Qt::KeyboardModifiers modifiers) override;
    bool drop(const QPointF& pos, Qt::KeyboardModifiers modifiers) override;
    void endDrop() override;
    async::Notification dropChanged() const override;

    bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) override;

    // Move
    //! NOTE Perform operations on selected elements
    void moveSelection(MoveDirection d, MoveSelectionType type) override;
    void movePitch(MoveDirection d, PitchMode mode) override; //! NOTE Requires a note to be selected
    void moveText(MoveDirection d, bool quickly) override;    //! NOTE Requires a text element to be selected

    // Text edit
    bool isTextEditingStarted() const override;
    void startEditText(Element* element, const QPointF& cursorPos) override;
    void editText(QKeyEvent* event) override;
    void endEditText() override;
    void changeTextCursorPosition(const QPointF& newCursorPos) override;
    async::Notification textEditingChanged() const override;

    void deleteSelection() override;

private:
    Ms::Score* score() const;

    void selectFirstTopLeftOrLast();

    Ms::Page* point2page(const QPointF& p) const;
    QList<Element*> hitElements(const QPointF& p_in, float w) const;
    QList<Element*> elementsAt(const QPointF& p) const;
    Element* elementAt(const QPointF& p) const;
    static bool elementIsLess(const Ms::Element* e1, const Ms::Element* e2);

    void setAnchorLines(const std::vector<QLineF>& anchorList);
    void resetAnchorLines();
    void drawAnchorLines(QPainter* painter);
    void drawTextEditMode(QPainter* painter);
    void moveElementSelection(MoveDirection d);

    Element* dropTarget(Ms::EditData& ed) const;
    bool dragMeasureAnchorElement(const QPointF& pos);
    bool dragTimeAnchorElement(const QPointF& pos);
    void setDropTarget(Element* el);
    bool dropCanvas(Element* e);

    void applyDropPaletteElement(Ms::Score* score, Ms::Element* target, Ms::Element* e, Qt::KeyboardModifiers modifiers,
                                 QPointF pt = QPointF(), bool pasteMode = false);
    void cmdAddSlur(const Ms::Slur* slurTemplate = nullptr);
    void addSlur(Ms::ChordRest* cr1, Ms::ChordRest* cr2, const Ms::Slur* slurTemplate);

    struct DragData
    {
        QPointF beginMove;
        QPointF elementOffset;
        Ms::EditData ed;
        std::vector<Element*> elements;
        std::vector<std::unique_ptr<Ms::ElementGroup> > dragGroups;
        void reset();
    };

    struct DropData
    {
        Ms::EditData ed;
        Element* dropTarget = nullptr;
    };

    Notation* m_notation = nullptr;
    ScoreCallbacks* m_scoreCallbacks = nullptr;

    async::Notification m_noteAdded;
    INotationInputStatePtr m_inputState = nullptr;
    async::Notification m_inputStateChanged;
    Ms::ShadowNote* m_shadowNote = nullptr;

    INotationSelectionPtr m_selection = nullptr;
    async::Notification m_selectionChanged;

    DragData m_dragData;
    async::Notification m_dragChanged;
    std::vector<QLineF> m_anchorLines;

    Ms::EditData m_textEditData;
    async::Notification m_textEditingChanged;

    DropData m_dropData;
    async::Notification m_dropChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONINTERACTION_H
