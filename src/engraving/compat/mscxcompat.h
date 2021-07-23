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
#ifndef MU_ENGRAVING_MSCXCOMPAT_H
#define MU_ENGRAVING_MSCXCOMPAT_H

#include <QByteArray>
#include "libmscore/masterscore.h"
#include "engravingproject.h"

namespace mu::engraving::compat {
Ms::Score::FileError mscxToMscz(const QString& mscxFilePath, QByteArray* msczData);
Ms::Score::FileError loadMsczOrMscx(Ms::MasterScore* score, const QString& path, bool ignoreVersionError = false);
Err loadMsczOrMscx(EngravingProjectPtr project, const QString& path, bool ignoreVersionError = false);
}

#endif // MU_ENGRAVING_MSCXCOMPAT_H
