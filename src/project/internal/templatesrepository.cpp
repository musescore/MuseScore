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

#include "templatesrepository.h"

#include "io/path.h"
#include "translation.h"
#include "log.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace mu::project;

mu::RetVal<Templates> TemplatesRepository::templates() const
{
    TRACEFUNC;

    Templates templates;

    for (const io::path_t& dir : configuration()->availableTemplateDirs()) {
        templates << readTemplates(dir);
    }

    return RetVal<Templates>::make_ok(templates);
}

Templates TemplatesRepository::readTemplates(const io::path_t& dirPath) const
{
    TRACEFUNC;

    io::path_t categoriesJsonPath = configuration()->templateCategoriesJsonPath(dirPath);

    if (!fileSystem()->exists(categoriesJsonPath)) {
        RetVal<io::paths_t> files = fileSystem()->scanFiles(dirPath, { "*.mscz", "*.mscx" });
        if (!files.ret) {
            LOGE() << files.ret.toString();
            return Templates();
        }

        return readTemplates(files.val, qtrc("project", "My templates"));
    }

    RetVal<ByteArray> categoriesJson = fileSystem()->readFile(categoriesJsonPath);
    if (!categoriesJson.ret) {
        LOGE() << categoriesJson.ret.toString();
        return Templates();
    }

    QJsonDocument document = QJsonDocument::fromJson(categoriesJson.val.toQByteArrayNoCopy());
    QVariantList categoryObjList = document.array().toVariantList();

    Templates templates;

    for (const QVariant& obj : categoryObjList) {
        QVariantMap map = obj.toMap();
        QString categoryTitle = qtrc("project", map["title"].toString().toUtf8().data());
        QStringList files = map["files"].toStringList();

        io::paths_t paths;
        for (const QString& path : files) {
            paths.push_back(path);
        }

        templates << readTemplates(paths, categoryTitle, dirPath);
    }

    return templates;
}

Templates TemplatesRepository::readTemplates(const io::paths_t& files, const QString& category, const io::path_t& dirPath) const
{
    TRACEFUNC;

    Templates templates;

    for (const io::path_t& file : files) {
        io::path_t path = dirPath.empty() ? file : dirPath + "/" + file;
        RetVal<ProjectMeta> meta = mscReader()->readMeta(path);
        if (!meta.ret) {
            LOGE() << QString("failed read template %1: %2")
                .arg(path.toQString())
                .arg(QString::fromStdString(meta.ret.toString()));
            continue;
        }

        Template templ;
        templ.categoryTitle = category;
        templ.meta = std::move(meta.val);

        templates << templ;
    }

    return templates;
}
