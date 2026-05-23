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

// The bad-format message must identify why a file was rejected and how to recover.
TEST(EncImporterErrors, EncryptedContainerMessage)
{
    for (const char* magic : { "ZBOT", "ZBOP", "ZBO6" }) {
        const QString path = writeTempFile(QStringLiteral("enc_err_%1.enc").arg(magic),
                                           QByteArray(magic) + QByteArray(40, '\0'));
        const QString msg = encoreLoadErrorMessage(path).toQString();
        EXPECT_TRUE(msg.contains("encrypted", Qt::CaseInsensitive))
            << magic << " -> " << msg.toStdString();
        EXPECT_TRUE(msg.contains(QString::fromLatin1(magic)))
            << "message should name the detected container: " << msg.toStdString();
        QFile::remove(path);
    }
}

TEST(EncImporterErrors, UnreadableEncoreFileMessage)
{
    // A SCOW header with no valid body: recognizable Encore file, but not parseable.
    const QString path = writeTempFile(QStringLiteral("enc_err_scow.enc"),
                                       QByteArray("SCOW") + QByteArray(1, '\xC4') + QByteArray(8, '\0'));
    const QString msg = encoreLoadErrorMessage(path).toQString();
    EXPECT_TRUE(msg.contains("could not be read", Qt::CaseInsensitive)) << msg.toStdString();
    EXPECT_TRUE(msg.contains("0xc4", Qt::CaseInsensitive))
        << "message should report the detected format version byte: " << msg.toStdString();
    QFile::remove(path);
}

TEST(EncImporterErrors, NotAnEncoreFileMessage)
{
    const QString path = writeTempFile(QStringLiteral("enc_err_noise.enc"),
                                       QByteArray("\x7F\x45\x4C\x46 random noise not encore", 28));
    const QString msg = encoreLoadErrorMessage(path).toQString();
    EXPECT_TRUE(msg.contains("not a recognized Encore file", Qt::CaseInsensitive)) << msg.toStdString();
    QFile::remove(path);
}
