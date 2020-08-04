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

#include "domain/notation/internal/templatesrepository.h"

#include "mocks/msczreadermock.h"
#include "mocks/notationconfigurationmock.h"
#include "system/tests/mocks/fsoperationsmock.h"

using ::testing::_;
using ::testing::Return;

using namespace mu;
using namespace mu::domain::notation;
using namespace mu::framework;

class TemplatesRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_repository = std::make_shared<TemplatesRepository>();
        m_msczReader = std::make_shared<MsczReaderMock>();
        m_fsOperations = std::make_shared<FsOperationsMock>();
        m_configuration = std::make_shared<NotationConfigurationMock>();

        m_repository->setconfiguration(m_configuration);
        m_repository->setmsczReader(m_msczReader);
        m_repository->setfsOperations(m_fsOperations);
    }

    Meta createMeta(const QString& title) const
    {
        Meta meta;

        meta.title = title;
        meta.creationDate = QDate::currentDate();

        return meta;
    }

    std::shared_ptr<TemplatesRepository> m_repository;
    std::shared_ptr<NotationConfigurationMock> m_configuration;
    std::shared_ptr<MsczReaderMock> m_msczReader;
    std::shared_ptr<FsOperationsMock> m_fsOperations;
};

namespace mu {
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

TEST_F(TemplatesRepositoryTest, TemplatesMeta)
{
    // [GIVEN] All paths to templates dirs
    QString templatesDirPath = "/path/to/templates";
    EXPECT_CALL(*m_configuration, templatesPath())
            .WillOnce(Return(templatesDirPath));

    QString userTemplatesDirPath = "/user/path/to/templates";
    EXPECT_CALL(*m_configuration, userTemplatesPath())
            .WillOnce(Return(userTemplatesDirPath));

    QStringList extensionsTemplatesDirPaths {
        "/extension1/templates",
        "/extension2/templates"
    };

    EXPECT_CALL(*m_configuration, extensionsTemplatesPaths())
            .WillOnce(Return(extensionsTemplatesDirPaths));

    // [GIVEN] All paths to mscz files
    QStringList allTemplatesDirPaths;
    allTemplatesDirPaths << templatesDirPath << userTemplatesDirPath << extensionsTemplatesDirPaths;

    QStringList allPathsToMsczFiles;

    for (int i = 0; i < allTemplatesDirPaths.size(); ++i) {
        QString dirPath = allTemplatesDirPaths[i];
        QString filePath = dirPath + QString("/file%1.mscz").arg(i);
        allPathsToMsczFiles << filePath;

        QStringList filters = { "*.mscz", "*.mscx" };

        ON_CALL(*m_fsOperations, scanForFiles(dirPath, filters, IFsOperations::ScanMode::IncludeSubdirs))
                .WillByDefault(Return(QStringList { filePath }));
    }

    // [GIVEN] Templates meta
    MetaList expectedMetaList;

    for (const QString& path: allPathsToMsczFiles) {
        Meta meta = createMeta(path);
        expectedMetaList << meta;

        ON_CALL(*m_msczReader, readMeta(io::pathFromQString(path)))
                .WillByDefault(Return(RetVal<Meta>::make_ok(meta)));
    }

    // [WHEN] Get templates meta
    RetVal<MetaList> metaList = m_repository->templatesMeta();

    // [THEN] Successfully got templates meta
    EXPECT_TRUE(metaList.ret);

    EXPECT_EQ(metaList.val.size(), expectedMetaList.size());
    for (const Meta& meta: metaList.val) {
        EXPECT_TRUE(expectedMetaList.contains(meta));
    }
}
