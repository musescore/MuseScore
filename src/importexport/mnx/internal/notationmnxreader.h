/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "project/inotationreader.h"
#include "types/bytearray.h"
#include "modularity/ioc.h"
#include "importexport/mnx/imnxconfiguration.h"

namespace mu::iex::mnxio {
class NotationMnxReader : public project::INotationReader, public muse::Injectable
{
public:
    NotationMnxReader(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    muse::Ret read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options()) override;

private:
    muse::Ret importJson(mu::engraving::MasterScore* score, muse::ByteArray&& jsonData, const muse::io::path_t& path) const;
    muse::Ret importMnx(mu::engraving::MasterScore* score, muse::ByteArray&& mnxData, const muse::io::path_t& path) const;
    const IMnxConfiguration* mnxConfiguration() const;

    muse::GlobalInject<IMnxConfiguration> m_mnxConfiguration;
};
}
