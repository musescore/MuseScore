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

#ifndef MU_IMPORTEXPORT_PDFWRITER_H
#define MU_IMPORTEXPORT_PDFWRITER_H

#include "abstractimagewriter.h"

#include "../iimagesexportconfiguration.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"

class QPdfWriter;

namespace mu::iex::imagesexport {
class PdfWriter : public AbstractImageWriter
{
    Inject<IImagesExportConfiguration> configuration = { this };
    Inject<muse::IApplication> application = { this };

public:
    PdfWriter(const muse::modularity::ContextPtr& iocCtx)
        : AbstractImageWriter(iocCtx) {}

    std::vector<project::INotationWriter::UnitType> supportedUnitTypes() const override;
    muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& dstDevice, const Options& options = Options()) override;
    muse::Ret writeList(const notation::INotationPtrList& notations, muse::io::IODevice& dstDevice,
                        const Options& options = Options()) override;

private:
    void preparePdfWriter(QPdfWriter& pdfWriter, const QString& title, const QSizeF& size) const;
};
}

#endif // MU_IMPORTEXPORT_PDFWRITER_H
