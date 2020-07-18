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
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "../inotationreadersregister.h"
#include "retval.h"

#include "igetscore.h"
#include "notationinteraction.h"
#include "notationplayback.h"

namespace Ms {
class MScore;
class MasterScore;
}

namespace mu {
namespace domain {
namespace notation {
class Notation : public INotation, public IGetScore, public async::Asyncable
{
    INJECT(notation, INotationReadersRegister, readers)

public:
    Notation();
    ~Notation();

    //! NOTE Needed at the moment to initialize libmscore
    static void init();

    Ret load(const io::path& path) override;
    Ret load(const io::path& path, const std::shared_ptr<INotationReader>& reader) override;
    io::path path() const override;

    Ret createNew(const ScoreCreateOptions& scoreOptions) override;

    void setViewSize(const QSizeF& vs) override;
    void paint(QPainter* p, const QRect& r) override;

    // Input (mouse)
    INotationInteraction* interaction() const override;

    // midi
    INotationPlayback* playback() const override;

    // notify
    async::Notification notationChanged() const override;

    // internal
    Ms::Score* score() const;
    QSizeF viewSize() const;

private:

    friend class NotationInteraction;

    Ret doLoadScore(Ms::MasterScore* score,const io::path& path,const std::shared_ptr<INotationReader>& reader) const;
    void notifyAboutNotationChanged();

    mu::RetVal<Ms::MasterScore*> newScore(const ScoreCreateOptions& scoreInfo);

    QSizeF m_viewSize;
    Ms::MScore* m_scoreGlobal = nullptr;
    Ms::MasterScore* m_score = nullptr;
    NotationInteraction* m_interaction = nullptr;
    NotationPlayback* m_playback = nullptr;
    async::Notification m_notationChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATION_H
