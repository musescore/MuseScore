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
#include <gmock/gmock.h>

#include "async/processevents.h"

#include "global/tests/mocks/filesystemmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationstylemock.h"
#include "project/tests/mocks/projectsessionmock.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/dummyparent.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/rootitem.h"

#include "notation/internal/notationactioncontroller.h"

using namespace ::testing;
using namespace mu::notation;
using namespace muse;

namespace {
const io::path_t STYLES_DIR("styles");

//! NOTE: keep the body queued, don't run it eagerly
//! otherwise resolve() can fire before onResolve is attached
async::Promise<io::path_t> chosenFile(const io::path_t& path)
{
    return async::make_promise<io::path_t>([path](auto resolve, auto) {
        return resolve(path);
    });
}

async::Promise<io::path_t> cancelled()
{
    return async::make_promise<io::path_t>([](auto, auto reject) {
        return reject(int(Ret::Code::Cancel), "");
    });
}

async::Promise<IInteractive::Result> pendingResult()
{
    return async::make_promise<IInteractive::Result>([](auto, auto) {
        return async::Promise<IInteractive::Result>::dummy_result();
    }, async::PromiseType::AsyncByBody);
}
}

namespace mu::notation {
class NotationScene_NotationActionControllerTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_realFileSystem = modularity::globalIoc()->resolve<io::IFileSystem>("utests");
        modularity::globalIoc()->unregister<io::IFileSystem>("utests");
        modularity::globalIoc()->registerExport<io::IFileSystem>("utests", m_fileSystem);

        m_controller = std::make_shared<NotationActionController>(muse::modularity::globalCtx());

        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        m_projectSession = std::make_shared<NiceMock<project::ProjectSessionMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_interaction = std::make_shared<NiceMock<NotationInteractionMock> >();
        m_style = std::make_shared<NiceMock<NotationStyleMock> >();

        m_controller->interactive.set(m_interactive);
        m_controller->configuration.set(m_configuration);
        m_controller->projectSession.set(m_projectSession);

        ON_CALL(*m_projectSession, currentNotation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_notation, interaction()).WillByDefault(Return(m_interaction));
        ON_CALL(*m_notation, style()).WillByDefault(Return(m_style));
        ON_CALL(*m_configuration, userStylesPath()).WillByDefault(Return(STYLES_DIR));

        ON_CALL(*m_fileSystem, exists(_)).WillByDefault(Return(make_ret(Ret::Code::UnknownError)));
    }

    void TearDown() override
    {
        //! NOTE The addImage flow holds on to the interaction past the test, so verify it here and let it leak
        Mock::VerifyAndClearExpectations(m_interaction.get());
        Mock::AllowLeak(m_interaction.get());

        modularity::globalIoc()->unregister<io::IFileSystem>("utests");
        if (m_realFileSystem) {
            modularity::globalIoc()->registerExport<io::IFileSystem>("utests", m_realFileSystem);
        }
    }

    void addImage() { m_controller->addImage(); }
    void loadStyle() { m_controller->loadStyle(); }
    void saveStyle() { m_controller->saveStyle(); }

    std::shared_ptr<NotationActionController> m_controller;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<NotationConfigurationMock> m_configuration;
    std::shared_ptr<project::ProjectSessionMock> m_projectSession;
    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<NotationInteractionMock> m_interaction;
    std::shared_ptr<NotationStyleMock> m_style;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<io::IFileSystem> m_realFileSystem;
};
}

// ==================================================
// addImage()
// ==================================================

TEST_F(NotationScene_NotationActionControllerTests, AddImage_UserChoosesAFile_AddsItToTheItem)
{
    //! [GIVEN] An item is selected, and the user chooses an image
    std::unique_ptr<mu::engraving::MasterScore> score(mu::engraving::compat::ScoreAccess::createMasterScore(modularity::globalCtx()));
    EngravingItem* item = score->dummy()->rootItem();

    ON_CALL(*m_interaction, contextItem()).WillByDefault(Return(item));
    ON_CALL(*m_interaction, canAddImageToItem(item)).WillByDefault(Return(make_ok()));

    EXPECT_CALL(*m_interactive, selectOpeningFile(_, _, _)).WillOnce(Return(chosenFile("image.png")));

    //! [THEN] The image is added to the item
    EXPECT_CALL(*m_interaction, addImageToItem(io::path_t("image.png"), item));

    //! [WHEN] Adding an image
    addImage();
    async::processMessages();
}

TEST_F(NotationScene_NotationActionControllerTests, AddImage_UserCancels_AddsNothing)
{
    //! [GIVEN] An item is selected, and the user cancels
    std::unique_ptr<mu::engraving::MasterScore> score(mu::engraving::compat::ScoreAccess::createMasterScore(modularity::globalCtx()));
    EngravingItem* item = score->dummy()->rootItem();

    ON_CALL(*m_interaction, contextItem()).WillByDefault(Return(item));
    ON_CALL(*m_interaction, canAddImageToItem(item)).WillByDefault(Return(make_ok()));

    EXPECT_CALL(*m_interactive, selectOpeningFile(_, _, _)).WillOnce(Return(cancelled()));

    //! [THEN] Nothing is added
    EXPECT_CALL(*m_interaction, addImageToItem(_, _)).Times(0);

    //! [WHEN] Adding an image
    addImage();
    async::processMessages();
}

TEST_F(NotationScene_NotationActionControllerTests, AddImage_ImageCannotBeAdded_AsksNothing)
{
    //! [GIVEN] An image cannot be added to the selected item
    ON_CALL(*m_interaction, canAddImageToItem(_)).WillByDefault(Return(make_ret(Ret::Code::UnknownError)));

    //! [THEN] No file is asked
    EXPECT_CALL(*m_interactive, selectOpeningFile(_, _, _)).Times(0);

    //! [WHEN] Adding an image
    addImage();
    async::processMessages();
}

// ==================================================
// loadStyle()
// ==================================================

TEST_F(NotationScene_NotationActionControllerTests, LoadStyle_UserChoosesAStyleFile_LoadsIt)
{
    //! [GIVEN] The user chooses a style file from the styles folder
    const io::path_t path("style.mss");
    ON_CALL(*m_fileSystem, exists(path)).WillByDefault(Return(make_ok()));
    ON_CALL(*m_fileSystem, readFile(path, _)).WillByDefault([](const io::path_t&, ByteArray& data) {
        data = ByteArray("<museScore><Style/></museScore>");
        return make_ok();
    });

    EXPECT_CALL(*m_interactive, selectOpeningFile(_, STYLES_DIR, _)).WillOnce(Return(chosenFile(path)));

    //! [THEN] The style is loaded
    EXPECT_CALL(*m_style, loadStyle(path, false)).WillOnce(Return(true));

    //! [WHEN] Loading a style
    loadStyle();
    async::processMessages();
}

TEST_F(NotationScene_NotationActionControllerTests, LoadStyle_UserChoosesNotAStyleFile_ShowsAnError)
{
    //! [GIVEN] The user chooses a file that is not a style
    EXPECT_CALL(*m_interactive, selectOpeningFile(_, _, _)).WillOnce(Return(chosenFile("missing.mss")));

    //! [THEN] An error is shown, and nothing is loaded
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).WillOnce(Return(pendingResult()));
    EXPECT_CALL(*m_style, loadStyle(_, _)).Times(0);

    //! [WHEN] Loading a style
    loadStyle();
    async::processMessages();
}

TEST_F(NotationScene_NotationActionControllerTests, LoadStyle_UserCancels_LoadsNothing)
{
    //! [GIVEN] The user cancels
    EXPECT_CALL(*m_interactive, selectOpeningFile(_, _, _)).WillOnce(Return(cancelled()));

    //! [THEN] Nothing is loaded
    EXPECT_CALL(*m_style, loadStyle(_, _)).Times(0);

    //! [WHEN] Loading a style
    loadStyle();
    async::processMessages();
}

// ==================================================
// saveStyle()
// ==================================================

TEST_F(NotationScene_NotationActionControllerTests, SaveStyle_UserChoosesAFile_SavesTheStyle)
{
    //! [GIVEN] The user chooses a file in the styles folder
    EXPECT_CALL(*m_interactive, selectSavingFile(_, STYLES_DIR, _, _)).WillOnce(Return(chosenFile("style.mss")));

    //! [THEN] The style is saved there
    EXPECT_CALL(*m_style, saveStyle(io::path_t("style.mss"))).WillOnce(Return(true));
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    //! [WHEN] Saving the style
    saveStyle();
    async::processMessages();
}

TEST_F(NotationScene_NotationActionControllerTests, SaveStyle_UserCancels_SavesNothing)
{
    //! [GIVEN] The user cancels
    EXPECT_CALL(*m_interactive, selectSavingFile(_, _, _, _)).WillOnce(Return(cancelled()));

    //! [THEN] Nothing is saved
    EXPECT_CALL(*m_style, saveStyle(_)).Times(0);

    //! [WHEN] Saving the style
    saveStyle();
    async::processMessages();
}
