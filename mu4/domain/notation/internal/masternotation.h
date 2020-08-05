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
#ifndef MU_DOMAIN_MASTERNOTATION_H
#define MU_DOMAIN_MASTERNOTATION_H

#include "../imasternotation.h"
#include "modularity/ioc.h"
#include "../inotationreadersregister.h"
#include "notation.h"
#include "retval.h"

namespace Ms {
class MasterScore;
}

namespace mu {
namespace domain {
namespace notation {
class MasterNotation : public IMasterNotation, public Notation
{
    INJECT(notation, INotationReadersRegister, readers)

public:
    explicit MasterNotation();

    Meta metaInfo() const override;

    Ret load(const io::path& path) override;
    io::path path() const override;

    Ret createNew(const ScoreCreateOptions& scoreOptions) override;

    std::vector<INotationPtr> parts() const override;

private:
    Ms::MasterScore* masterScore() const;

    Ret load(const io::path& path, const INotationReaderPtr& reader);
    Ret doLoadScore(Ms::MasterScore* score, const io::path& path, const INotationReaderPtr& reader) const;
    mu::RetVal<Ms::MasterScore*> newScore(const ScoreCreateOptions& scoreInfo);
};
}
}
}

#endif // MU_DOMAIN_MASTERNOTATION_H
