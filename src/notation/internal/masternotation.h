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
#include "types/retval.h"
#include "project/projecttypes.h"

#include "notation.h"
#include "../imasternotation.h"

namespace mu::engraving {
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

    void setMasterScore(mu::engraving::MasterScore* masterScore);
    Ret setupNewScore(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions);
    void applyOptions(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions, bool createdFromTemplate = false);

    INotationPtr notation() override;

    mu::ValNt<bool> needSave() const override;

    IExcerptNotationPtr createEmptyExcerpt(const QString& name = QString()) const override;

    ValCh<ExcerptNotationList> excerpts() const override;
    const ExcerptNotationList& potentialExcerpts() const override;

    void initExcerpts(const ExcerptNotationList& excerpts) override;
    void addExcerpts(const ExcerptNotationList& excerpts) override;
    void removeExcerpts(const ExcerptNotationList& excerpts) override;
    void sortExcerpts(ExcerptNotationList& excerpts) override;

    void setExcerptIsOpen(const INotationPtr excerptNotation, bool open) override;

    INotationPartsPtr parts() const override;
    bool hasParts() const override;
    async::Notification hasPartsChanged() const override;

    INotationPlaybackPtr playback() const override;

private:

    friend class project::NotationProject;
    explicit MasterNotation();

    mu::engraving::MasterScore* masterScore() const;

    void updatePotentialExcerpts() const;
    void initExcerptNotations(const std::vector<mu::engraving::Excerpt*>& excerpts);
    void addExcerptsToMasterScore(const std::vector<mu::engraving::Excerpt*>& excerpts);
    void doSetExcerpts(ExcerptNotationList excerpts);
    void updateExcerpts();
    void unloadExcerpts(ExcerptNotationList& excerpts);

    bool containsExcerpt(const mu::engraving::Excerpt* excerpt) const;
    bool containsExcerptForPart(const Part* part) const;

    void notifyAboutNeedSaveChanged();

    void markScoreAsNeedToSave();

    ValCh<ExcerptNotationList> m_excerpts;
    INotationPlaybackPtr m_notationPlayback = nullptr;
    async::Notification m_needSaveNotification;
    async::Notification m_hasPartsChanged;

    mutable bool m_needUpdatePotentialExcerpts = false;
    mutable ExcerptNotationList m_potentialExcerpts;
};

using MasterNotationPtr = std::shared_ptr<MasterNotation>;
}

#endif // MU_NOTATION_MASTERNOTATION_H
