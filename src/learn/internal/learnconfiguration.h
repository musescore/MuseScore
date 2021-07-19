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
#ifndef MU_LEARN_LEARNCONFIGURATION_H
#define MU_LEARN_LEARNCONFIGURATION_H

#include "ilearnconfiguration.h"

namespace mu::learn {
class LearnConfiguration : public ILearnConfiguration
{
public:
    network::RequestHeaders headers() const override;

    QUrl startedPlaylistUrl() const override;
    QUrl advancedPlaylistUrl() const override;

    QUrl videosInfoUrl(const std::vector<std::string>& videosIds) const override;
    QUrl videoOpenUrl(const std::string& videoId) const override;

private:
    QString apiRootUrl() const;

    QString playlistItemsParams(const QString& playlistId) const;
    QString videosParams(const std::vector<std::string>& videosIds) const;
};
}

#endif // MU_LEARN_LEARNCONFIGURATION_H
