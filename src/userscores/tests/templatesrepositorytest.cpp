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

#include "notation/tests/mocks/msczreadermock.h"
#include "mocks/userscoresconfigurationmock.h"
#include "system/tests/mocks/filesystemmock.h"

using ::testing::_;
using ::testing::Return;

using namespace mu;
using namespace mu::notation;
using namespace mu::userscores;
using namespace mu::system;

class TemplatesRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_repository = std::make_shared<TemplatesRepository>();
        m_msczReader = std::make_shared<MsczReaderMock>();
        m_fileSystem = std::make_shared<FileSystemMock>();
        m_configuration = std::make_shared<UserScoresConfigurationMock>();

        m_repository->setconfiguration(m_configuration);
        m_repository->setmsczReader(m_msczReader);
        m_repository->setfileSystem(m_fileSystem);
    }

    Meta createMeta(const io::path& path) const
    {
        Meta meta;

        meta.title = path.toQString();
        meta.filePath = path;
        meta.creationDate = QDate::currentDate();

        return meta;
    }

    std::shared_ptr<TemplatesRepository> m_repository;
    std::shared_ptr<UserScoresConfigurationMock> m_configuration;
    std::shared_ptr<MsczReaderMock> m_msczReader;
    std::shared_ptr<FileSystemMock> m_fileSystem;
};

namespace mu::userscores {
bool operator==(const Template& templ1, const Template& templ2)
{
    bool equals = true;

    equals &= (templ1.title == templ2.title);
    equals &= (templ1.categoryTitle == templ2.categoryTitle);
    equals &= (templ1.creationDate == templ2.creationDate);

    return equals;
}
}

TEST_F(TemplatesRepositoryTest, Templates)
{
    // [GIVEN] All paths to templates dirs
    io::paths templatesDirPaths {
        "/path/to/templates",
        "/path/to/user/templates"
        "/extensions/templates"
    };

    EXPECT_CALL(*m_configuration, availableTemplatesPaths())
    .WillOnce(Return(templatesDirPaths));

    // [GIVEN] All paths to mscz files
    io::paths allPathsToMsczFiles;

    for (size_t i = 0; i < templatesDirPaths.size(); ++i) {
        io::path dirPath = templatesDirPaths[i];
        io::path filePath = dirPath + QString("/file%1.mscz").arg(i);
        allPathsToMsczFiles.push_back(filePath);

        QStringList filters = { "*.mscz", "*.mscx" };

        RetVal<io::paths> result = RetVal<io::paths>::make_ok(io::paths { filePath });
        ON_CALL(*m_fileSystem, scanFiles(dirPath, filters, IFileSystem::ScanMode::IncludeSubdirs))
        .WillByDefault(Return(result));
    }

    // [GIVEN] Templates meta
    Templates expectedTemplates;

    for (const io::path& path: allPathsToMsczFiles) {
        Meta meta = createMeta(path.toQString());

        ON_CALL(*m_msczReader, readMetaList(io::paths { path }))
        .WillByDefault(Return(MetaList { meta }));

        Template templ(meta);
        templ.categoryTitle = io::dirname(path).toQString();
        expectedTemplates << templ;
    }

    // [WHEN] Get templates meta
    RetVal<Templates> templates = m_repository->templates();

    // [THEN] Successfully got templates meta
    EXPECT_TRUE(templates.ret);

    EXPECT_EQ(templates.val.size(), expectedTemplates.size());
    for (const Template& templ: templates.val) {
        EXPECT_TRUE(expectedTemplates.contains(templ));
    }
}
