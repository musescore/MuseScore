/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include <QProcess>
#include <QTextStream>

static const QString ROOT_DIR(VTEST_ROOT_DIR);
static const QString MSCORE_REF_BIN(VTEST_MSCORE_REF_BIN);
static const QString MSCORE_BIN(VTEST_MSCORE_BIN);
static const QString REF_DIR("./reference_pngs");
static const QString CURRENT_DIR("./current_pngs");

class Engraving_VTest : public ::testing::Test
{
public:
};

static int run_command(const QString& name, const QStringList& args)
{
    QString path = ROOT_DIR + "/" + name;

    QProcess p;

    QObject::connect(&p, &QProcess::readyReadStandardOutput, [&p]() {
        QByteArray ba = p.readAllStandardOutput();
        QTextStream outputText(stdout);
        outputText << QString(ba);
    });

    p.start(path, args);

    if (!p.waitForFinished(60000 * 5)) {
        return -1;
    }

    int code = p.exitCode();
    return code;
}

TEST_F(Engraving_VTest, 1_GenerateRef)
{
    ASSERT_EQ(run_command("vtest-generate-pngs.sh",
                          { "--mscore", MSCORE_REF_BIN,
                            "--output-dir", REF_DIR
                          }), 0);
}

TEST_F(Engraving_VTest, 2_GenerateCurrentAndCompare)
{
    // GenerateCurrent
    ASSERT_EQ(run_command("vtest-generate-pngs.sh",
                          { "--mscore", MSCORE_BIN,
                            "--output-dir", CURRENT_DIR
                          }), 0);

    // Compare
    ASSERT_EQ(run_command("vtest-compare-pngs.sh",
                          { "--gen-gif", "0"
                          }), 0);
}
