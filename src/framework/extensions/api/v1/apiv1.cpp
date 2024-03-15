/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "apiv1.h"

#include <QtQml>

#include "messagedialog.h"
#include "filedialog.h"
#include "qqmlsettings_p.h"

#include "util.h"

using namespace mu::extensions::apiv1;

void ApiV1::registerQmlTypes()
{
    qmlRegisterType<MsProcess>("MuseScore", 3, 0, "QProcess");
    qmlRegisterType<FileIO, 1>("FileIO",    3, 0, "FileIO");

    qmlRegisterUncreatableType<StandardButton>("MuseScore", 3, 0, "StandardButton", "Cannot create an enumeration");
    qmlRegisterType<MessageDialog>("MuseScore", 3, 0, "MessageDialog");
    qmlRegisterType<QQmlSettings>("MuseScore", 3, 0, "Settings");
    qmlRegisterType<FileDialog>("MuseScore", 3, 0, "FileDialog");
}
