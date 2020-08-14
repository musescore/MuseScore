//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <gtest/gtest.h>

#include "userscores/internal/templatesrepository.h"

#include "domain/notation/tests/mocks/msczreadermock.h"
#include "mocks/userscoresconfigurationmock.h"
#include "system/tests/mocks/fsoperationsmock.h"

using ::testing::_;
using ::testing::Return;

using namespace mu;
using namespace mu::domain::notation;
using namespace mu::userscores;
using namespace mu::framework;

class TemplatesRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_repository = std::make_shared<TemplatesRepository>();
        m_msczReader = std::make_shared<MsczReaderMock>();
        m_fsOperations = std::make_shared<FsOperationsMock>();
        m_configuration = std::make_shared<UserScoresConfigurationMock>();

        m_repository->setconfiguration(m_configuration);
        m_repository->setmsczReader(m_msczReader);
        m_repository->setfsOperations(m_fsOperations);
    }

    TemplateCategory createCategory(const QString& title, const QString& code) const
    {
        TemplateCategory category;

        category.codeKey = code;
        category.title = title;

        return category;
    }

    Meta createMeta(const QString& title) const
    {
        Meta meta;

        meta.title = title;
        meta.creationDate = QDate::currentDate();

        return meta;
    }

    std::shared_ptr<TemplatesRepository> m_repository;
    std::shared_ptr<UserScoresConfigurationMock> m_configuration;
    std::shared_ptr<MsczReaderMock> m_msczReader;
    std::shared_ptr<FsOperationsMock> m_fsOperations;
};

namespace mu {
namespace userscores {
bool operator==(const TemplateCategory& category1, const TemplateCategory& category2)
{
    bool equals = true;

    equals &= (category1.codeKey == category2.codeKey);
    equals &= (category1.title == category2.title);

    return equals;
}
}

namespace domain {
namespace notation {
bool operator==(const Meta& meta1, const Meta& meta2)
{
    bool equals = true;

    equals &= (meta1.title == meta2.title);
    equals &= (meta1.creationDate == meta2.creationDate);

    return equals;
}
}
}
}

TEST_F(TemplatesRepositoryTest, Categories)
{
    // [GIVEN] All paths to mscz files dirs
    QStringList templatesDirPaths {
        "/path/to/templates/AAAA",
        "/path/to/templates/01-some_category_name",
        "/path/to/templates/99#another_category_name",
        "/path/to/empty/dir"
    };

    ON_CALL(*m_configuration, templatesDirPaths())
    .WillByDefault(Return(templatesDirPaths));

    ON_CALL(*m_fsOperations, dirName(templatesDirPaths[0]))
    .WillByDefault(Return("AAAA"));

    ON_CALL(*m_fsOperations, dirName(templatesDirPaths[1]))
    .WillByDefault(Return("01-some_category_name"));

    ON_CALL(*m_fsOperations, dirName(templatesDirPaths[2]))
    .WillByDefault(Return("99#another_category_name"));

    // [GIVEN] Some dirs have MSCZ files
    QStringList filters = { "*.mscz", "*.mscx" };
    for (int i = 0; i < 3; ++i) {
        ON_CALL(*m_fsOperations, scanFiles(templatesDirPaths[i], filters, IFsOperations::ScanMode::IncludeSubdirs))
        .WillByDefault(Return(RetVal<QStringList>::make_ok(QStringList { "/some/path/to/file.mscz" })));
    }

    // [WHEN] Get templates categories
    RetVal<TemplateCategoryList> categories = m_repository->categories();

    // [THEN] Successfully got categories, empty dir was skipped
    EXPECT_TRUE(categories.ret);

    TemplateCategoryList expectedCategories;
    expectedCategories << createCategory("AAAA", "/path/to/templates/AAAA")
                       << createCategory("some category name", "/path/to/templates/01-some_category_name")
                       << createCategory("another category name", "/path/to/templates/99#another_category_name");

    EXPECT_EQ(categories.val.size(), expectedCategories.size());
    for (const TemplateCategory& category: categories.val) {
        EXPECT_TRUE(expectedCategories.contains(category));
    }
}

TEST_F(TemplatesRepositoryTest, TemplatesMeta)
{
    // [GIVEN] Category codeKey
    QString codeKey = "/path/to/templates";

    // [GIVEN] Category templates
    QStringList pathsToMsczFiles;

    for (int i = 0; i < 5; ++i) {
        QString filePath = codeKey + QString("/file%1.mscz").arg(i);
        pathsToMsczFiles << filePath;
    }

    QStringList filters = { "*.mscz", "*.mscx" };
    ON_CALL(*m_fsOperations, scanFiles(codeKey, filters, IFsOperations::ScanMode::IncludeSubdirs))
    .WillByDefault(Return(RetVal<QStringList>::make_ok(pathsToMsczFiles)));

    // [GIVEN] Templates meta
    MetaList expectedMetaList;

    for (const QString& path: pathsToMsczFiles) {
        Meta meta = createMeta(path);
        expectedMetaList << meta;

        ON_CALL(*m_msczReader, readMeta(io::pathFromQString(path)))
        .WillByDefault(Return(RetVal<Meta>::make_ok(meta)));
    }

    // [WHEN] Get templates meta
    RetVal<MetaList> metaList = m_repository->templatesMeta(codeKey);

    // [THEN] Successfully got templates meta
    EXPECT_TRUE(metaList.ret);

    EXPECT_EQ(metaList.val.size(), expectedMetaList.size());
    for (const Meta& meta: metaList.val) {
        EXPECT_TRUE(expectedMetaList.contains(meta));
    }
}
