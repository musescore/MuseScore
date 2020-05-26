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

#include "msmrwriter.h"

#include <QBuffer>

#include "libmscore/score.h"

#include "avslog.h"
#include "msmrfile.h"

using namespace Ms::Avs;

MsmrWriter::MsmrWriter() {}

//---------------------------------------------------------
//   saveMsmrFile
//---------------------------------------------------------

bool MsmrWriter::saveMsmrFile(Ms::MasterScore* score, QIODevice* file, const QFileInfo& info)
{
    IF_ASSERT(score->avsOmr()) {
        return false;
    }

    std::shared_ptr<MsmrFile> msmr = score->avsOmr()->msmrFile();
    IF_ASSERT(msmr) {
        return false;
    }

    QByteArray mscz;
    QBuffer b(&mscz);
    bool ok = score->saveCompressedFile(&b, info, false, false);
    if (!ok) {
        LOGE() << "failed save mscz file";
        return false;
    }

    ok = msmr->writeMscz(mscz);
    if (!ok) {
        LOGE() << "failed save mscz data";
        return false;
    }

    ok = msmr->writeTo(file);
    if (!ok) {
        LOGE() << "failed write msmr file";
        return false;
    }

    return true;
}
