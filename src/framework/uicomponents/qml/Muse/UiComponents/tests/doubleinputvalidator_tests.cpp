/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "../validators/doubleinputvalidator.h"

// Teach GoogleTest how to print QString so failure diffs are readable
// instead of a UTF-16 byte dump.
inline void PrintTo(const QString& s, std::ostream* os)
{
    *os << '"' << s.toStdString() << '"';
}

using namespace muse;
using namespace muse::uicomponents;

namespace muse::uicomponents {
class DoubleInputValidatorTests : public ::testing::Test, public QObject
{
public:
    void SetUp() override
    {
        m_validator = new DoubleInputValidator(nullptr);
    }

    void TearDown() override
    {
        delete m_validator;
    }

protected:
    DoubleInputValidator* m_validator = nullptr;
};

TEST_F(DoubleInputValidatorTests, ValidateDotLocale) {
    struct Input
    {
        QString str;
        QValidator::State expectedState;
        QString fixedStr = {};
    };

    // Use a dot-decimal locale explicitly
    QLocale prev = QLocale();
    QLocale::setDefault(QLocale("en_US"));

    m_validator->setTop(1000.0);
    m_validator->setBottom(-1000.0);
    m_validator->setDecimal(2);

    std::vector<Input> inputs = {
        { "-0.1", QValidator::Acceptable },
        { "0", QValidator::Acceptable },
        { "1.23", QValidator::Acceptable },
        { "99.99", QValidator::Acceptable },
        { "-100", QValidator::Acceptable },
        { "2.5", QValidator::Acceptable },
        { "0.0", QValidator::Intermediate, "0" },
        { "00.", QValidator::Intermediate, "0" },
        { "2.00", QValidator::Intermediate, "2" },
        { "2.", QValidator::Intermediate, "2" },
        { "-1000.1", QValidator::Intermediate, "-1,000" },
        { "1000.1", QValidator::Intermediate, "1,000" },
        { "1.123", QValidator::Invalid }, // more than 2 decimal places
        { "1000", QValidator::Acceptable, "1,000" },
        { "10000", QValidator::Invalid }, // more than top
        { "abc", QValidator::Invalid },
        { "", QValidator::Intermediate, "0" }
    };

    int pos = 0;
    for (Input& input : inputs) {
        EXPECT_EQ(m_validator->validate(input.str, pos), input.expectedState);

        if (QValidator::Invalid == input.expectedState) {
            continue;
        }

        QString fixInput = input.str;
        m_validator->fixup(fixInput);

        QString expectedStr = QValidator::Acceptable == input.expectedState ? input.str : input.fixedStr;
        EXPECT_EQ(expectedStr, fixInput);
    }

    // Restore previous locale
    QLocale::setDefault(prev);
}

TEST_F(DoubleInputValidatorTests, ValidateCommaLocale) {
    struct Input
    {
        QString str;
        QValidator::State expectedState;
        QString fixedStr = {};
    };

    // Use a comma-decimal locale explicitly
    QLocale prev = QLocale();
    QLocale::setDefault(QLocale("ro_RO"));

    m_validator->setTop(1000.0);
    m_validator->setBottom(-1000.0);
    m_validator->setDecimal(2);

    std::vector<Input> inputs = {
        { "-0,1", QValidator::Acceptable },
        { "0", QValidator::Acceptable },
        { "1,23", QValidator::Acceptable },
        { "99,99", QValidator::Acceptable },
        { "-100", QValidator::Acceptable },
        { "2,5", QValidator::Acceptable },
        { "0,0", QValidator::Intermediate, "0" },
        { "00,", QValidator::Intermediate, "0" },
        { "2,00", QValidator::Intermediate, "2" },
        { "2,", QValidator::Intermediate, "2" },
        { "-1000,1", QValidator::Intermediate, "-1.000" },
        { "1000,1", QValidator::Intermediate, "1.000" },
        { "1,123", QValidator::Invalid }, // more than 2 decimal places
        { "abc", QValidator::Invalid },
        { "", QValidator::Intermediate, "0" }
    };

    int pos = 0;
    for (Input& input : inputs) {
        EXPECT_EQ(m_validator->validate(input.str, pos), input.expectedState);

        if (QValidator::Invalid == input.expectedState) {
            continue;
        }

        QString fixInput = input.str;
        m_validator->fixup(fixInput);

        QString expectedStr = QValidator::Acceptable == input.expectedState ? input.str : input.fixedStr;
        EXPECT_EQ(expectedStr, fixInput);
    }

    // Restore previous locale
    QLocale::setDefault(prev);
}

TEST_F(DoubleInputValidatorTests, ValidateSmallRange) {
    struct Input
    {
        QString str;
        QValidator::State expectedState;
        QString fixedStr = {};
    };

    QLocale prev = QLocale();
    QLocale::setDefault(QLocale("en_US"));

    m_validator->setTop(1.0);
    m_validator->setBottom(0.0);
    m_validator->setDecimal(2);

    std::vector<Input> inputs = {
        { "0", QValidator::Acceptable },
        { "1", QValidator::Acceptable },
        { "0.5", QValidator::Acceptable },
        { "0.99", QValidator::Acceptable },
        { "-0.1", QValidator::Intermediate, "0" }, // below bottom, clamp to 0
        { "2", QValidator::Intermediate, "1" },    // above top, clamp to 1
        { "1.", QValidator::Intermediate, "1" },
        { "0.0", QValidator::Intermediate, "0" },
        { "10", QValidator::Invalid },             // too many integer digits
        { "1.123", QValidator::Invalid },          // more than 2 decimal places
        { "abc", QValidator::Invalid },
        { "", QValidator::Intermediate, "0" }
    };

    int pos = 0;
    for (Input& input : inputs) {
        EXPECT_EQ(m_validator->validate(input.str, pos), input.expectedState);

        if (QValidator::Invalid == input.expectedState) {
            continue;
        }

        QString fixInput = input.str;
        m_validator->fixup(fixInput);

        QString expectedStr = QValidator::Acceptable == input.expectedState ? input.str : input.fixedStr;
        EXPECT_EQ(expectedStr, fixInput);
    }

    QLocale::setDefault(prev);
}
}
