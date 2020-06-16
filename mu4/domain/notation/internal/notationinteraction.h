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
#ifndef MU_DOMAIN_NOTATIONINTERACTION_H
#define MU_DOMAIN_NOTATIONINTERACTION_H

#include <memory>
#include <vector>
#include <QPointF>
#include <QLineF>

#include "modularity/ioc.h"

#include "../inotationinteraction.h"
#include "../inotationconfiguration.h"

#include "igetscore.h"
#include "scorecallbacks.h"
#include "notationinputstate.h"
#include "notationselection.h"

#include "libmscore/element.h"
#include "libmscore/elementgroup.h"

namespace Ms {
class ShadowNote;
}

namespace mu {
namespace domain {
namespace notation {
class Notation;
class NotationInteraction : public INotationInteraction
{
    INJECT(notation, INotationConfiguration, configuration)
public:
    NotationInteraction(Notation* notation);
    ~NotationInteraction();

    void paint(QPainter* p);

    // Put notes
    void startNoteEntry() override;
    void endNoteEntry() override;
    void padNote(const Pad& pad) override;
    void putNote(const QPointF& pos, bool replace, bool insert) override;
    INotationInputState* inputState() const override;
    async::Notification inputStateChanged() const override;

    // Shadow note
    void showShadowNote(const QPointF& p) override;
    void hideShadowNote() override;
    void paintShadowNote(QPainter* p) override;

    // Select
    Element* hitElement(const QPointF& pos, float width) const override;
    void select(Element* e, SelectType type, int staffIdx = 0) override;
    INotationSelection* selection() const override;
    async::Notification selectionChanged() const override;

    // Drag
    bool isDragStarted() const override;
    void startDrag(const std::vector<Element*>& elems, const QPointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode) override;
    void endDrag() override;
    async::Notification dragChanged() override;

private:

    Ms::Score* score() const;

    void selectFirstTopLeftOrLast();

    Ms::Page* point2page(const QPointF& p) const;
    QList<Element*> hitElements(const QPointF& p_in, float w) const;
    static bool elementIsLess(const Ms::Element* e1, const Ms::Element* e2);

    void setAnchorLines(const std::vector<QLineF>& anchorList);
    void resetAnchorLines();
    void drawAnchorLines(QPainter* painter);

    struct DragData
    {
        QPointF beginMove;
        QPointF elementOffset;
        Ms::EditData editData;
        std::vector<Element*> elements;
        std::vector<std::unique_ptr<Ms::ElementGroup> > dragGroups;
        void reset();
    };

    Notation* m_notation = nullptr;
    ScoreCallbacks* m_scoreCallbacks = nullptr;

    NotationInputState* m_inputState = nullptr;
    async::Notification m_inputStateChanged;
    Ms::ShadowNote* m_shadowNote = nullptr;

    NotationSelection* m_selection = nullptr;
    async::Notification m_selectionChanged;

    DragData m_dragData;
    async::Notification m_dragChanged;
    std::vector<QLineF> m_anchorLines;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONINTERACTION_H
