/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// INotationReader for .enc files: adapts the reader interface to the Encore importer.

#ifndef MU_IMPORTEXPORT_NOTATIONENCOREREADER_H
#define MU_IMPORTEXPORT_NOTATIONENCOREREADER_H

#include "modularity/ioc.h"
#include "project/inotationreader.h"
#include "../ienc-importconfiguration.h"

namespace mu::iex::enc {
class NotationEncoreReader : public project::INotationReader
{
public:
    muse::Ret read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options()) override;

private:
    muse::GlobalInject<IEncImportConfiguration> encoreConfiguration;
};
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_NOTATIONENCOREREADER_H
