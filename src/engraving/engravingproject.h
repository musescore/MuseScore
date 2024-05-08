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
#ifndef MU_ENGRAVING_ENGRAVINGPROJECT_H
#define MU_ENGRAVING_ENGRAVINGPROJECT_H

#include <memory>

#include "global/types/ret.h"
#include "infrastructure/mscreader.h"
#include "infrastructure/mscwriter.h"
#include "infrastructure/ifileinfoprovider.h"
#include "types/types.h"

#include "modularity/ioc.h"
#include "devtools/iengravingelementsprovider.h"

//! NOTE In addition to the score itself, the mscz file also stores other data,
//! such as synthesizer, mixer settings, omr, etc.
//! We should talk not just about the score, but about the Project.
//! So, the Engraving Project contains the score and all other data,
//! such as additional styles, sound settings, sound samples, and others.
//!
//! To simplify logic and reduce combinatorics,
//! we need to strive to ensure that there is work with the project everywhere;
//! accordingly, only the project should create and load the master score.

namespace mu::engraving {
class MasterScore;
class MStyle;

class EngravingProject : public std::enable_shared_from_this<EngravingProject>, public muse::Injectable
{
public:
    Inject<IEngravingElementsProvider> engravingElementsProvider = { this };

public:
    ~EngravingProject();

    static std::shared_ptr<EngravingProject> create(const muse::modularity::ContextPtr& iocCtx);
    static std::shared_ptr<EngravingProject> create(const MStyle& style, const muse::modularity::ContextPtr& iocCtx);

    IFileInfoProviderPtr fileInfoProvider() const;
    void setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider);

    String appVersion() const;
    int mscVersion() const;

    bool readOnly() const;

    MasterScore* masterScore() const;
    muse::Ret setupMasterScore(bool forceMode);

    muse::Ret loadMscz(const MscReader& msc, SettingsCompat& settingsCompat, bool ignoreVersionError);
    bool writeMscz(MscWriter& writer, bool onlySelection, bool createThumbnail);

    bool isCorruptedUponLoading() const;
    muse::Ret checkCorrupted() const;

private:
    friend class MasterScore;

    EngravingProject(const muse::modularity::ContextPtr& iocCtx);

    void init(const MStyle& style);

    muse::Ret doSetupMasterScore(bool forceMode);

    MasterScore* m_masterScore = nullptr;

    bool m_isCorruptedUponLoading = false;
};

using EngravingProjectPtr = std::shared_ptr<EngravingProject>;
}

#endif // MU_ENGRAVING_PROJECT_H
