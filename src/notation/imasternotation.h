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
#ifndef MU_NOTATION_IMASTERNOTATION_H
#define MU_NOTATION_IMASTERNOTATION_H

#include "types/retval.h"

#include "inotation.h"
#include "iexcerptnotation.h"
#include "inotationplayback.h"

namespace mu::notation {
using ExcerptNotationList = std::vector<IExcerptNotationPtr>;

class IMasterNotation
{
public:

    virtual Ret setupNewScore(engraving::MasterScore* score, const ScoreCreateOptions& options) = 0;
    virtual void applyOptions(engraving::MasterScore* score, const ScoreCreateOptions& options, bool createdFromTemplate = false) = 0;
    virtual engraving::MasterScore* masterScore() const = 0;
    virtual void setMasterScore(engraving::MasterScore* masterScore) = 0;

    virtual INotationPtr notation() = 0;

    virtual int mscVersion() const = 0;

    virtual IExcerptNotationPtr createEmptyExcerpt(const QString& name = QString()) const = 0;

    virtual ValCh<ExcerptNotationList> excerpts() const = 0;
    virtual const ExcerptNotationList& potentialExcerpts() const = 0;

    virtual void initExcerpts(const ExcerptNotationList& excerpts) = 0;
    virtual void addExcerpts(const ExcerptNotationList& excerpts) = 0;
    virtual void removeExcerpts(const ExcerptNotationList& excerpts) = 0;
    virtual void resetExcerpt(IExcerptNotationPtr excerpt) = 0;
    virtual void sortExcerpts(ExcerptNotationList& excerpts) = 0;

    virtual void setExcerptIsOpen(const INotationPtr excerptNotation, bool opened) = 0;

    virtual INotationPartsPtr parts() const = 0;
    virtual bool hasParts() const = 0;
    virtual async::Notification hasPartsChanged() const = 0;

    virtual INotationPlaybackPtr playback() const = 0;

    virtual void setSaved(bool arg) = 0;
    virtual ValNt<bool> needSave() const = 0;
};

using IMasterNotationPtr = std::shared_ptr<IMasterNotation>;
}

#endif // MU_NOTATION_IMASTERNOTATION_H
