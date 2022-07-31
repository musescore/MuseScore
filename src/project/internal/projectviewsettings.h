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

#ifndef MU_PROJECT_PROJECTVIEWSETTINGS_H
#define MU_PROJECT_PROJECTVIEWSETTINGS_H

#include "../iprojectviewsettings.h"

#include "types/ret.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"

namespace mu::project {
class ProjectViewSettings : public IProjectViewSettings
{
public:
    notation::ViewMode masterNotationViewMode() const override;
    void setMasterNotationViewMode(const notation::ViewMode& mode) override;

    notation::ViewMode excerptViewMode(size_t excerptIndex) const override;
    void setExcerptViewMode(size_t excerptIndex, const notation::ViewMode& mode) override;
    void removeExcerpt(size_t excerptIndex) override;

    mu::ValNt<bool> needSave() const override;

    Ret read(const engraving::MscReader& reader);
    Ret write(engraving::MscWriter& writer);

    //! NOTE Used for new or imported project (score)
    void makeDefault();

private:
    friend class NotationProject;
    ProjectViewSettings() = default;

    void setNeedSave(bool needSave);

    notation::ViewMode m_masterViewMode = notation::ViewMode::PAGE;
    std::vector<notation::ViewMode> m_excerptsViewModes;

    bool m_needSave = false;
    async::Notification m_needSaveNotification;
};
using ProjectViewSettingsPtr = std::shared_ptr<ProjectViewSettings>;
}

#endif // MU_PROJECT_PROJECTVIEWSETTINGS_H
