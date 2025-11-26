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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/spellcheckermock.h"

#include "notation/internal/lyricsspellcheckservice.h"
#include "modularity/ioc.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

using namespace mu::notation;

class Notation_LyricsSpellCheckTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_spellChecker = std::make_shared<NiceMock<SpellCheckerMock> >();

        // Set up the mock spellchecker to be available by default
        ON_CALL(*m_spellChecker, isAvailable()).WillByDefault(Return(true));
        ON_CALL(*m_spellChecker, language()).WillByDefault(Return("en_US"));

        // Register mock in IoC - the service will pick it up
        muse::modularity::globalIoc()->unregister<ISpellChecker>("lyricsspellcheck_tests");
        muse::modularity::globalIoc()->registerExport<ISpellChecker>("lyricsspellcheck_tests", m_spellChecker);

        m_service = std::make_unique<LyricsSpellCheckService>();
    }

    void TearDown() override
    {
        m_service.reset();
        muse::modularity::globalIoc()->unregister<ISpellChecker>("lyricsspellcheck_tests");
    }

    std::shared_ptr<SpellCheckerMock> m_spellChecker;
    std::unique_ptr<LyricsSpellCheckService> m_service;
};

/**
 * Test that the service correctly reports availability based on the spellchecker
 */
TEST_F(Notation_LyricsSpellCheckTests, isAvailable_WhenSpellCheckerAvailable_ReturnsTrue)
{
    ON_CALL(*m_spellChecker, isAvailable()).WillByDefault(Return(true));
    EXPECT_TRUE(m_service->isAvailable());
}

TEST_F(Notation_LyricsSpellCheckTests, isAvailable_WhenSpellCheckerUnavailable_ReturnsFalse)
{
    ON_CALL(*m_spellChecker, isAvailable()).WillByDefault(Return(false));
    EXPECT_FALSE(m_service->isAvailable());
}

/**
 * Test that available languages are passed through from spellchecker
 */
TEST_F(Notation_LyricsSpellCheckTests, availableLanguages_ReturnsSpellCheckerLanguages)
{
    QStringList expectedLanguages = { "en_US", "en_GB", "de_DE" };
    ON_CALL(*m_spellChecker, availableLanguages()).WillByDefault(Return(expectedLanguages));

    QStringList result = m_service->availableLanguages();
    EXPECT_EQ(result, expectedLanguages);
}

/**
 * Test that current language is passed through from spellchecker
 */
TEST_F(Notation_LyricsSpellCheckTests, currentLanguage_ReturnsSpellCheckerLanguage)
{
    ON_CALL(*m_spellChecker, language()).WillByDefault(Return("de_DE"));

    QString result = m_service->currentLanguage();
    EXPECT_EQ(result, "de_DE");
}

/**
 * Test that setLanguage delegates to spellchecker and returns success
 */
TEST_F(Notation_LyricsSpellCheckTests, setLanguage_WhenSuccessful_ReturnsTrue)
{
    EXPECT_CALL(*m_spellChecker, setLanguage(QString("fr_FR"))).WillOnce(Return(true));
    EXPECT_TRUE(m_service->setLanguage("fr_FR"));
}

/**
 * Test that setLanguage delegates to spellchecker and returns failure
 */
TEST_F(Notation_LyricsSpellCheckTests, setLanguage_WhenFailed_ReturnsFalse)
{
    EXPECT_CALL(*m_spellChecker, setLanguage(QString("invalid"))).WillOnce(Return(false));
    EXPECT_FALSE(m_service->setLanguage("invalid"));
}

/**
 * Test that checkLyrics returns error when spellchecker is unavailable
 */
TEST_F(Notation_LyricsSpellCheckTests, checkLyrics_WhenSpellCheckerUnavailable_ReturnsError)
{
    ON_CALL(*m_spellChecker, isAvailable()).WillByDefault(Return(false));

    LyricsSpellCheckResult result = m_service->checkLyrics(nullptr);

    EXPECT_FALSE(result.spellCheckerAvailable);
    EXPECT_FALSE(result.errorMessage.isEmpty());
}

/**
 * Test that checkLyrics returns error when notation is null
 */
TEST_F(Notation_LyricsSpellCheckTests, checkLyrics_WhenNotationNull_ReturnsError)
{
    LyricsSpellCheckResult result = m_service->checkLyrics(nullptr);

    EXPECT_TRUE(result.spellCheckerAvailable);
    EXPECT_FALSE(result.errorMessage.isEmpty());
}
