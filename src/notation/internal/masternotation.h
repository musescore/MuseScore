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
#ifndef MU_NOTATION_MASTERNOTATION_H
#define MU_NOTATION_MASTERNOTATION_H

#include <memory>

#include "../imasternotation.h"
#include "../inotationreadersregister.h"
#include "../inotationwritersregister.h"

#include "modularity/ioc.h"
#include "notation.h"
#include "retval.h"

namespace Ms {
class MasterScore;
}

namespace mu::notation {
class MasterNotation : public IMasterNotation, public Notation, public std::enable_shared_from_this<MasterNotation>
{
    INJECT(notation, INotationReadersRegister, readers)
    INJECT(notation, INotationWritersRegister, writers)

public:
    explicit MasterNotation();

    INotationPtr notation() override;

    Meta metaInfo() const override;

    Ret load(const io::path& path) override;
    io::path path() const override;

    Ret createNew(const ScoreCreateOptions& scoreOptions) override;
    RetVal<bool> created() const override;

    Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) override;
    mu::ValNt<bool> needSave() const override;

    ValCh<ExcerptNotationList> excerpts() const override;
    void setExcerpts(const ExcerptNotationList& excerpts) override;

    INotationPartsPtr parts() const override;
    INotationPtr clone() const override;

private:
    Ret exportScore(const io::path& path, const std::string& suffix);

    Ms::MasterScore* masterScore() const;

    Ret load(const io::path& path, const INotationReaderPtr& reader);
    Ret doLoadScore(Ms::MasterScore* score, const io::path& path, const INotationReaderPtr& reader) const;
    mu::RetVal<Ms::MasterScore*> newScore(const ScoreCreateOptions& scoreInfo);

    void doSetExcerpts(ExcerptNotationList excerpts);

    void initExcerpts(const QList<Ms::Excerpt*>& scoreExcerpts = QList<Ms::Excerpt*>());

    void createNonexistentExcerpts(const ExcerptNotationList& newExcerpts);

    void updateExcerpts();
    IExcerptNotationPtr createExcerpt(Ms::Part* part);

    Ret saveScore(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save);
    Ret saveSelectionOnScore(const io::path& path = io::path());

    ValCh<ExcerptNotationList> m_excerpts;
    INotationPartsPtr m_parts;
};
}

#endif // MU_NOTATION_MASTERNOTATION_H
