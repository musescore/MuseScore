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
#ifndef MU_PROJECT_PROJECTAUDIOSETTINGS_H
#define MU_PROJECT_PROJECTAUDIOSETTINGS_H

#include <memory>

#include "../iprojectaudiosettings.h"

#include "ret.h"
#include "engraving/io/mscreader.h"
#include "engraving/io/mscwriter.h"

namespace mu::project {
class ProjectAudioSettings : public IProjectAudioSettings
{
public:

    int someValue() const override;
    void setSomeValue(int val) override;

    //! NOTE Used for new or imported project (score)
    void makeDefault();

    Ret read(const engraving::MscReader& reader);
    Ret write(engraving::MscWriter& writer);

private:

    friend class NotationProject;
    ProjectAudioSettings() = default;

    int m_someValue = 0;
};

using ProjectAudioSettingsPtr = std::shared_ptr<ProjectAudioSettings>;
}

#endif // MU_PROJECT_PROJECTAUDIOSETTINGS_H
