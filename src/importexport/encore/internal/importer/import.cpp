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

// Top-level Encore (.enc) import: read the file, build the score, and run whole-score fix-up passes.
// Binary format reverse-engineered by Leon Vinken (Enc2MusicXML, GPL v3+) building on enc2ly by Felipe Castro.

#include "ctx.h"
#include "builders.h"
#include "debug-dump.h"

#include "import.h"

#include "../parser/elem.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <map>
#include <set>
#include <vector>

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/system.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/mscore.h"
#include "engraving/types/spatium.h"
#include "engraving/engravingerrors.h"

#include "engraving/editing/editenharmonicspelling.h"
#include "engraving/editing/implodeexplode.h"
#include "engraving/editing/editvoice.h"
#include "engraving/editing/transaction/transaction.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
static void buildScore(MasterScore* score, const EncRoot& enc, const EncImportOptions& opts)
{
    ScoreLoad sl;   // import edits run outside any undo transaction; see mergeNonOverlappingVoices

    BuildCtx ctx{ score, enc, opts };
    buildParts(ctx);
    // Assign MIDI ports/channels to every part. The file read path does this on load,
    // but a direct import builds the score in memory without it, leaving each channel
    // at -1; that makes Part::midiPort() index m_midiMapping[-1] and crash on a
    // straight-to-MusicXML export.
    score->rebuildMidiMapping();
    score->setUpTempoMap();
    score->doLayout();
}

muse::String encoreLoadErrorMessage(const QString& path)
{
    QByteArray head;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        head = file.read(5);
    }
    const QByteArray magic = head.left(4);
    const muse::String name = muse::String::fromQString(QFileInfo(path).fileName());

    // Older encrypted Encore container (ZBOT/ZBOP/ZBO6).
    if (magic == "ZBOT" || magic == "ZBOP" || magic == "ZBO6") {
        return muse::mtrc("engraving",
                          "“%1” is in an older, encrypted Encore format (%2) that this importer cannot read. "
                          "Open it in Encore and save it again, then import the saved file.")
               .arg(name).arg(muse::String::fromQString(QString::fromLatin1(magic)));
    }
    // Recognizable Encore header, but the file could not be parsed: unsupported variant, damaged,
    // or empty.
    if (magic == "SCOW" || magic == "SCO5") {
        const QString ver = QStringLiteral("%1").arg(
            head.size() >= 5 ? static_cast<unsigned char>(head[4]) : 0, 2, 16, QChar('0'));
        return muse::mtrc("engraving",
                          "“%1” could not be read as an Encore file (format version 0x%2). It may be damaged "
                          "or use an unsupported variant. Try opening it in Encore and saving it again, then "
                          "import the saved file.")
               .arg(name).arg(muse::String::fromQString(ver));
    }
    // No recognizable Encore header at all.
    return muse::mtrc("engraving",
                      "“%1” is not a recognized Encore file. Its header does not match any known Encore "
                      "format, so it may be corrupted or a different type of file.")
           .arg(name);
}

Err importEncore(MasterScore* score, const QString& path, const EncImportOptions& opts)
{
    if (!QFileInfo::exists(path)) {
        return Err::FileNotFound;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return Err::FileOpenError;
    }

    // ZBOT/ZBOP/ZBO6 are older encrypted Encore containers (only the first 42 bytes decrypt with
    // a known XOR key; see ENCORE_FORMAT.md). Not supported here. See MuseScore#24341.
    {
        QByteArray magic4 = file.read(4);
        file.seek(0);
        if (magic4 == "ZBOT" || magic4 == "ZBOP" || magic4 == "ZBO6") {
            LOGW() << "Encore: encrypted format (" << magic4.toStdString()
                   << ") is not supported; re-save the file in Encore as an unencrypted file first.";
            return Err::FileBadFormat;
        }
    }

    QDataStream ds(&file);
    ds.setByteOrder(QDataStream::LittleEndian);

    EncRoot enc;
    if (!enc.read(ds)) {
        return Err::FileBadFormat;
    }

    if (enc.instruments.empty() || enc.measures.empty()) {
        return Err::FileBadFormat;
    }

    logEncRootInfo(enc);
    buildScore(score, enc, opts);

    muse::Ret integrity = score->sanityCheck();
    if (!integrity) {
        LOGW() << "Encore import: score corruption detected:\n" << integrity.text();
    }

    return Err::NoError;
}
} // namespace mu::iex::enc
