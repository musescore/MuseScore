/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "async/notification.h"

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

    muse::Ret setupNewScore(engraving::MasterScore* score, const ScoreCreateOptions& options) override;
    void applyOptions(engraving::MasterScore* score, const ScoreCreateOptions& options, bool createdFromTemplate = false) override;
    engraving::MasterScore* masterScore() const override;
    void setMasterScore(engraving::MasterScore* masterScore) override;

    INotationPtr notation() override;
    int mscVersion() const override;

    IExcerptNotationPtr createEmptyExcerpt(const QString& name = QString()) const override;

    const ExcerptNotationList& excerpts() const override;
    muse::async::Notification excerptsChanged() const override;
    const ExcerptNotationList& potentialExcerpts() const override;

    void initExcerpts(const ExcerptNotationList& excerpts) override;
    void setExcerpts(const ExcerptNotationList& excerpts) override;
    void resetExcerpt(IExcerptNotationPtr excerptNotation) override;
    void sortExcerpts(ExcerptNotationList& excerpts) override;

    void setExcerptIsOpen(const INotationPtr excerptNotation, bool open) override;

    INotationPartsPtr parts() const override;
    bool hasParts() const override;
    muse::async::Notification hasPartsChanged() const override;

    INotationPlaybackPtr playback() const override;
    void initNotationSoloMuteState(const INotationPtr notation) override;

private:

    friend class NotationCreator;
    explicit MasterNotation(const muse::modularity::ContextPtr& iocCtx);

    void initAfterSettingScore(const engraving::MasterScore* score);

    void initExcerptNotations(const std::vector<engraving::Excerpt*>& excerpts);
    void addExcerptsToMasterScore(const std::vector<engraving::Excerpt*>& excerpts);
    void doSetExcerpts(const ExcerptNotationList& excerpts);
    void updateExcerpts();
    void updatePotentialExcerpts() const;
    void unloadExcerpts(ExcerptNotationList& excerpts);

    bool containsExcerpt(const engraving::Excerpt* excerpt) const;

    void onPartsChanged();

    void notifyAboutNeedSaveChanged();

    void markScoreAsNeedToSave();

    ExcerptNotationList m_excerpts;
    muse::async::Notification m_excerptsChanged;
    INotationPlaybackPtr m_notationPlayback = nullptr;
    muse::async::Notification m_hasPartsChanged;

    mutable ExcerptNotationList m_potentialExcerpts;

    // When the user first removes instruments (`Parts`) and then adds new ones,
    // the new ones might have the same ID as the removed ones. In this case,
    // we need to regenerate potential excerpts, even though for all part IDs a
    // potential excerpt already exists.
    mutable bool m_potentialExcerptsForcedDirty = false;
};

using MasterNotationPtr = std::shared_ptr<MasterNotation>;
}

#endif // MU_NOTATION_MASTERNOTATION_H
