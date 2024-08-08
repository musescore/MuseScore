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
#ifndef MU_IMPORTEXPORT_GUITARPROREADER_H
#define MU_IMPORTEXPORT_GUITARPROREADER_H

#include "project/inotationreader.h"
#include "modularity/ioc.h"
#include "../iguitarproconfiguration.h"

namespace mu::iex::guitarpro {
class GuitarProReader : public project::INotationReader, public muse::Injectable
{
    muse::Inject<mu::iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration = { this };

public:
    GuitarProReader(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    muse::Ret read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options()) override;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GUITARPROREADER_H
