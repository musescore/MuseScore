//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include "musescoredialogs.h"
#include "icons.h"
#include "preferences.h"
#include "musescore.h"
#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
// InsertMeasuresDialog
//---------------------------------------------------------

InsertMeasuresDialog::InsertMeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("InsertMeasuresDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);
      insmeasures->setFocus();
      insmeasures->selectAll();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void InsertMeasuresDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch (buttonBox->buttonRole(button)) {
            case QDialogButtonBox::AcceptRole:
                  accept();
                  // fall through
            case QDialogButtonBox::RejectRole:
                  close();
            default:
                  break;
            }
      }

//---------------------------------------------------------
// Insert Measure -->   accept
//---------------------------------------------------------

void InsertMeasuresDialog::accept()
      {
      int n = insmeasures->value();
      if (mscore->currentScore())
            mscore->currentScoreView()->cmdInsertMeasures(n, ElementType::MEASURE);
      done(1);
      }

//---------------------------------------------------------
// InsertMeasuresDialog hideEvent
//---------------------------------------------------------

void InsertMeasuresDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

MeasuresDialog::MeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("MeasuresDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);
      measures->setFocus();
      measures->selectAll();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void MeasuresDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch (buttonBox->buttonRole(button)) {
            case QDialogButtonBox::AcceptRole:
                  accept();
                  // fall through
            case QDialogButtonBox::RejectRole:
                  close();
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MeasuresDialog::accept()
      {
      int n = measures->value();
      if (mscore->currentScore())
            mscore->currentScoreView()->cmdAppendMeasures(n, ElementType::MEASURE);
      done(1);
      }

//---------------------------------------------------------
// MeasuresDialog hideEvent
//---------------------------------------------------------

void MeasuresDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }


//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutBoxDialog::AboutBoxDialog()
      {
      setupUi(this);
      museLogo->setPixmap(QPixmap(preferences.isThemeDark() ?
            ":/data/musescore-logo-transbg-m.png" : ":/data/musescore_logo_full.png"));

      if (MuseScore::unstable())
            versionLabel->setText(tr("Unstable Prerelease for Version: %1").arg(VERSION) + tr(" Evolution"));
      else {
            auto msVersion = QString(VERSION);
            if (strlen(BUILD_NUMBER))
                  msVersion += QString(".") + QString(BUILD_NUMBER);// +QString(" Beta");
            versionLabel->setText(tr("Version: %1").arg(msVersion) + tr(" Evolution"));
      }

      revisionLabel->setText(tr("Revision: %1").arg(revision));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      auto compilerDateISO = []() -> std::string {
            // Store __DATE__ in ISO 8601 format ( e.g. 2025-04-20 )
            int month, day, year;
            char buffer[10];
            static const char monthNames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
            sscanf(__DATE__, "%s %d %d", buffer, &day, &year);
            month = ((strstr(monthNames, buffer) - monthNames) / 3) + 1;
            sprintf(buffer, "%d-%02d-%02d", year, month, day);
            return std::string(buffer);
            };

      std::string dateTime;
      dateTime += preferences.getBool(PREF_UI_APP_BUILD_DATE_ISO) ? compilerDateISO() : __DATE__;
      dateTime += " ";
      dateTime += __TIME__;

      buildDateLabel->setText(tr("Build date: %1").arg(dateTime.c_str()));

      QString visitAndDonateString;
#if !defined(FOR_WINSTORE) && 0
      visitAndDonateString = tr("Visit %1 for new versions and more information.\nGet %2help%3 with the program or %4contribute%5 to its development.")
                  .arg("<a href=\"https://www.musescore.org/\">www.musescore.org</a>",
                       "<a href=\"https://www.musescore.org/forum\">", "</a>",
                       "<a href=\"https://www.musescore.org/contribute\">", "</a>");
      visitAndDonateString += "\n\n";
#endif
      QString finalString = visitAndDonateString + tr("Copyright &copy; 1999-2025 MuseScore BVBA and others.\nPublished under the %1GNU General Public License version 2%2.")
                  .arg("<a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">", "</a>");
      finalString.replace("\n", "<br/>");
      copyrightLabel->setText(QString("<span style=\"font-size:10pt;\">%1</span>").arg(finalString));
      connect(copyRevisionButton, SIGNAL(clicked()), this, SLOT(copyRevisionToClipboard()));
      copyRevisionButton->setIcon(*icons[int(Icons::copy_ICON)]);
      }

//---------------------------------------------------------
//   copyRevisionToClipboard
//---------------------------------------------------------

void AboutBoxDialog::copyRevisionToClipboard()
      {
      QClipboard* cb = QApplication::clipboard();
      QString sysinfo = "OS: ";
      sysinfo += QSysInfo::prettyProductName();
      if (QSysInfo::productType() == "windows" && (QSysInfo::productVersion() == "10" || QSysInfo::productVersion() == "11"))
            sysinfo += " or later";
      sysinfo += ", Arch.: ";
      sysinfo += QSysInfo::currentCpuArchitecture();
      // endianness?
      sysinfo += ", MuseScore version (";
      sysinfo += QSysInfo::WordSize==32 ? "32" : "64";
      sysinfo += "-bit): " + QString(VERSION);
      if (strlen(BUILD_NUMBER))
            sysinfo += QString(".") + QString(BUILD_NUMBER);
      sysinfo += ", revision: ";
      sysinfo += "GitHub-Jojo-Schmitz-MuseScore-";
      sysinfo += revision;
      cb->setText(sysinfo);
      }

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutMusicXMLBoxDialog::AboutMusicXMLBoxDialog()
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      label->setText(QString("<span style=\"font-size:10pt;\">%1<br/></span>")
                     .arg(tr(   "MusicXML is an open file format for exchanging digital sheet music,\n"
                                "supported by many applications.\n"
                                "Copyright © 2004-2017 the Contributors to the MusicXML\n"
                                "Specification, published by the W3C Music Notation Community\n"
                                "Group under the W3C Community Final Specification Agreement:\n%1\n"
                                "A human-readable summary is available:\n%2")
                          .arg( "\n&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"https://www.w3.org/community/about/process/final/\">https://www.w3.org/community/about/process/final/</a>\n",
                                "\n&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"https://www.w3.org/community/about/process/fsa-deed/\">https://www.w3.org/community/about/process/fsa-deed/</a>\n")
                          .replace("\n","<br/>")));
      }

} // namespace Ms
