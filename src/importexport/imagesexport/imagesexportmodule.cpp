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
#include "imagesexportmodule.h"

#include "modularity/ioc.h"

#include "project/inotationwritersregister.h"
#include "internal/pdfwriter.h"
#include "internal/pngwriter.h"
#include "internal/svgwriter.h"

#include "internal/imagesexportconfiguration.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;

std::string ImagesExportModule::moduleName() const
{
    return "iex_imagesexport";
}

void ImagesExportModule::registerExports()
{
    m_configuration = std::make_shared<ImagesExportConfiguration>();

    modularity::ioc()->registerExport<IImagesExportConfiguration>(moduleName(), m_configuration);
}

void ImagesExportModule::resolveImports()
{
    auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "pdf" }, std::make_shared<PdfWriter>());
        writers->reg({ "svg" }, std::make_shared<SvgWriter>());
        writers->reg({ "png" }, std::make_shared<PngWriter>());
    }
}

void ImagesExportModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode == framework::IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}
