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

#pragma once

#include "engraving/iengravingpluginapihelper.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iprojectfilescontroller.h"
#include "iexportprojectscenario.h"
#include "inotationwritersregister.h"

namespace mu::project {
class EngravingPluginAPIHelper : public engraving::IEngravingPluginAPIHelper, public muse::Contextable
{
    muse::GlobalInject<INotationWritersRegister> writers;
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<IProjectFilesController> projectFilesController = { this };
    muse::ContextInject<IExportProjectScenario> exportProjectScenario = { this };

public:
    EngravingPluginAPIHelper(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    bool writeScore(const QString& name, const QString& ext) override;
    engraving::Score* readScore(const QString& name) override;
    void closeScore() override;

private:
    std::optional<INotationWriter::UnitType> determineWriterUnitType(const std::string& ext) const;
};
}
