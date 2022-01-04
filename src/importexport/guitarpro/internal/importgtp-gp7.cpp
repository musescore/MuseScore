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
#include "importgtp.h"

#include <QDebug>

#include "gtp/gp7dombuilder.h"
#include "libmscore/factory.h"
#include "libmscore/bracketItem.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"

#include "thirdparty/qzip/qzipreader_p.h"

using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro7::read(QFile* fp)
{
    f = fp;
    previousTempo = -1;
    MQZipReader zip(fp);
    QByteArray fileData = zip.fileData("Content/score.gpif");
    zip.close();
    readGpif(&fileData);
    return true;
}

std::unique_ptr<IGPDomBuilder> GuitarPro7::createGPDomBuilder() const
{
    return std::make_unique<GP7DomBuilder>();
}
}
