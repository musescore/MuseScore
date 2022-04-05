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

#include "modularity/ioc.h"
#include "retval.h"
#include "project/projecttypes.h"

#include "notation.h"
#include "../imasternotation.h"

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
    Ret setupNewScore(Ms::MasterScore* score, const ScoreCreateOptions& scoreOptions);
    void applyOptions(Ms::MasterScore* score, const ScoreCreateOptions& scoreOptions, bool createdFromTemplate = false);

    INotationPtr notation() override;

    bool isNewlyCreated() const override;
    mu::ValNt<bool> needSave() const override;

    IExcerptNotationPtr newExcerptBlankNotation() const override;
    ValCh<ExcerptNotationList> excerpts() const override;
    ExcerptNotationList potentialExcerpts() const override;

    void addExcerpts(const ExcerptNotationList& excerpts) override;
    void removeExcerpts(const ExcerptNotationList& excerpts) override;

    void setExcerptIsOpen(const INotationPtr excerptNotation, bool open) override;

    INotationPartsPtr parts() const override;
    INotationPlaybackPtr playback() const override;

private:

    friend class project::NotationProject;
    explicit MasterNotation();

    Ms::MasterScore* masterScore() const;

    void initExcerptNotations(const QList<Ms::Excerpt*>& excerpts);
    void addExcerptsToMasterScore(const QList<Ms::Excerpt*>& excerpts);
    void doSetExcerpts(ExcerptNotationList excerpts);
    void updateExerpts();
    bool containsExcerpt(const Ms::Excerpt* excerpt) const;

    void notifyAboutNeedSaveChanged();

    void markScoreAsNeedToSave();

    ValCh<ExcerptNotationList> m_excerpts;
    INotationPlaybackPtr m_notationPlayback = nullptr;

    async::Notification m_needSaveNotification;
};

using MasterNotationPtr = std::shared_ptr<MasterNotation>;
}

#endif // MU_NOTATION_MASTERNOTATION_H
