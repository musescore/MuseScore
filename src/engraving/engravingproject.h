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
#ifndef MU_ENGRAVING_ENGRAVINGPROJECT_H
#define MU_ENGRAVING_ENGRAVINGPROJECT_H

#include <memory>

#include "engravingerrors.h"
#include "infrastructure/io/mscreader.h"
#include "infrastructure/io/mscwriter.h"
#include "infrastructure/io/ifileinfoprovider.h"

#include "modularity/ioc.h"
#include "diagnostics/iengravingelementsprovider.h"

//! NOTE In addition to the score itself, the mscz file also stores other data,
//! such as synthesizer, mixer settings, omr, etc.
//! We should talk not just about the score, but about the Project.
//! So, the Engraving Project contains the score and all other data,
//! such as additional styles, sound settings, sound samples, and others.
//!
//! To simplify logic and reduce combinatorics,
//! we need to strive to ensure that there is work with the project everywhere;
//! accordingly, only the project should create and load the master score.

namespace Ms {
class MasterScore;
class MStyle;
}

namespace mu::engraving {
class EngravingProject : public std::enable_shared_from_this<EngravingProject>
{
    INJECT(engraving, diagnostics::IEngravingElementsProvider, engravingElementsProvider)

public:
    ~EngravingProject();

    static std::shared_ptr<EngravingProject> create();
    static std::shared_ptr<EngravingProject> create(const Ms::MStyle& style);

    IFileInfoProviderPtr fileInfoProvider() const;
    void setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider);

    QString appVersion() const;
    int mscVersion() const;

    bool readOnly() const;

    Ms::MasterScore* masterScore() const;
    Err setupMasterScore();

    Err loadMscz(const mu::engraving::MscReader& msc, bool ignoreVersionError);
    bool writeMscz(mu::engraving::MscWriter& writer, bool onlySelection, bool createThumbnail);

private:
    friend class Ms::MasterScore;

    EngravingProject() = default;

    void init(const Ms::MStyle& style);

    Err doSetupMasterScore(Ms::MasterScore* score);

    Ms::MasterScore* m_masterScore = nullptr;
};

using EngravingProjectPtr = std::shared_ptr<EngravingProject>;
}

#endif // MU_ENGRAVING_PROJECT_H
