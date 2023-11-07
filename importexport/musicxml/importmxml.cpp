//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015 Werner Schweer and others
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

#include "libmscore/box.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"

#include "importmxml.h"
#include "importmxmllogger.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"

#include "mscore/preferences.h"

namespace Ms {

Score::FileError removeInstrumentNames(Score* score, MxmlLogger* logger)
      {
      for (Part* part : score->parts()) {
            logger->logDebugInfo(QString("Removing instrument names from '%1')")
                                    .arg(part->instrumentName()));
            part->setPlainLongName(QString(""));
            part->setPlainShortName(QString(""));
      }

      return Score::FileError::FILE_NO_ERROR;
      }

Score::FileError applyMusicXMLPVGStyles(Score* score, MxmlLogger* logger)
      {
            
      // Reset styles to default
      auto ignoreStyles = pageStyles();
      ignoreStyles.unite(fretStyles());
      ignoreStyles.insert(Sid::concertPitch);
      ignoreStyles.insert(Sid::createMultiMeasureRests);
      score->style().resetAllStyles(score, ignoreStyles);
      score->update();

      // Apply PVG MusicXML Style Sheet
      bool styleLoadResult = score->loadStyle(":/styles/PVGMusicXML.mss", /*ign*/false, /*overlap*/true);
      if (!styleLoadResult) {
            logger->logError(QString("importMusicXMLfromBuffer could not load :/styles/PVGMusicXML.mss"));
            return Score::FileError::FILE_NOT_FOUND;
      }

      // Remove instrument names
      if (preferences.getBool(PREF_IMPORT_MUSICXML_REMOVEINSTRUMENTNAMES))
            removeInstrumentNames(score, logger);

      return Score::FileError::FILE_NO_ERROR;
      }


//---------------------------------------------------------
//   musicXMLImportErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML import error(s).
 */

static int musicXMLImportErrorDialog(QString text, QString detailedText)
      {
      QMessageBox errorDialog;
      errorDialog.setIcon(QMessageBox::Question);
      errorDialog.setText(text);
      errorDialog.setInformativeText(QObject::tr("Do you want to try to load this file anyway?"));
      errorDialog.setDetailedText(detailedText);
      errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      errorDialog.setDefaultButton(QMessageBox::No);
      return errorDialog.exec();
      }

#if 0
static void updateNamesForAccidentals(Instrument* inst)
      {
      auto replace = [](QString name) {
            name = name.replace(QRegExp(
                                  R"(((?:^|\s)([A-Ga-g]|[Uu][Tt]|[Dd][Oo]|[Rr][EeÉé]|[MmSsTt][Ii]|[FfLl][Aa]|[Ss][Oo][Ll]))b(?=\s|$))"),
                                QString::fromStdString(R"($1♭)"));

            name = name.replace(QRegExp(
                                  R"(((?:^|\s)([A-Ga-g]|[Uu][Tt]|[Dd][Oo]|[Rr][EeÉé]|[MmSsTt][Ii]|[FfLl][Aa]|[Ss][Oo][Ll]))#(?=\s|$))"),
                                QString::fromStdString(R"($1♯)"));

            return name;
            };
      // change staff names from simple text (eg 'Eb') to text using accidental symbols (eg 'E♭')

      // Instrument::longNames() is const af so we need to make a deep copy, update it, and then set it again
      QList<StaffName> longNamesCopy = inst->longNames();
      for (StaffName& sn : longNamesCopy)
            sn.setName(replace(sn.name()));
      QList<StaffName> shortNamesCopy = inst->shortNames();
      for (StaffName& sn : shortNamesCopy)
            sn.setName(replace(sn.name()));
      inst->setLongNames(longNamesCopy);
      inst->setShortNames(shortNamesCopy);
      }
#endif

//---------------------------------------------------------
//   importMusicXMLfromBuffer
//---------------------------------------------------------

Score::FileError importMusicXMLfromBuffer(Score* score, const QString& /*name*/, QIODevice* dev)
      {
      //qDebug("importMusicXMLfromBuffer(score %p, name '%s', dev %p)",
      //       score, qPrintable(name), dev);

      MxmlLogger logger;
      logger.setLoggingLevel(MxmlLogger::Level::MXML_ERROR); // errors only
      //logger.setLoggingLevel(MxmlLogger::Level::MXML_INFO);
      //logger.setLoggingLevel(MxmlLogger::Level::MXML_TRACE); // also include tracing

      // Reset and apply styles (possibly unique to backend conversion job)
      Score::FileError styleRes = applyMusicXMLPVGStyles(score, &logger);      
      if (styleRes != Score::FileError::FILE_NO_ERROR)
            logger.logError(QString("Error while applying MusicXML styles. Continuing."));

      // pass 1
      dev->seek(0);
      MusicXMLParserPass1 pass1(score, &logger);
      Score::FileError res = pass1.parse(dev);
      const auto pass1_errors = pass1.errors();

      // pass 2
      MusicXMLParserPass2 pass2(score, pass1, &logger);
      if (res == Score::FileError::FILE_NO_ERROR) {
          dev->seek(0);
          res = pass2.parse(dev);
      }

#if 0
      for (const Part* part : score->parts()) {
            for (const auto& pair : *part->instruments()) {
                  pair.second->updateInstrumentId();
                  updateNamesForAccidentals(pair.second);
                  }
            }
#endif

      // report result
      const auto pass2_errors = pass2.errors();
      if (!(pass1_errors.isEmpty() && pass2_errors.isEmpty())) {
            if (!MScore::noGui) {
                  const QString text { QObject::tr("Error(s) found, import may be incomplete.") };
                  if (musicXMLImportErrorDialog(text, pass1.errors() + pass2.errors()) != QMessageBox::Yes)
                        res = Score::FileError::FILE_USER_ABORT;
                  }
            }

      return res;
      }

} // namespace Ms
