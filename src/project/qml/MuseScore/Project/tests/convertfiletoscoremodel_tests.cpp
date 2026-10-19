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

#include <QUrl>

#include "async/processevents.h"
#include "global/dataformatter.h"
#include "global/io/path.h"
#include "global/translation.h"

#include "project/qml/MuseScore/Project/internal/ConvertFileToScore/convertfiletoscoremodel.h"

#include "project/tests/utils/testutils.h"
#include "project/tests/mocks/convertfiletoscorescenariomock.h"
#include "project/tests/mocks/projectconfigurationmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "cloud/tests/mocks/audiocomservicemock.h"
#include "cloud/tests/mocks/authorizationservicemock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
async::Promise<IInteractive::Result> resolvedResultPromise(const IInteractive::Result& result = IInteractive::Result())
{
    return resolvedPromise(result);
}

Matcher<const IInteractive::Text&> TextIs(const std::string& text)
{
    return Field(&IInteractive::Text::text, text);
}

Matcher<const IInteractive::ButtonDatas&> ButtonIdsAre(const std::vector<int>& expectedIds)
{
    return Truly([expectedIds](const IInteractive::ButtonDatas& buttons) {
        if (buttons.size() != expectedIds.size()) {
            return false;
        }
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].btn != expectedIds[i]) {
                return false;
            }
        }
        return true;
    });
}

std::vector<std::string> filePickerFilters(const std::string& patterns)
{
    return {
        muse::trc("project", "All supported files") + " (" + patterns + ")",
        muse::trc("project", "All") + " (*)"
    };
}

QString sizeText(qint64 sizeBytes)
{
    return DataFormatter::formatFileSize(size_t(sizeBytes)).toQString();
}

QStringList sectionTitles(const QVariantList& sections)
{
    QStringList titles;
    for (const QVariant& section : sections) {
        titles << section.toMap().value("title").toString();
    }

    return titles;
}

QStringList sectionItems(const QVariantList& sections, const QString& title)
{
    for (const QVariant& section : sections) {
        const QVariantMap map = section.toMap();
        if (map.value("title").toString() == title) {
            return map.value("items").toStringList();
        }
    }

    return {};
}
}

namespace mu::project {
class Project_ConvertFileToScoreModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_scenario = std::make_shared<NiceMock<ConvertFileToScoreScenarioMock> >();
        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_audioComService = std::make_shared<NiceMock<AudioComServiceMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();

        m_model.interactive.set(m_interactive);
        m_model.convertFileToScoreScenario.set(m_scenario);
        m_model.museScoreComService.set(m_museScoreComService);
        m_model.audioComService.set(m_audioComService);
        m_model.configuration.set(m_configuration);
        m_model.fileListModel()->fileSystem.set(m_fileSystem);

        ON_CALL(*m_scenario, config())
        .WillByDefault(ReturnRef(m_config));
    }

    void setAudioComUrl(const QString& url)
    {
        CloudInfo info;
        info.url = QUrl(url);
        ON_CALL(*m_audioComService, cloudInfo())
        .WillByDefault(Return(info));
    }

    void expectValidateFiles(const io::paths_t& paths, ConvertType type, FileCategory category)
    {
        EXPECT_CALL(*m_scenario, validateFiles(paths))
        .WillOnce(Return(RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { type, category })));
    }

    void expectConfirmationQuestion(const std::string& title, IInteractive::Button clickedButton)
    {
        const std::string text = muse::trc("project/convert", "Your current selection will be lost.");

        EXPECT_CALL(*m_interactive,
                    question(title, TextIs(text),
                             ButtonIdsAre({ int(IInteractive::Button::No), int(IInteractive::Button::Yes) }),
                             int(IInteractive::Button::No), IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
        .WillOnce(Invoke([clickedButton](auto&&...) {
            return resolvedResultPromise(IInteractive::Result(int(clickedButton)));
        }));
    }

    const QString AUDIO_COM_URL = "test://audio";
    const QString BOLD_YOUTUBE = "<b>YouTube</b>";
    const QString BOLD_AUDIO_COM = "<b><a href=\"test://audio\">Audio.com</a></b>";

    ConvertFileToScoreModel m_model;

    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<ConvertFileToScoreScenarioMock> m_scenario;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<AudioComServiceMock> m_audioComService;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;

    ConvertConfig m_config;
};
}

// ==================================================
// accountAvatarUrl() / guidelinesUrl()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, AccountAvatarUrl_ComesFromMuseScoreComAccount)
{
    // [GIVEN] The MuseScore.com account has an avatar
    AccountInfo accountInfo;
    accountInfo.avatarUrl = QUrl("test://avatar.png");

    std::shared_ptr<AuthorizationServiceMock> authorization = std::make_shared<NiceMock<AuthorizationServiceMock> >();
    ON_CALL(*authorization, accountInfo())
    .WillByDefault(ReturnRef(accountInfo));
    ON_CALL(*m_museScoreComService, authorization())
    .WillByDefault(Return(authorization));

    // [THEN] The model exposes it
    EXPECT_EQ(m_model.accountAvatarUrl(), "test://avatar.png");
}

TEST_F(Project_ConvertFileToScoreModelTest, GuidelinesUrl_ComesFromConfiguration)
{
    // [GIVEN] The configured guidelines url
    ON_CALL(*m_configuration, scoreUploadingGuidelinesUrl())
    .WillByDefault(Return(QUrl("test://guidelines")));

    // [THEN] The model exposes it
    EXPECT_EQ(m_model.guidelinesUrl(), "test://guidelines");
}

// ==================================================
// load()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, Load_SetsPathsAndConvertType)
{
    // [THEN] Count the convertTypeChanged and defaultSaveAsNameChanged notifications
    int convertTypeChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::convertTypeChanged, [&convertTypeChangedCount]() {
        ++convertTypeChangedCount;
    });
    int defaultSaveAsNameChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::defaultSaveAsNameChanged, [&defaultSaveAsNameChangedCount]() {
        ++defaultSaveAsNameChangedCount;
    });

    // [WHEN] Loading an audio file
    const QStringList paths { "/some/song.mp3" };
    m_model.load(paths, int(ConvertType::Audio2Score));

    // [THEN] The file and the convert type are set
    EXPECT_EQ(m_model.fileListModel()->paths(), paths);
    EXPECT_EQ(m_model.fileListModel()->fileCategory(), FileCategory::Audio);
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Audio2Score));
    EXPECT_EQ(convertTypeChangedCount, 1);
    EXPECT_EQ(defaultSaveAsNameChangedCount, 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, Load_PassesConfigToFileListModel)
{
    // [GIVEN] A limit on the number of PDF files
    m_config.omr.pdf.maxFiles = 1;

    // [WHEN] Loading a PDF file
    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));

    // [THEN] The file list uses that limit
    EXPECT_EQ(m_model.fileListModel()->convertLimits().value("maxFileCount").toInt(), 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, Load_ClearsSelectedLink)
{
    // [GIVEN] A selected link
    m_model.setSelectedLink("test://link");

    // [WHEN] Loading a file
    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));

    // [THEN] The link is cleared
    EXPECT_TRUE(m_model.selectedLink().isEmpty());
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Omr));
}

// ==================================================
// setSelectedLink()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, SetSelectedLink_SwitchesToAudio2ScoreAndClearsFiles)
{
    // [GIVEN] A selected PDF file
    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));

    // [THEN] Count the selectedLinkChanged and convertTypeChanged notifications
    int selectedLinkChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::selectedLinkChanged, [&selectedLinkChangedCount]() {
        ++selectedLinkChangedCount;
    });
    int convertTypeChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::convertTypeChanged, [&convertTypeChangedCount]() {
        ++convertTypeChangedCount;
    });

    // [WHEN] Selecting a link
    m_model.setSelectedLink("test://link");

    // [THEN] The link replaces the file, and the convert type is Audio2Score
    EXPECT_EQ(m_model.selectedLink(), "test://link");
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Audio2Score));
    EXPECT_EQ(m_model.fileListModel()->rowCount(), 0);
    EXPECT_EQ(selectedLinkChangedCount, 1);
    EXPECT_EQ(convertTypeChangedCount, 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, SetSelectedLink_SameLink_DoesNothing)
{
    // [GIVEN] A selected link
    m_model.setSelectedLink("test://link");

    // [THEN] Count the selectedLinkChanged notifications
    int selectedLinkChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::selectedLinkChanged, [&selectedLinkChangedCount]() {
        ++selectedLinkChangedCount;
    });

    // [WHEN] Selecting the same link again
    m_model.setSelectedLink("test://link");

    // [THEN] Nothing is notified
    EXPECT_EQ(selectedLinkChangedCount, 0);
}

TEST_F(Project_ConvertFileToScoreModelTest, SetSelectedLink_Empty_KeepsConvertType)
{
    // [GIVEN] A selected link
    m_model.setSelectedLink("test://link");

    // [THEN] Count the selectedLinkChanged and convertTypeChanged notifications
    int selectedLinkChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::selectedLinkChanged, [&selectedLinkChangedCount]() {
        ++selectedLinkChangedCount;
    });
    int convertTypeChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::convertTypeChanged, [&convertTypeChangedCount]() {
        ++convertTypeChangedCount;
    });

    // [WHEN] Clearing the link
    m_model.setSelectedLink(QString());

    // [THEN] The link is cleared, but the convert type is kept
    EXPECT_TRUE(m_model.selectedLink().isEmpty());
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Audio2Score));
    EXPECT_EQ(selectedLinkChangedCount, 1);
    EXPECT_EQ(convertTypeChangedCount, 0);
}

// ==================================================
// defaultSaveAsName()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, DefaultSaveAsName_NoFiles_IsUntitled)
{
    EXPECT_EQ(m_model.defaultSaveAsName(), muse::qtrc("project", "Untitled score"));
}

TEST_F(Project_ConvertFileToScoreModelTest, DefaultSaveAsName_IsBaseNameOfFirstFile)
{
    // [WHEN] Loading several files
    m_model.load({ "/some/My.Song.v2.png", "/some/other.png" }, int(ConvertType::Omr));

    // [THEN] The name comes from the first one, with only the last suffix removed
    EXPECT_EQ(m_model.defaultSaveAsName(), "My.Song.v2");
}

// ==================================================
// fileRequirements()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, FileRequirements_NoFiles_ShowsAllSections)
{
    // [GIVEN] Limits for every file category
    m_config.omr.pdf.maxFileSizeBytes = 20LL * 1024 * 1024;
    m_config.omr.pdf.maxPages = 30;
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.images.allowedExtensions = { "png", "jpg", "jpeg" };
    m_config.omr.images.maxFileSizeBytes = 40LL * 1024 * 1024;
    m_config.omr.images.maxFiles = 15;
    m_config.audio2score.file.allowedExtensions = { "mp3", "wav" };
    m_config.audio2score.file.maxFileSizeBytes = 50LL * 1024 * 1024;
    m_config.audio2score.file.maxFiles = 1;

    const QString pdfTitle = muse::qtrc("project/convert", "PDF");
    const QString imagesTitle = muse::qtrc("project/convert", "Images");
    const QString audioTitle = muse::qtrc("project/convert", "Audio");

    // [WHEN] Reading the requirements before any file is selected
    const QVariantList requirements = m_model.fileRequirements();

    // [THEN] Every section is shown
    EXPECT_EQ(sectionTitles(requirements), QStringList({ pdfTitle, imagesTitle, audioTitle }));

    EXPECT_EQ(sectionItems(requirements, pdfTitle), QStringList({
        muse::qtrc("project/convert", "%1 max").arg(sizeText(m_config.omr.pdf.maxFileSizeBytes)),
        muse::qtrc("project/convert", "%n page(s) max.", nullptr, 30),
        muse::qtrc("project/convert", "%n file(s) per conversion", nullptr, 1)
    }));

    // [THEN] JPG and JPEG are shown as a single format
    EXPECT_EQ(sectionItems(requirements, imagesTitle), QStringList({
        "PNG, JPG",
        muse::qtrc("project/convert", "%1 max combined").arg(sizeText(m_config.omr.images.maxFileSizeBytes)),
        muse::qtrc("project/convert", "Max. %n image(s)", nullptr, 15)
    }));

    EXPECT_EQ(sectionItems(requirements, audioTitle), QStringList({
        muse::qtrc("project/convert", "%1 format").arg("MP3, WAV"),
        muse::qtrc("project/convert", "%1 max").arg(sizeText(m_config.audio2score.file.maxFileSizeBytes)),
        muse::qtrc("project/convert", "%n file(s) per conversion", nullptr, 1),
        muse::qtrc("project/convert", "Recommended for solo arrangements only")
    }));
}

TEST_F(Project_ConvertFileToScoreModelTest, FileRequirements_OnlyShowsSectionOfSelectedCategory)
{
    // [GIVEN] Limits for every file category
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.images.maxFiles = 15;
    m_config.audio2score.file.maxFiles = 1;

    // [WHEN] A PDF file is selected
    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));

    // [THEN] Only the PDF section is shown
    EXPECT_EQ(sectionTitles(m_model.fileRequirements()), QStringList({ muse::qtrc("project/convert", "PDF") }));

    // [WHEN] An image is selected
    m_model.load({ "/some/file.png" }, int(ConvertType::Omr));

    // [THEN] Only the images section is shown
    EXPECT_EQ(sectionTitles(m_model.fileRequirements()), QStringList({ muse::qtrc("project/convert", "Images") }));

    // [WHEN] An audio file is selected
    m_model.load({ "/some/file.mp3" }, int(ConvertType::Audio2Score));

    // [THEN] Only the audio section is shown
    EXPECT_EQ(sectionTitles(m_model.fileRequirements()), QStringList({ muse::qtrc("project/convert", "Audio") }));
}

TEST_F(Project_ConvertFileToScoreModelTest, FileRequirements_EmptyConfig_SkipsImagesAndAudioSections)
{
    // [GIVEN] A config without any limits
    const QString pdfTitle = muse::qtrc("project/convert", "PDF");

    // [WHEN] Reading the requirements
    const QVariantList requirements = m_model.fileRequirements();

    // [THEN] Only the PDF section remains, with no items
    EXPECT_EQ(sectionTitles(requirements), QStringList({ pdfTitle }));
    EXPECT_TRUE(sectionItems(requirements, pdfTitle).isEmpty());
}

TEST_F(Project_ConvertFileToScoreModelTest, FileRequirements_NotifiesOnPathsAndConvertTypeChange)
{
    // [THEN] Count the fileRequirementsChanged notifications
    int fileRequirementsChangedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::fileRequirementsChanged, [&fileRequirementsChangedCount]() {
        ++fileRequirementsChangedCount;
    });

    // [WHEN] Loading a file of a different convert type
    m_model.load({ "/some/file.mp3" }, int(ConvertType::Audio2Score));

    // [THEN] The requirements are notified for both the new paths and the new convert type
    EXPECT_EQ(fileRequirementsChangedCount, 2);

    // [WHEN] Removing the file
    m_model.fileListModel()->removeAt(0);

    // [THEN] The requirements are notified again
    EXPECT_EQ(fileRequirementsChangedCount, 3);
}

// ==================================================
// canSelectMultipleFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, CanSelectMultipleFiles_NoFiles_TrueIfAnyCategoryAllowsIt)
{
    // [GIVEN] Every category allows a single file only
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.images.maxFiles = 1;
    m_config.audio2score.file.maxFiles = 1;

    EXPECT_FALSE(m_model.canSelectMultipleFiles());

    // [GIVEN] Images allow multiple files
    m_config.omr.images.maxFiles = 15;

    EXPECT_TRUE(m_model.canSelectMultipleFiles());
}

TEST_F(Project_ConvertFileToScoreModelTest, CanSelectMultipleFiles_DependsOnSelectedCategory)
{
    // [GIVEN] Only images allow multiple files
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.images.maxFiles = 15;
    m_config.audio2score.file.maxFiles = 1;

    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));
    EXPECT_FALSE(m_model.canSelectMultipleFiles());

    m_model.load({ "/some/file.png" }, int(ConvertType::Omr));
    EXPECT_TRUE(m_model.canSelectMultipleFiles());

    m_model.load({ "/some/file.mp3" }, int(ConvertType::Audio2Score));
    EXPECT_FALSE(m_model.canSelectMultipleFiles());
}

TEST_F(Project_ConvertFileToScoreModelTest, CanSelectMultipleFiles_NoLimit_IsUnlimited)
{
    // [GIVEN] No limit on the number of audio files
    m_config.audio2score.file.maxFiles = 0;

    // [WHEN] An audio file is selected
    m_model.load({ "/some/file.mp3" }, int(ConvertType::Audio2Score));

    // [THEN] More files can be added
    EXPECT_TRUE(m_model.canSelectMultipleFiles());
}

TEST_F(Project_ConvertFileToScoreModelTest, CanSelectMultipleFiles_FalseWhenLinkIsSelected)
{
    // [GIVEN] Images allow multiple files
    m_config.omr.images.maxFiles = 15;

    // [WHEN] A link is selected
    m_model.setSelectedLink("test://link");

    // [THEN] Multiple files can't be selected
    EXPECT_FALSE(m_model.canSelectMultipleFiles());
}

// ==================================================
// Link hints
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, LinkHints_BothSources)
{
    // [GIVEN] Both link sources are allowed
    m_config.audio2score.link.allowedSources = LinkSource::YouTube | LinkSource::AudioCom;
    setAudioComUrl(AUDIO_COM_URL);

    // [THEN] The hints mention both of them
    EXPECT_EQ(m_model.linkHintText(),
              muse::qtrc("project/convert", "Or paste a link from %1 or %2 (beta)").arg(BOLD_YOUTUBE, BOLD_AUDIO_COM));
    EXPECT_EQ(m_model.linkPageHintText(),
              muse::qtrc("project/convert", "Paste a link from %1 or %2 (beta)").arg(BOLD_YOUTUBE, BOLD_AUDIO_COM));
    EXPECT_EQ(m_model.linkPageHintPlainText(),
              muse::qtrc("project/convert", "Paste a link from %1 or %2 (beta)").arg(QString("YouTube"), QString("Audio.com")));
    EXPECT_EQ(m_model.audioComUrl(), AUDIO_COM_URL);
}

TEST_F(Project_ConvertFileToScoreModelTest, LinkHints_YouTubeOnly)
{
    // [GIVEN] Only YouTube is allowed
    m_config.audio2score.link.allowedSources = LinkSource::YouTube;

    // [THEN] The hints mention YouTube only
    EXPECT_EQ(m_model.linkHintText(), muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(BOLD_YOUTUBE));
    EXPECT_EQ(m_model.linkPageHintText(), muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg(BOLD_YOUTUBE));
    EXPECT_EQ(m_model.linkPageHintPlainText(), muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg("YouTube"));
    EXPECT_TRUE(m_model.audioComUrl().isEmpty());
}

TEST_F(Project_ConvertFileToScoreModelTest, LinkHints_AudioComOnly)
{
    // [GIVEN] Only Audio.com is allowed
    m_config.audio2score.link.allowedSources = LinkSource::AudioCom;
    setAudioComUrl(AUDIO_COM_URL);

    // [THEN] The hints mention Audio.com only
    EXPECT_EQ(m_model.linkHintText(), muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(BOLD_AUDIO_COM));
    EXPECT_EQ(m_model.linkPageHintText(), muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg(BOLD_AUDIO_COM));
    EXPECT_EQ(m_model.linkPageHintPlainText(), muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg("Audio.com"));
    EXPECT_EQ(m_model.audioComUrl(), AUDIO_COM_URL);
}

TEST_F(Project_ConvertFileToScoreModelTest, LinkHints_NoSources_AreEmpty)
{
    // [GIVEN] No link sources are allowed
    m_config.audio2score.link.allowedSources = LinkSources();

    // [THEN] There are no hints
    EXPECT_TRUE(m_model.linkHintText().isEmpty());
    EXPECT_TRUE(m_model.linkPageHintText().isEmpty());
    EXPECT_TRUE(m_model.linkPageHintPlainText().isEmpty());
    EXPECT_TRUE(m_model.audioComUrl().isEmpty());
}

TEST_F(Project_ConvertFileToScoreModelTest, MaxLinkLength_ComesFromConfig)
{
    m_config.audio2score.link.maxLength = 255;

    EXPECT_EQ(m_model.maxLinkLength(), 255);
}

// ==================================================
// validateAndAddFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, ValidateAndAddFiles_Success_SetsFilesCategoryAndConvertType)
{
    // [GIVEN] The scenario accepts the file as an Audio2Score candidate
    expectValidateFiles({ "/some/song.xyz" }, ConvertType::Audio2Score, FileCategory::Audio);

    // [WHEN] Adding the file
    const bool ok = m_model.validateAndAddFiles({ "/some/song.xyz" });

    // [THEN] The file is added, with the category and convert type reported by the scenario
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ "/some/song.xyz" }));
    EXPECT_EQ(m_model.fileListModel()->fileCategory(), FileCategory::Audio);
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Audio2Score));
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateAndAddFiles_AppendsToExistingFiles)
{
    // [GIVEN] An already selected image
    m_model.load({ "/some/first.png" }, int(ConvertType::Omr));

    // [THEN] The existing and the new files are validated together
    expectValidateFiles({ "/some/first.png", "/some/second.png" }, ConvertType::Omr, FileCategory::Image);

    // [WHEN] Adding another image
    const bool ok = m_model.validateAndAddFiles({ "/some/second.png" });

    // [THEN] It is appended
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ "/some/first.png", "/some/second.png" }));
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateAndAddFiles_ConvertsFileUrlsToLocalPaths)
{
    // [THEN] The file url (e.g. from drag-and-drop) is validated as a local path
    expectValidateFiles({ "/some/dir/file.pdf" }, ConvertType::Omr, FileCategory::Pdf);

    // [WHEN] Adding a file url
    const bool ok = m_model.validateAndAddFiles({ "file:///some/dir/file.pdf" });

    // [THEN] The local path is added
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ "/some/dir/file.pdf" }));
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateAndAddFiles_Success_ClearsSelectedLink)
{
    // [GIVEN] A selected link
    m_model.setSelectedLink("test://link");

    expectValidateFiles({ "/some/file.pdf" }, ConvertType::Omr, FileCategory::Pdf);

    // [WHEN] Adding a file
    m_model.validateAndAddFiles({ "/some/file.pdf" });

    // [THEN] The file replaces the link
    EXPECT_TRUE(m_model.selectedLink().isEmpty());
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Omr));
    EXPECT_EQ(m_model.fileListModel()->rowCount(), 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateAndAddFiles_Failure_KeepsSelection)
{
    // [GIVEN] An already selected image
    const QStringList paths { "/some/first.png" };
    m_model.load(paths, int(ConvertType::Omr));

    // [GIVEN] The scenario rejects the files
    EXPECT_CALL(*m_scenario, validateFiles(_))
    .WillOnce(Return(RetVal<ConvertFilesValidation>::make_ret(make_ret(Ret::Code::UnknownError))));

    // [WHEN] Adding a file
    const bool ok = m_model.validateAndAddFiles({ "/some/file.xyz" });

    // [THEN] The selection is unchanged
    EXPECT_FALSE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), paths);
    EXPECT_EQ(m_model.fileListModel()->fileCategory(), FileCategory::Image);
    EXPECT_EQ(m_model.convertType(), int(ConvertType::Omr));
}

// ==================================================
// selectAndValidateFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, SelectAndValidateFiles_NoFiles_OffersAllSupportedFormats)
{
    // [GIVEN] The supported formats and the default directory
    m_config.omr.images.allowedExtensions = { "png", "jpg", "jpeg" };
    m_config.audio2score.file.allowedExtensions = { "mp3" };

    const io::path_t defaultDir = "/default/dir";
    ON_CALL(*m_configuration, defaultConvertFilePath())
    .WillByDefault(Return(defaultDir));

    // [THEN] The multiple files picker is opened, offering every supported format
    const io::path_t pickedFile = "/some/dir/file.png";
    EXPECT_CALL(*m_interactive,
                selectOpeningFilesSync(muse::trc("ui", "Choose file"), defaultDir,
                                       filePickerFilters("*.pdf *.png *.jpg *.jpeg *.mp3"), _))
    .WillOnce(Return(io::paths_t { pickedFile }));
    EXPECT_CALL(*m_interactive, selectOpeningFileSync(_, _, _, _)).Times(0);

    // [THEN] The directory of the picked file is remembered
    EXPECT_CALL(*m_configuration, setLastOpenedConvertFilePath(io::dirpath(pickedFile)))
    .Times(1);

    expectValidateFiles({ pickedFile }, ConvertType::Omr, FileCategory::Image);

    // [WHEN] Selecting files
    const bool ok = m_model.selectAndValidateFiles();

    // [THEN] The picked file is added
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ pickedFile.toQString() }));
}

TEST_F(Project_ConvertFileToScoreModelTest, SelectAndValidateFiles_SingleFileOnly_OpensSingleFilePicker)
{
    // [GIVEN] Every category allows a single file only
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.images.maxFiles = 1;
    m_config.audio2score.file.maxFiles = 1;

    // [THEN] The single file picker is opened
    const io::path_t pickedFile = "/some/dir/file.pdf";
    EXPECT_CALL(*m_interactive, selectOpeningFileSync(_, _, _, _))
    .WillOnce(Return(pickedFile));
    EXPECT_CALL(*m_interactive, selectOpeningFilesSync(_, _, _, _)).Times(0);

    expectValidateFiles({ pickedFile }, ConvertType::Omr, FileCategory::Pdf);

    // [WHEN] Selecting files
    const bool ok = m_model.selectAndValidateFiles();

    // [THEN] The picked file is added
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ pickedFile.toQString() }));
}

TEST_F(Project_ConvertFileToScoreModelTest, SelectAndValidateFiles_ExistingFiles_OffersOnlyTheirFormats)
{
    // [GIVEN] An already selected JPG image
    m_config.omr.images.allowedExtensions = { "png", "jpg", "jpeg" };
    m_model.load({ "/some/first.JPG" }, int(ConvertType::Omr));

    // [THEN] The picker only offers JPG/JPEG files
    EXPECT_CALL(*m_interactive, selectOpeningFilesSync(_, _, filePickerFilters("*.jpg *.jpeg"), _))
    .WillOnce(Return(io::paths_t { "/some/second.jpeg" }));

    expectValidateFiles({ "/some/first.JPG", "/some/second.jpeg" }, ConvertType::Omr, FileCategory::Image);

    // [WHEN] Selecting more files
    const bool ok = m_model.selectAndValidateFiles();

    // [THEN] The picked file is appended
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_model.fileListModel()->paths(), QStringList({ "/some/first.JPG", "/some/second.jpeg" }));
}

TEST_F(Project_ConvertFileToScoreModelTest, SelectAndValidateFiles_Cancelled_DoesNothing)
{
    // [GIVEN] The user cancels the file picker
    EXPECT_CALL(*m_interactive, selectOpeningFilesSync(_, _, _, _))
    .WillOnce(Return(io::paths_t()));

    // [THEN] Nothing is validated or remembered
    EXPECT_CALL(*m_scenario, validateFiles(_)).Times(0);
    EXPECT_CALL(*m_configuration, setLastOpenedConvertFilePath(_)).Times(0);

    // [WHEN] Selecting files
    const bool ok = m_model.selectAndValidateFiles();

    // [THEN] No files are added
    EXPECT_FALSE(ok);
    EXPECT_EQ(m_model.fileListModel()->rowCount(), 0);
}

// ==================================================
// validateLink()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, ValidateLink_DelegatesToScenario)
{
    EXPECT_CALL(*m_scenario, validateLink(QUrl("test://good-link")))
    .WillOnce(Return(make_ok()));
    EXPECT_CALL(*m_scenario, validateLink(QUrl("test://bad-link")))
    .WillOnce(Return(make_ret(Ret::Code::UnknownError)));

    EXPECT_TRUE(m_model.validateLink("test://good-link"));
    EXPECT_FALSE(m_model.validateLink("test://bad-link"));
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateLink_AddsSchemeToSchemelessLink)
{
    // [THEN] The scenario receives the link with a scheme
    EXPECT_CALL(*m_scenario, validateLink(QUrl("http://link.xyz/song")))
    .WillOnce(Return(make_ok()));

    // [WHEN] Validating a link typed without a scheme
    EXPECT_TRUE(m_model.validateLink("link.xyz/song"));
}

// ==================================================
// validateFileName()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, ValidateFileName_ValidOrEmpty_NoError)
{
    EXPECT_TRUE(m_model.validateFileName(QString()).isEmpty());
    EXPECT_TRUE(m_model.validateFileName("My score").isEmpty());
}

TEST_F(Project_ConvertFileToScoreModelTest, ValidateFileName_NotAllowed_ReturnsError)
{
    const QString name = "My:score?";
    const QString error = muse::qtrc("project/convert", "“%1” cannot be used as a file name. Please choose a different name.")
                          .arg(name);

    EXPECT_EQ(m_model.validateFileName(name), error);
}

// ==================================================
// confirmCancel() / confirmGoingBack()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, ConfirmCancel_UserConfirms_EmitsCancelConfirmed)
{
    // [GIVEN] The user confirms with "Yes, cancel"
    expectConfirmationQuestion(muse::trc("project/convert", "Are you sure you want to cancel?"), IInteractive::Button::Yes);

    // [THEN] Count the cancelConfirmed notifications
    int cancelConfirmedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::cancelConfirmed, [&cancelConfirmedCount]() { ++cancelConfirmedCount; });

    // [WHEN] Asking to cancel
    m_model.confirmCancel();
    async::processMessages();

    // [THEN] The cancellation is confirmed
    EXPECT_EQ(cancelConfirmedCount, 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, ConfirmCancel_UserStays_DoesNotEmit)
{
    // [GIVEN] The user dismisses with "No, stay here"
    expectConfirmationQuestion(muse::trc("project/convert", "Are you sure you want to cancel?"), IInteractive::Button::No);

    // [THEN] Count the cancelConfirmed notifications
    int cancelConfirmedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::cancelConfirmed, [&cancelConfirmedCount]() { ++cancelConfirmedCount; });

    // [WHEN] Asking to cancel
    m_model.confirmCancel();
    async::processMessages();

    // [THEN] Nothing is confirmed
    EXPECT_EQ(cancelConfirmedCount, 0);
}

TEST_F(Project_ConvertFileToScoreModelTest, ConfirmGoingBack_UserConfirms_EmitsGoingBackConfirmed)
{
    // [GIVEN] The user confirms with "Yes, go back"
    expectConfirmationQuestion(muse::trc("project/convert", "Are you sure you want to go back?"), IInteractive::Button::Yes);

    // [THEN] Count the goingBackConfirmed notifications
    int goingBackConfirmedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::goingBackConfirmed, [&goingBackConfirmedCount]() {
        ++goingBackConfirmedCount;
    });

    // [WHEN] Asking to go back
    m_model.confirmGoingBack();
    async::processMessages();

    // [THEN] Going back is confirmed
    EXPECT_EQ(goingBackConfirmedCount, 1);
}

TEST_F(Project_ConvertFileToScoreModelTest, ConfirmGoingBack_UserStays_DoesNotEmit)
{
    // [GIVEN] The user dismisses with "No, stay here"
    expectConfirmationQuestion(muse::trc("project/convert", "Are you sure you want to go back?"), IInteractive::Button::No);

    // [THEN] Count the goingBackConfirmed notifications
    int goingBackConfirmedCount = 0;
    QObject::connect(&m_model, &ConvertFileToScoreModel::goingBackConfirmed, [&goingBackConfirmedCount]() {
        ++goingBackConfirmedCount;
    });

    // [WHEN] Asking to go back
    m_model.confirmGoingBack();
    async::processMessages();

    // [THEN] Nothing is confirmed
    EXPECT_EQ(goingBackConfirmedCount, 0);
}

// ==================================================
// clearSelection()
// ==================================================

TEST_F(Project_ConvertFileToScoreModelTest, ClearSelection_ClearsFiles)
{
    // [GIVEN] A selected file
    m_model.load({ "/some/file.pdf" }, int(ConvertType::Omr));

    // [WHEN] Clearing the selection
    m_model.clearSelection();

    // [THEN] No files remain
    EXPECT_EQ(m_model.fileListModel()->rowCount(), 0);
    EXPECT_EQ(m_model.fileListModel()->fileCategory(), FileCategory::Unknown);
}

TEST_F(Project_ConvertFileToScoreModelTest, ClearSelection_ClearsLink)
{
    // [GIVEN] A selected link
    m_model.setSelectedLink("test://link");

    // [WHEN] Clearing the selection
    m_model.clearSelection();

    // [THEN] The link is cleared
    EXPECT_TRUE(m_model.selectedLink().isEmpty());
}
