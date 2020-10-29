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
#ifndef MU_NOTATION_NOTATION_H
#define MU_NOTATION_NOTATION_H

#include "inotation.h"
#include "igetscore.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "inotationconfiguration.h"

namespace Ms {
class MScore;
class Score;
}

namespace mu {
namespace notation {
class NotationInteraction;
class NotationPlayback;
class Notation : virtual public INotation, public IGetScore, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    explicit Notation(Ms::Score* score = nullptr);
    ~Notation() override;

    static void init();

    Meta metaInfo() const override;

    void setViewSize(const QSizeF& vs) override;
    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;
    void paint(QPainter* painter, const QRectF& frameRect) override;
    QRectF previewRect() const override;

    ValCh<bool> opened() const override;
    void setOpened(bool opened) override;

    INotationInteractionPtr interaction() const override;
    INotationMidiInputPtr midiInput() const override;
    INotationUndoStackPtr undoStack() const override;
    INotationElementsPtr elements() const override;
    INotationStylePtr style() const override;
    INotationPlaybackPtr playback() const override;
    INotationAccessibilityPtr accessibility() const override;
    INotationPartsPtr parts() const override;

    async::Notification notationChanged() const override;

protected:
    Ms::Score* score() const override;
    void setScore(Ms::Score* score);
    Ms::MScore* scoreGlobal() const;

private:
    friend class NotationInteraction;

    void paintPages(QPainter* painter, const QRectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const;
    void paintPageBorder(QPainter* painter, const Ms::Page* page) const;
    void paintElements(QPainter* painter, const QList<Element*>& elements) const;

    QSizeF viewSize() const;

    void notifyAboutNotationChanged();

    QSizeF m_viewSize;
    Ms::MScore* m_scoreGlobal = nullptr;
    Ms::Score* m_score = nullptr;
    ValCh<bool> m_opened;

    INotationInteractionPtr m_interaction;
    INotationPlaybackPtr m_playback;
    INotationUndoStackPtr m_undoStack;
    INotationStylePtr m_style;
    INotationMidiInputPtr m_midiInput;
    INotationAccessibilityPtr m_accessibility;
    INotationElementsPtr m_elements;
    INotationPartsPtr m_parts;

    async::Notification m_notationChanged;
};
}
}

#endif // MU_NOTATION_NOTATION_H
