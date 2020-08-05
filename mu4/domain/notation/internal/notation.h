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
#ifndef MU_DOMAIN_NOTATION_H
#define MU_DOMAIN_NOTATION_H

#include "inotation.h"
#include "async/asyncable.h"
#include "igetscore.h"
#include "notationinteraction.h"
#include "notationplayback.h"

namespace Ms {
class MScore;
class Score;
}

namespace mu {
namespace domain {
namespace notation {
class Notation : public INotation, public IGetScore, public async::Asyncable
{
public:
    explicit Notation();
    ~Notation() override;

    static void init();

    void setViewSize(const QSizeF& vs) override;
    void paint(QPainter* p, const QRect& r) override;

    // Input (mouse)
    INotationInteraction* interaction() const override;

    INotationUndoStack* undoStack() const override;

    INotationStyle* style() const override;

    // midi
    INotationPlayback* playback() const override;

    // notify
    async::Notification notationChanged() const override;

    // accessibility
    INotationAccessibility* accessibility() const override;

protected:
    Ms::Score* score() const override;
    void setScore(Ms::Score* score);
    Ms::MScore* scoreGlobal() const;

private:
    friend class NotationInteraction;

    QSizeF viewSize() const;

    void notifyAboutNotationChanged();

    QSizeF m_viewSize;
    Ms::MScore* m_scoreGlobal = nullptr;
    Ms::Score* m_score = nullptr;
    NotationInteraction* m_interaction = nullptr;
    INotationUndoStack* m_undoStackController = nullptr;
    INotationStyle* m_style = nullptr;
    NotationPlayback* m_playback = nullptr;
    INotationAccessibility* m_accessibility = nullptr;
    async::Notification m_notationChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATION_H
