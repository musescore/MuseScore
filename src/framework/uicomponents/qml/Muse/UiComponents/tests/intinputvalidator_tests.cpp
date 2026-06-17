/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include <gtest/gtest.h>
#include <qvalidator.h>

#include "../validators/intinputvalidator.h"

// Teach GoogleTest how to print QString so failure diffs are readable
// instead of a UTF-16 byte dump.
inline void PrintTo(const QString& s, std::ostream* os)
{
    *os << '"' << s.toStdString() << '"';
}

using namespace muse;
using namespace muse::uicomponents;

namespace muse::uicomponents {
struct Input
{
    QString str;
    QValidator::State expectedState;
    QString fixedStr = {};
};

class IntInputValidatorTests : public ::testing::Test, public QObject
{
public:
    void SetUp() override
    {
        m_validator = new IntInputValidator(nullptr);
    }

    void TearDown() override
    {
        delete m_validator;
    }

protected:
    IntInputValidator* m_validator = nullptr;

    void runInputTests(const std::vector<Input>& inputs)
    {
        int pos = 0;
        for (const Input& input : inputs) {
            QString inputCopy = input.str;
            auto state = m_validator->validate(inputCopy, pos);
            EXPECT_EQ(state, input.expectedState) << "validate(\"" << input.str.toStdString() << "\")";

            if (QValidator::Invalid == input.expectedState) {
                continue;
            }

            QString fixInput = input.str;
            m_validator->fixup(fixInput);

            QString expectedStr = input.fixedStr.isEmpty() ? input.str : input.fixedStr;
            EXPECT_EQ(expectedStr, fixInput) << "fixup(\"" << input.str.toStdString() << "\")";
        }
    }
};

TEST_F(IntInputValidatorTests, ValidateCommaLocale) {
    QLocale prev = QLocale();
    QLocale::setDefault(QLocale("en_US"));

    m_validator->setTop(48000);
    m_validator->setBottom(-48000);

    runInputTests({
            { "0", QValidator::Acceptable },
            { "1", QValidator::Acceptable },
            { "100", QValidator::Acceptable },
            { "1,000", QValidator::Acceptable },
            { "1000", QValidator::Acceptable, "1,000" },
            { "1,0000", QValidator::Acceptable, "10,000" },
            { "48,000", QValidator::Acceptable },
            { "48,001", QValidator::Invalid },
            { "-100", QValidator::Acceptable },
            { "-1,000", QValidator::Acceptable },
            { "-1000", QValidator::Acceptable, "-1,000" },
            { "-1,0000", QValidator::Acceptable, "-10,000" },
            { "-48,000", QValidator::Acceptable },
            { "-48,001", QValidator::Invalid },
            { "2147483647", QValidator::Invalid },
            { "-2147483648", QValidator::Invalid },
            { "abc", QValidator::Invalid },
            { "", QValidator::Intermediate, "0" }
        });

    m_validator->setTop(10);
    m_validator->setBottom(1);

    runInputTests({
            { "0", QValidator::Invalid },
            { "", QValidator::Invalid },
            { "1", QValidator::Acceptable }
        });

    QLocale::setDefault(prev);
}

TEST_F(IntInputValidatorTests, ValidateDotLocale) {
    QLocale prev = QLocale();
    QLocale::setDefault(QLocale("ro_RO"));

    m_validator->setTop(48000);
    m_validator->setBottom(-48000);

    runInputTests({
            { "0", QValidator::Acceptable },
            { "1", QValidator::Acceptable },
            { "100", QValidator::Acceptable },
            { "1.000", QValidator::Acceptable },
            { "1000", QValidator::Acceptable, "1.000" },
            { "1.0000", QValidator::Acceptable, "10.000" },
            { "48.000", QValidator::Acceptable },
            { "48.001", QValidator::Invalid },
            { "-100", QValidator::Acceptable },
            { "-1.000", QValidator::Acceptable },
            { "-1000", QValidator::Acceptable, "-1.000" },
            { "-1.0000", QValidator::Acceptable, "-10.000" },
            { "-48.000", QValidator::Acceptable },
            { "-48.001", QValidator::Invalid },
            { "abc", QValidator::Invalid },
            { "", QValidator::Intermediate, "0" }
        });

    QLocale::setDefault(prev);
}
}
