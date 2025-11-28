/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QIODevice>

#include "project/inotationwriter.h"
#include "../ilyricsexportconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::iex::lrcexport {
class LRCWriter : public project::INotationWriter
{
public:
    INJECT_STATIC(mu::iex::lrcexport::LyricsExportConfiguration, configuration)

public:
    // Interface implementation
    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType) const override;
    muse::Ret write(notation::INotationPtr, muse::io::IODevice&, const Options&) override;
    void writeMetadata(QIODevice& device, const engraving::Score* score) const;
    muse::Ret writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&) override;

private:
    // Core lyric functionality
    QMap<qreal, QString> collectLyrics(const engraving::Score*) const;
    QString formatTimestamp(qreal ms) const;
};
} // namespace mu::iex::lrcexport
