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

#include <gtest/gtest.h>

#include "project/internal/templatesrepository.h"

#include "notation/tests/mocks/msczreadermock.h"
#include "mocks/projectconfigurationmock.h"
#include "global/tests/mocks/filesystemmock.h"

#include <QJsonDocument>
#include <QJsonArray>

using ::testing::_;
using ::testing::Return;

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;

class Project_TemplatesRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_repository = std::make_shared<TemplatesRepository>();
        m_msczReader = std::make_shared<MsczReaderMock>();
        m_fileSystem = std::make_shared<FileSystemMock>();
        m_configuration = std::make_shared<ProjectConfigurationMock>();

        m_repository->configuration.set(m_configuration);
        m_repository->mscReader.set(m_msczReader);
        m_repository->fileSystem.set(m_fileSystem);
    }

    QVariantMap buildCategory(const QString& title, const QStringList& files) const
    {
        QVariantMap obj;
        obj["title"] = title;
        obj["files"] = files;

        return obj;
    }

    Template buildTemplate(const QString& categoryTitle, const muse::io::path_t& path, bool isCustom) const
    {
        Template templ;
        templ.categoryTitle = categoryTitle;
        templ.meta.title = path.toQString();
        templ.meta.filePath = path;
        templ.meta.creationDate = QDate::currentDate();
        templ.isCustom = isCustom;

        return templ;
    }

    QByteArray categoriesToJson(const QVariantList& categories) const
    {
        QJsonDocument document;
        document.setArray(QJsonArray::fromVariantList(categories));

        return document.toJson();
    }

    std::shared_ptr<TemplatesRepository> m_repository;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<MsczReaderMock> m_msczReader;
    std::shared_ptr<FileSystemMock> m_fileSystem;
};

namespace mu::project {
inline bool operator==(const Template& templ1, const Template& templ2)
{
    bool equals = true;

    equals &= (templ1.meta.title == templ1.meta.title);
    equals &= (templ1.categoryTitle == templ2.categoryTitle);
    equals &= (templ1.meta.creationDate == templ2.meta.creationDate);
    equals &= (templ1.isCustom == templ2.isCustom);

    return equals;
}
}

TEST_F(Project_TemplatesRepositoryTest, Templates)
{
    // [GIVEN] All template dirs
    io::paths_t templateDirs {
        "/path/to/standard/templates",
        "/path/to/user/templates",
        "/path/to/user/templates/without/categories_json"
    };

    muse::io::path_t otherUserTemplatesDir = templateDirs[2];

    ON_CALL(*m_configuration, availableTemplateDirs())
    .WillByDefault(Return(templateDirs));

    // [GIVEN] All files with categories
    io::paths_t categoriesJsonPaths {
        "/path/to/standard/templates/categories.json",
        "/path/to/user/templates/categories.json"
    };

    for (size_t i = 0; i < templateDirs.size(); ++i) {
        if (i < categoriesJsonPaths.size()) {
            ON_CALL(*m_configuration, templateCategoriesJsonPath(templateDirs[i]))
            .WillByDefault(Return(categoriesJsonPaths[i]));

            ON_CALL(*m_fileSystem, exists(categoriesJsonPaths[i]))
            .WillByDefault(Return(Ret(true)));
        }
    }

    // [GIVEN] One dir has no categories.json file
    ON_CALL(*m_fileSystem, exists(otherUserTemplatesDir))
    .WillByDefault(Return(Ret(false)));

    io::paths_t otherUserTemplates {
        "/path/to/user/templates/without/categories_json/AAA.mscz",
        "/path/to/user/templates/without/categories_json/BBB.mscx"
    };

    std::vector<std::string> filters = { "*.mscz", "*.mscx" };
    ON_CALL(*m_fileSystem, scanFiles(otherUserTemplatesDir, filters, ScanMode::FilesInCurrentDirAndSubdirs))
    .WillByDefault(Return(RetVal<io::paths_t>::make_ok(otherUserTemplates)));

    // [GIVEN] All templates
    QVariantList standardTemplates {
        buildCategory("Jazz", { "Big_Band.mscx", "Jazz_Combo.mscz" }),
        buildCategory("Solo", { "Guitar.mscx" })
    };

    ByteArray standardTemplatesJson = ByteArray::fromQByteArray(categoriesToJson(standardTemplates));
    ON_CALL(*m_fileSystem, readFile(categoriesJsonPaths[0]))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(standardTemplatesJson)));

    QVariantList userTemplates {
        buildCategory("Popular", { "Rock_Band.mscz" })
    };

    ByteArray userTemplatesJson = ByteArray::fromQByteArray(categoriesToJson(userTemplates));
    ON_CALL(*m_fileSystem, readFile(categoriesJsonPaths[1]))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(userTemplatesJson)));

    // [GIVEN] Expected templates after reading
    Templates expectedTemplates {
        buildTemplate("Jazz", "/path/to/standard/templates/Big_Band.mscx", false /*isCustom*/),
        buildTemplate("Jazz", "/path/to/standard/templates/Jazz_Combo.mscz", false),
        buildTemplate("Solo", "/path/to/standard/templates/Guitar.mscx", false),
        buildTemplate("Popular", "/path/to/user/templates/Rock_Band.mscz", false)
    };

    for (const muse::io::path_t& otherTemplatePath : otherUserTemplates) {
        expectedTemplates << buildTemplate("My templates", otherTemplatePath, true /*isCustom*/);
    }

    for (const Template& templ : expectedTemplates) {
        ON_CALL(*m_msczReader, readMeta(templ.meta.filePath))
        .WillByDefault(Return(RetVal<ProjectMeta>::make_ok(templ.meta)));
    }

    // [WHEN] Get templates meta
    RetVal<Templates> templates = m_repository->templates();

    // [THEN] Successfully got templates meta
    EXPECT_TRUE(templates.ret);

    EXPECT_EQ(templates.val.size(), expectedTemplates.size());
    for (const Template& templ : templates.val) {
        EXPECT_TRUE(expectedTemplates.contains(templ));
    }
}
