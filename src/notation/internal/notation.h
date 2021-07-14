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
#ifndef MU_NOTATION_NOTATION_H
#define MU_NOTATION_NOTATION_H

#include "../inotation.h"
#include "igetscore.h"
#include "inotationmidievents.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "../inotationconfiguration.h"

namespace Ms {
class MScore;
class Score;
}

namespace mu::notation {
class NotationInteraction;
class NotationPlayback;
class Notation : virtual public INotation, public IGetScore, public async::Asyncable
{
    INJECT_STATIC(notation, INotationConfiguration, configuration)

public:
    explicit Notation(Ms::Score* score = nullptr);
    ~Notation() override;

    static void init();

    Meta metaInfo() const override;
    void setMetaInfo(const Meta& meta) override;

    ScoreOrder scoreOrder() const override;

    INotationPtr clone() const override;

    void setViewSize(const QSizeF& vs) override;
    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;
    void paint(draw::Painter* painter, const RectF& frameRect) override;

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
    void notifyAboutNotationChanged();

    INotationPartsPtr m_parts = nullptr;

private:
    friend class NotationInteraction;

    void paintPages(mu::draw::Painter* painter, const RectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const;
    void paintPageBorder(mu::draw::Painter* painter, const Ms::Page* page) const;
    void paintForeground(mu::draw::Painter* painter, const RectF& pageRect) const;

    QSizeF viewSize() const;

    QSizeF m_viewSize;
    Ms::MScore* m_scoreGlobal = nullptr;
    Ms::Score* m_score = nullptr;
    ValCh<bool> m_opened;

    INotationInteractionPtr m_interaction = nullptr;
    INotationMidiEventsPtr m_midiEventsProvider = nullptr;
    INotationPlaybackPtr m_playback = nullptr;
    INotationUndoStackPtr m_undoStack = nullptr;
    INotationStylePtr m_style = nullptr;
    INotationMidiInputPtr m_midiInput = nullptr;
    INotationAccessibilityPtr m_accessibility = nullptr;
    INotationElementsPtr m_elements = nullptr;

    async::Notification m_notationChanged;
};
}

#endif // MU_NOTATION_NOTATION_H
