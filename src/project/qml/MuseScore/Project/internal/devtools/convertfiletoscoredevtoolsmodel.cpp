/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "convertfiletoscoredevtoolsmodel.h"

#include <QUrl>

#include "actions/actiontypes.h"

using namespace mu::project;
using namespace muse;
using namespace muse::actions;

ConvertFileToScoreDevToolsModel::ConvertFileToScoreDevToolsModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ConvertFileToScoreDevToolsModel::selectAndConvertFiles()
{
    dispatcher()->dispatch("file-convert-to-score");
}

void ConvertFileToScoreDevToolsModel::filesDropped(const QStringList& urls)
{
    io::paths_t paths;
    paths.reserve(urls.size());
    for (const QString& url : urls) {
        paths.push_back(io::path_t(QUrl(url).toLocalFile()));
    }

    dispatcher()->dispatch("file-convert-to-score", ActionData::make_arg1<io::paths_t>(paths));
}
