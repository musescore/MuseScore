/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "project/iprojectwriter.h"

#include "../ilyricsexportconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::iex::lrcexport {
class LRCWriter : public project::IProjectWriter, public muse::Contextable
{
public:
    muse::GlobalInject<mu::iex::lrcexport::ILyricsExportConfiguration> configuration;

public:
    LRCWriter(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx)
    {
    }

    // Interface implementation
    std::vector<project::WriteUnitType> supportedUnitTypes() const override;
    bool supportsUnitType(project::WriteUnitType) const override;
    muse::Ret write(project::INotationProjectPtr project, muse::io::IODevice& device,
                    const project::WriteOptions& options = project::WriteOptions()) override;
    muse::Ret write(project::INotationProjectPtr project, const muse::io::path_t& filePath,
                    const project::WriteOptions& options = project::WriteOptions()) override;
    void writeMetadata(muse::io::IODevice* device, const engraving::Score* score) const;
    bool writeScore(mu::engraving::Score* score, const muse::io::path_t& path, bool enhancedLrc);

private:
    muse::Ret doWrite(mu::engraving::Score* score, muse::io::IODevice*, bool);

    // Core lyric functionality
    std::map<double, QString> collectLyrics(const mu::engraving::Score*);
    QString formatTimestamp(double ms) const;
    void findTrackAndLyricToExport(const engraving::Score* score, mu::engraving::track_idx_t& trackNumber, int& lyricNumber);
};
} // namespace mu::iex::lrcexport
