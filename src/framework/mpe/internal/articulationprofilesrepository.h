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

#ifndef MUSE_MPE_ARTICULATIONPROFILESREPOSITORY_H
#define MUSE_MPE_ARTICULATIONPROFILESREPOSITORY_H

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "async/asyncable.h"

#include "iarticulationprofilesrepository.h"

namespace muse::mpe {
class ArticulationProfilesRepository : public IArticulationProfilesRepository, public Injectable, public async::Asyncable
{
    Inject<io::IFileSystem> fileSystem = { this };

public:
    ArticulationProfilesRepository(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    ArticulationsProfilePtr createNew() const override;
    ArticulationsProfilePtr defaultProfile(const ArticulationFamily family) const override;
    ArticulationsProfilePtr loadProfile(const io::path_t& path) const override;
    void saveProfile(const io::path_t& path, const ArticulationsProfilePtr profilePtr) override;
    async::Channel<io::path_t> profileChanged() const override;

private:
    std::vector<ArticulationFamily> supportedFamiliesFromJson(const QJsonArray& array) const;
    QJsonArray supportedFamiliesToJson(const std::vector<ArticulationFamily>& families) const;

    ArticulationPattern patternsScopeFromJson(const QJsonArray& array) const;
    QJsonArray patternsScopeToJson(const ArticulationPattern& scope) const;

    ArrangementPattern arrangementPatternFromJson(const QJsonObject& obj) const;
    QJsonObject arrangementPatternToJson(const ArrangementPattern& pattern) const;

    PitchPattern pitchPatternFromJson(const QJsonObject& obj) const;
    QJsonObject pitchPatternToJson(const PitchPattern& pattern) const;

    ExpressionPattern expressionPatternFromJson(const QJsonObject& obj) const;
    QJsonObject expressionPatternToJson(const ExpressionPattern& pattern) const;

    mutable std::unordered_map<ArticulationFamily, ArticulationsProfilePtr> m_defaultProfiles;

    async::Channel<io::path_t> m_profileChanged;
};
}

#endif // MUSE_MPE_ARTICULATIONPROFILESREPOSITORY_H
