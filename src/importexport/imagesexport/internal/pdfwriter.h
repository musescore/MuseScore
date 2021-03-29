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

#ifndef MU_IMPORTEXPORT_PDFWRITER_H
#define MU_IMPORTEXPORT_PDFWRITER_H

#include "notation/abstractnotationwriter.h"

#include "../iimagesexportconfiguration.h"
#include "modularity/ioc.h"

namespace mu::iex::imagesexport {
class PdfWriter : public notation::AbstractNotationWriter
{
    INJECT(iex_imagesexport, IImagesExportConfiguration, configuration)

public:
    std::vector<INotationWriter::UnitType> supportedUnitTypes() const override;
    Ret write(const notation::INotationPtr notation, system::IODevice& destinationDevice, const Options& options = Options()) override;
    Ret writeList(const notation::INotationPtrList& notations, system::IODevice& destinationDevice,
                  const Options& options = Options()) override;

private:
    QString documentTitle(const Ms::Score& score) const;
};
}

#endif // MU_IMPORTEXPORT_PDFWRITER_H
