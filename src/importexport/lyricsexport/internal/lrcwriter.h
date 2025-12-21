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

#include "project/inotationwriter.h"

#include "../ilyricsexportconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::iex::lrcexport {
class LRCWriter : public project::INotationWriter
{
public:
    INJECT_STATIC(mu::iex::lrcexport::ILyricsExportConfiguration, configuration)

public:
    // Interface implementation
    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType) const override;
    muse::Ret write(notation::INotationPtr, muse::io::IODevice&, const Options&) override;
    void writeMetadata(muse::io::IODevice* device, const engraving::Score* score) const;
    muse::Ret writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&) override;

    bool writeScore(mu::engraving::Score* score, const muse::io::path_t& path, bool enhancedLrc);

private:
    muse::Ret doWrite(mu::engraving::Score* score, muse::io::IODevice*, bool);

    // Core lyric functionality
    std::map<double, QString> collectLyrics(const mu::engraving::Score*);
    QString formatTimestamp(double ms) const;
    void findTrackAndLyricToExport(const engraving::Score* score, mu::engraving::track_idx_t& trackNumber, int& lyricNumber);
};
} // namespace mu::iex::lrcexport
