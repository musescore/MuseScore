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

#include "../inotation.h"
#include "actions/action.h"

#include "notationinputstate.h"

namespace Ms {
class MScore;
class MasterScore;
class ShadowNote;
}

namespace mu {
namespace domain {
namespace notation {
class ScoreCallbacks;
class Notation : public INotation
{
public:
    Notation();
    ~Notation();

    //! NOTE Needed at the moment to initialize libmscore
    static void init();

    bool load(const std::string& path, const Params& params) override;
    void paint(QPainter* p, const QRect& r) override;

    INotationInputState* inputState() const;
    void startNoteEntry() override;
    void endNoteEntry() override;
    void padNote(const actions::ActionName& name) override;
    void putNote(const QPointF& pos, bool replace, bool insert) override;

    // shadow note
    void showShadowNote(const QPointF& p);
    void hideShadowNote();
    void paintShadowNote(QPainter* p);

private:

    Ms::MasterScore* score() const;

    Ms::MScore* m_scoreGlobal = nullptr;
    Ms::MasterScore* m_score = nullptr;
    ScoreCallbacks* m_scoreCallbacks = nullptr;
    Ms::ShadowNote* m_shadowNote = nullptr;
    NotationInputState* m_inputState = nullptr;
};
}
}
}

#endif // MU_DOMAIN_NOTATION_H
