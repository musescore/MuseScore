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

namespace Ms {
class MScore;
class Score;
}

namespace mu {
namespace notation {
class NotationInteraction;
class NotationPlayback;
class NotationMidiInput;
class Notation : virtual public INotation, public IGetScore, public async::Asyncable
{
public:
    explicit Notation(Ms::Score* score = nullptr);
    ~Notation() override;

    static void init();

    Meta metaInfo() const override;

    void setViewSize(const QSizeF& vs) override;
    void paint(QPainter* painter) override;
    QRectF previewRect() const override;

    INotationInteraction* interaction() const override;

    INotationMidiInput* midiInput() const override;

    INotationUndoStack* undoStack() const override;

    INotationElements* elements() const override;

    INotationStyle* style() const override;

    INotationPlayback* playback() const override;

    async::Notification notationChanged() const override;

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
    NotationMidiInput* m_midiInput = nullptr;
    INotationAccessibility* m_accessibility = nullptr;
    INotationElements* m_elements = nullptr;

    async::Notification m_notationChanged;
};
}
}

#endif // MU_NOTATION_NOTATION_H
