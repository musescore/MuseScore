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
#ifndef MU_NOTATION_MASTERNOTATION_H
#define MU_NOTATION_MASTERNOTATION_H

#include <memory>

#include "../imasternotation.h"

#include "modularity/ioc.h"
#include "notation.h"
#include "retval.h"

#include "project/projecttypes.h"

namespace Ms {
class MasterScore;
}

namespace mu::project {
class NotationProject;
}

namespace mu::notation {
class MasterNotation : public IMasterNotation, public Notation, public std::enable_shared_from_this<MasterNotation>
{
public:
    ~MasterNotation();

    void setMasterScore(Ms::MasterScore* masterScore);
    Ret setupNewScore(Ms::MasterScore* score, Ms::MasterScore* templateScore, const ScoreCreateOptions& scoreOptions);
    void onSaveCopy();

    INotationPtr notation() override;

    Meta metaInfo() const override;
    void setMetaInfo(const Meta& meta) override;

    RetVal<bool> created() const override;

    mu::ValNt<bool> needSave() const override;

    IExcerptNotationPtr newExcerptNotation() const override;
    ValCh<ExcerptNotationList> excerpts() const override;
    void setExcerpts(const ExcerptNotationList& excerpts) override;

    INotationPartsPtr parts() const override;
    INotationPtr clone() const override;

private:

    friend class project::NotationProject;
    explicit MasterNotation();

    Ms::MasterScore* masterScore() const;

    void doSetExcerpts(ExcerptNotationList excerpts);

    void initExcerpts(const QList<Ms::Excerpt*>& scoreExcerpts = QList<Ms::Excerpt*>());

    void createNonexistentExcerpts(const ExcerptNotationList& newExcerpts);

    void updateExcerpts();
    IExcerptNotationPtr createExcerpt(Ms::Part* part);

    ValCh<ExcerptNotationList> m_excerpts;
};

using MasterNotationPtr = std::shared_ptr<MasterNotation>;
}

#endif // MU_NOTATION_MASTERNOTATION_H
