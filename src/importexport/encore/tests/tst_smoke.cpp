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

// Smoke test: verifies the test harness initializes and the stub importEncore() is reachable.

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>

#include "engraving/engravingerrors.h"
#include "../internal/importer/import.h"
#include "../internal/importer/import-options.h"

using namespace mu::engraving;
using namespace mu::iex::enc;

TEST(EncImporterSmoke, StubRejectsEveryFile)
{
    // The stub importEncore returns an error before any parser is built.
    Err err = importEncore(nullptr, "nonexistent.enc");
    EXPECT_NE(err, Err::NoError);
}

// Write a small file with the given leading bytes and return its path.
static QString writeTempFile(const QString& name, const QByteArray& bytes)
{
    const QString path = QDir::temp().filePath(name);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(bytes);
        f.close();
    }
    return path;
}

TEST(EncImporterErrors, UnreadableEncoreFileMessage)
{
    // A SCOW header with no valid body. The message does not separate a damaged Encore file from a
    // file that was never one: both are equally unopenable and the advice would be the same.
    const QString path = writeTempFile(QStringLiteral("enc_err_scow.enc"),
                                       QByteArray("SCOW") + QByteArray(1, '\xC4') + QByteArray(8, '\0'));
    const QString msg = encoreLoadErrorMessage(path).toQString();
    EXPECT_TRUE(msg.contains("Unrecognized Encore file", Qt::CaseInsensitive)) << msg.toStdString();
    QFile::remove(path);
}

// The .mus extension is shared with Finale and with Myriad's Melody and Harmony Assistant, so the
// message names the program and says how to get the score out of it.
TEST(EncImporterErrors, ForeignFormatMessagesNameTheProgram)
{
    struct Case {
        const char* file;
        QByteArray head;
        const char* expected;
    };
    const std::vector<Case> cases = {
        { "enc_err_finale.mus",   QByteArray("ENIGMA BINARY FILE") + QByteArray(16, '\0'),        "Finale" },
        { "enc_err_finale_etf.mus", QByteArray("ENIGMA TRANSPORTABLE FILE") + QByteArray(8, '\0'), "Finale" },
        { "enc_err_finale_pc.mus", QByteArray("Finale(R) PC 2.0 Copyright") + QByteArray(8, '\0'), "Finale" },
        { "enc_err_myriad.mus",   QByteArray("SOLF") + QByteArray(20, '\x7F'),                     "Melody Assistant" },
        { "enc_err_mtpro.mus",    QByteArray("RO\0\0", 4) + QByteArray("song.mts", 8),             "Master Tracks Pro" },
    };
    for (const Case& c : cases) {
        const QString path = writeTempFile(QString::fromLatin1(c.file), c.head);
        const QString msg = encoreLoadErrorMessage(path).toQString();
        EXPECT_TRUE(msg.contains(c.expected, Qt::CaseInsensitive)) << c.file << ": " << msg.toStdString();
        EXPECT_TRUE(msg.contains("export it as MusicXML", Qt::CaseInsensitive))
            << c.file << ": " << msg.toStdString();
        QFile::remove(path);
    }
}

TEST(EncImporterErrors, NotAnEncoreFileMessage)
{
    const QString path = writeTempFile(QStringLiteral("enc_err_noise.enc"),
                                       QByteArray("\x7F\x45\x4C\x46 random noise not encore", 28));
    const QString msg = encoreLoadErrorMessage(path).toQString();
    EXPECT_TRUE(msg.contains("Unrecognized Encore file", Qt::CaseInsensitive)) << msg.toStdString();
    QFile::remove(path);
}
