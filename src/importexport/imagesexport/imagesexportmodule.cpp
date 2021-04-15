/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "imagesexportmodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationwritersregister.h"
#include "internal/pdfwriter.h"
#include "internal/pngwriter.h"
#include "internal/svgwriter.h"

#include "internal/imagesexportconfiguration.h"

using namespace mu::iex::imagesexport;
using namespace mu::notation;

static std::shared_ptr<ImagesExportConfiguration> s_configuration = std::make_shared<ImagesExportConfiguration>();

std::string ImagesExportModule::moduleName() const
{
    return "iex_imagesexport";
}

void ImagesExportModule::registerExports()
{
    framework::ioc()->registerExport<IImagesExportConfiguration>(moduleName(), s_configuration);
}

void ImagesExportModule::resolveImports()
{
    auto writers = framework::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "pdf" }, std::make_shared<PdfWriter>());
        writers->reg({ "svg" }, std::make_shared<SvgWriter>());
        writers->reg({ "png" }, std::make_shared<PngWriter>());
    }
}

void ImagesExportModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
}
