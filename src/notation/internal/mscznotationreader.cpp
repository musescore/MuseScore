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
#include "mscznotationreader.h"

#include <QFile>
#include <QByteArray>
#include <QBuffer>

#include "libmscore/score.h"
#include "io/msczfile.h"
#include "notation/notationerrors.h"

using namespace mu::notation;
using namespace mu::engraving;

mu::Ret MsczNotationReader::read(Ms::MasterScore* score, const io::path& path, const Options& options)
{
    Ms::Score::FileError err;
    if (io::syffix(path) == "mscx") {
        //! NOTE Convert mscx -> mscz

        QFile mscxFile(path.toQString());
        if (mscxFile.open(QIODevice::ReadOnly)) {
            QByteArray mscxData = mscxFile.readAll();

            QByteArray msczData;
            QBuffer buf(&msczData);
            buf.open(QIODevice::ReadWrite);

            MsczFile msczFile(&buf);
            msczFile.setFilePath(path.toQString());
            msczFile.writeMscx(mscxData);

            err = score->loadMscz(msczFile, options.contains(OptionKey::ForceMode));
        } else {
            err = Ms::Score::FileError::FILE_OPEN_ERROR;
        }
    } else {
        err = score->loadMscz(path.toQString(), options.contains(OptionKey::ForceMode));
    }

    return mu::notation::scoreFileErrorToRet(err, path);
}
