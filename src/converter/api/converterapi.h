/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "global/api/apiobject.h"

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "global/io/ifilesystem.h"
#include "../iconvertercontroller.h"

namespace mu::converter::api {
class ConverterApi : public muse::api::ApiObject, public muse::async::Asyncable
{
    Q_OBJECT

    muse::Inject<muse::IInteractive> interactive;
    muse::Inject<muse::io::IFileSystem> fileSystem;
    muse::Inject<IConverterController> converter;

public:
    explicit ConverterApi(muse::api::IApiEngine* e);

    // work with FS
    Q_INVOKABLE QString selectDir(const QString& title, const QString& dir = QString()) const;
    Q_INVOKABLE QStringList scanDir(const QString& dir, const QStringList& filters = QStringList()) const;
    Q_INVOKABLE QString basename(const QString& filePath) const;

    // convert
    Q_INVOKABLE bool batch(const QString& outDir, const QString& job, const QString& uriQuery, QJSValue progress = QJSValue());
};
}
