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

#include <memory>
#include <QFileInfo>
#include <QByteArray>
#include <QIODevice>

#include "libmscore/score.h"
#include "libmscore/text.h"

#include "avslog.h"
#include "msmrfile.h"
#include "avsomr.h"
#include "avsomrreader.h"
#include "avsomrnetrecognizer.h"
#include "avsomrlocalrecognizer.h"
#include "avsomrlocal.h"
#include "ui/recognitionproccessdialog.h"
#include "ui/infopopup.h"

namespace Ms {
extern Score::FileError importMusicXml(MasterScore* score, QIODevice* dev, const QString& name);

//---------------------------------------------------------
//   doImportMSMR
//     return Score::FileError::FILE_* errors
//---------------------------------------------------------

static Score::FileError doImportMSMR(MasterScore* score,
                                     QIODevice* data,
                                     const QString& filePath,
                                     bool created)
{
    IF_ASSERT(score) {
        return Score::FileError::FILE_ERROR;
    }

    IF_ASSERT(data) {
        return Score::FileError::FILE_ERROR;
    }

    QString baseName = QFileInfo(filePath).baseName();
    auto msmr = std::make_shared<Avs::MsmrFile>(data->readAll(), baseName);

    // import score
    QByteArray msczData = msmr->readMscz();
    if (!msczData.isEmpty()) {
        // read mscz
        QBuffer msczBuf(&msczData);
        msczBuf.open(QIODevice::ReadOnly);
        Score::FileError err = score->loadMsc("score.mscz", &msczBuf, true);
        if (err != Score::FileError::FILE_NO_ERROR) {
            return err;
        }
    } else {
        // import MusicXml
        QByteArray muzicXmlData = msmr->readMuzicXml();
        if (muzicXmlData.isEmpty()) {
            return Score::FileError::FILE_BAD_FORMAT;
        }

        QBuffer muzicXmlBuf(&muzicXmlData);
        muzicXmlBuf.open(QIODevice::ReadOnly);

        Score::FileError err = importMusicXml(score, &muzicXmlBuf, baseName);
        if (err != Score::FileError::FILE_NO_ERROR) {
            return err;
        }

        // corrections after import

        //! NOTE Audiveris sets the title "[Audiveris detected movement]",
        //! I decided it was not convenient for the user,
        //! the file base name as title is more convenient.
        QString titleText = baseName;
        Text* title = score->getText(Tid::TITLE);
        if (title) {
            title->setPlainText(titleText);
        }

        score->setMetaTag("movementNumber", "");
        score->setMetaTag("movementTitle", "");
        score->setMetaTag("source", QFileInfo(filePath).fileName());
        score->setMetaTag("workTitle", title ? title->plainText() : titleText);
    }

    // import omr
    QByteArray ormData = msmr->readOmr();
    if (ormData.isEmpty()) {
        return Score::FileError::FILE_BAD_FORMAT;
    }

    QBuffer ormBuf(&ormData);
    Avs::AvsOmrReader reader;
    std::shared_ptr<Avs::AvsOmr> omr = reader.read(&ormBuf);
    if (!omr) {
        return Score::FileError::FILE_BAD_FORMAT;
    }

    omr->setMsmrFile(msmr);
    score->setAvsOmr(omr);

    // set format
    score->setCreated(created);

    //! NOTE format of file determined by .ext,
    //! therefore setting .msmr for correctly saving
    score->fileInfo()->setFile(QFileInfo(filePath).path() + "/" + baseName + ".msmr");

    return Score::FileError::FILE_NO_ERROR;
}

//---------------------------------------------------------
//   importMSMR
//     return Score::FileError::FILE_* errors
//---------------------------------------------------------

Score::FileError importMSMR(MasterScore* score, const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        LOGE() << "not exists file: " << filePath;
        return Score::FileError::FILE_NOT_FOUND;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open file: " << filePath;
        return Score::FileError::FILE_OPEN_ERROR;
    }

    return doImportMSMR(score, &file, filePath, false);
}

//---------------------------------------------------------
//   loadAndImportMSMR
//     return Score::FileError::FILE_* errors
//---------------------------------------------------------

Score::FileError loadAndImportMSMR(MasterScore* score, const QString& filePath)
{
    Avs::RecognitionProccessDialog progDialog;

    auto onStep = [&progDialog](const Avs::AvsOmrNetRecognizer::Step& step) {
                      progDialog.onStep(step);
                  };

    std::shared_ptr<Avs::IAvsOmrRecognizer> recognizer;
    bool useLocal = Avs::AvsOmrLocal::instance()->isUseLocal();
    if (useLocal) {
        recognizer = std::make_shared<Avs::AvsOmrLocalRecognizer>();
    } else {
        recognizer = std::make_shared<Avs::AvsOmrNetRecognizer>();
    }

    LOGI() << "try use avs omr recognizer: " << recognizer->type();
    progDialog.setType(recognizer->type());
    progDialog.show();

    QByteArray data;
    bool ok = recognizer->recognize(filePath, &data, onStep);

    progDialog.onFinished(ok);

    if (!ok) {
        //! NOTE A message has already been shown to the user
        return Score::FileError::FILE_IGNORE_ERROR;
    }

    QBuffer buf(&data);
    buf.open(QIODevice::ReadOnly);

    return doImportMSMR(score, &buf, filePath, true);
}
}
