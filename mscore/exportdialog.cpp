//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include "exportdialog.h"
#include "musescore.h"
#include "preferences.h"

// Supported export formats
// ------------------------
//  - PDF
//  - PNG Bitmap Graphic (*.png)
//  - Scalable Vector Graphics (*.svg)
//  - MP3 Audio (*.mp3)
//  - Wave Audio (*.wav)
//  - FLAC Audio (*.flac)
//  - Ogg Vorbis Audio (*.ogg)
//  - MIDI
//  - MusicXML:
//      - Compressed (*.mxl)
//      - Uncompressed (*.musicxml)
//      - Uncompressed (outdated) (*.xml)
//  - Uncompressed MuseScore File (*.mscx)

namespace Ms {

//---------------------------------------------------------
//   ExportScoreItem
//---------------------------------------------------------

ExportScoreItem::ExportScoreItem(Score* s, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      _score = s;
      setText(s->isMaster() ? ExportDialog::tr("Full Score") : s->title());
      }

//---------------------------------------------------------
//   ExportDialog
//---------------------------------------------------------

ExportDialog::ExportDialog(Score* s, QWidget* parent)
   : AbstractDialog(parent), cs(s)
      {
      setObjectName("ExportDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
            
      connect(listWidget, &QListWidget::itemChanged, this, &ExportDialog::setOkButtonEnabled);
      connect(fileTypeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(fileTypeChosen(int)));
      
      pdfSeparateOrSingleFiles = new QButtonGroup(this);
      pdfSeparateOrSingleFiles->addButton(pdfSeparateFilesRadioButton, 0);
      pdfSeparateOrSingleFiles->addButton(pdfOneFileRadioButton, 1);

#if !defined(HAS_AUDIOFILE) || !defined(USE_LAME)
      // Disable audio options that are unavailable
      // Source: https://stackoverflow.com/a/38915478
      QStandardItemModel* fileTypeComboBoxModel = qobject_cast<QStandardItemModel*>(fileTypeComboBox->model());
      Q_ASSERT(fileTypeComboBoxModel != nullptr);
# ifndef USE_LAME
      // Disable .mp3 option if unavailable
      QStandardItem* mp3Item = fileTypeComboBoxModel->item(3);
      mp3Item->setFlags(audioItem->flags() & ~Qt::ItemIsEnabled);
# endif
# ifndef HAS_AUDIOFILE
      // Disable .wav, .flac and .ogg options if unavailable
      for (int i = 4; i < 7; i++) {
            QStandardItem* audioItem = fileTypeComboBoxModel->item(i);
            audioItem->setFlags(audioItem->flags() & ~Qt::ItemIsEnabled);
            }
# endif
#endif
      
      fileTypeComboBox->setCurrentIndex(0);
      pageStack->setCurrentIndex(0);
      
      pdfSeparateFilesRadioButton->setChecked(true);

      audioSampleRate->clear();
      audioSampleRate->addItem(tr("32000"), 32000);
      audioSampleRate->addItem(tr("44100"), 44100); // default
      audioSampleRate->addItem(tr("48000"), 48000);

      mp3BitRate->clear();
      mp3BitRate->addItem( tr("32"),  32);
      mp3BitRate->addItem( tr("40"),  40);
      mp3BitRate->addItem( tr("48"),  48);
      mp3BitRate->addItem( tr("56"),  56);
      mp3BitRate->addItem( tr("64"),  64);
      mp3BitRate->addItem( tr("80"),  80);
      mp3BitRate->addItem( tr("96"),  96);
      mp3BitRate->addItem(tr("112"), 112);
      mp3BitRate->addItem(tr("128"), 128); // default
      mp3BitRate->addItem(tr("160"), 160);
      mp3BitRate->addItem(tr("192"), 192);
      mp3BitRate->addItem(tr("224"), 224);
      mp3BitRate->addItem(tr("256"), 256);
      mp3BitRate->addItem(tr("320"), 320);
      
      retranslate();
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void ExportDialog::retranslate()
      {
      retranslateUi(this);
      
      if (listWidget->item(0))
            listWidget->item(0)->setText(tr("Full Score"));
      
      fileTypeComboBox->setItemText(0, tr("PDF File"));
      fileTypeComboBox->setItemText(1, tr("PNG Images"));
      fileTypeComboBox->setItemText(2, tr("SVG Images"));
      fileTypeComboBox->setItemText(3, tr("MP3 Audio"));
      fileTypeComboBox->setItemText(4, tr("WAV Audio"));
      fileTypeComboBox->setItemText(5, tr("FLAC Audio"));
      fileTypeComboBox->setItemText(6, tr("OGG Audio"));
      fileTypeComboBox->setItemText(7, tr("MIDI"));
      fileTypeComboBox->setItemText(8, tr("MusicXML"));
      fileTypeComboBox->setItemText(9, tr("Uncompressed MuseScore File") + " (*.mscx)");
      
      musicxmlFileTypeComboBox->setItemText(0, tr("Compressed") + " (*.mxl)");
      musicxmlFileTypeComboBox->setItemText(1, tr("Uncompressed") + " (*.musicxml)");
      musicxmlFileTypeComboBox->setItemText(2, tr("Uncompressed (deprecated)") + " (*.xml)");

      buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export…"));
      }

//---------------------------------------------------------
//   loadValues
///   Load the preferences values into the UI
//---------------------------------------------------------

void ExportDialog::loadValues()
      {
      pdfDpiSpinbox->setValue(preferences.getInt(PREF_EXPORT_PDF_DPI));
      
      pngDpiSpinbox->setValue(preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION));
      pngTransparentBackgroundCheckBox->setChecked(preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY));
      
      audioNormaliseCheckBox->setChecked(preferences.getBool(PREF_EXPORT_AUDIO_NORMALIZE));
      int audioSampleRateIndex = audioSampleRate->findData(preferences.getInt(PREF_EXPORT_AUDIO_SAMPLERATE));
      if (audioSampleRateIndex == -1)
            audioSampleRateIndex = audioSampleRate->findData(preferences.defaultValue(PREF_EXPORT_AUDIO_SAMPLERATE));
      audioSampleRate->setCurrentIndex(audioSampleRateIndex);
      int mp3BitRateIndex = mp3BitRate->findData(preferences.getInt(PREF_EXPORT_MP3_BITRATE));
      if (mp3BitRateIndex == -1)
            mp3BitRateIndex = mp3BitRate->findData(preferences.defaultValue(PREF_EXPORT_MP3_BITRATE));
      mp3BitRate->setCurrentIndex(mp3BitRateIndex);
      
      midiExpandRepeats->setChecked(preferences.getBool(PREF_IO_MIDI_EXPANDREPEATS));
      midiExportRPNs->setChecked(preferences.getBool(PREF_IO_MIDI_EXPORTRPNS));
      
      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            musicxmlExportAllLayout->setChecked(true);
      } else {
            switch (preferences.musicxmlExportBreaks()) {
                  case MusicxmlExportBreaks::ALL:
                        musicxmlExportAllBreaks->setChecked(true);
                        break;
                  case MusicxmlExportBreaks::MANUAL:
                        musicxmlExportManualBreaks->setChecked(true);
                        break;
                  case MusicxmlExportBreaks::NO:
                        musicxmlExportNoBreaks->setChecked(true);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   loadScoreAndPartsList
//---------------------------------------------------------

void ExportDialog::loadScoreAndPartsList()
      {
      listWidget->clear();
      
      ExportScoreItem* scoreItem = new ExportScoreItem(cs->masterScore()->score());
      listWidget->addItem(scoreItem);
      
      for (Excerpt* e : cs->masterScore()->excerpts()) {
            Score* s = e->partScore();
            ExportScoreItem* item = new ExportScoreItem(s);
            item->setChecked(s == cs);
            listWidget->addItem(item);
            }
      }

//---------------------------------------------------------
//   selection
//    Change the selection in the list of scores/parts.
//---------------------------------------------------------

// Select all parts and the score
void ExportDialog::selectAll()
      {
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            item->setChecked(true);
            }
      }

// Select the currently opened score/part
void ExportDialog::selectCurrent()
      {
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            item->setChecked(item->score() == cs);
            }
      }

// Select the score
void ExportDialog::selectScore()
      {
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            item->setChecked(item->score()->isMaster());
            }
      }

// Select the parts
void ExportDialog::selectParts()
      {
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            item->setChecked(!item->score()->isMaster());
            }
      }

// Select nothing
void ExportDialog::clearSelection()
      {
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            item->setChecked(false);
            }
      }

//---------------------------------------------------------
//   setOkButtonEnabled
//---------------------------------------------------------

void ExportDialog::setOkButtonEnabled()
      {
      bool isAnythingSelected = false;
      for (int i = 0; i < listWidget->count(); i++) {
            ExportScoreItem* item = static_cast<ExportScoreItem*>(listWidget->item(i));
            if (item->isChecked()) {
                  isAnythingSelected = true;
                  break;
                  }
            }
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isAnythingSelected);
      }

//---------------------------------------------------------
//   fileTypeChosen
//---------------------------------------------------------

void ExportDialog::fileTypeChosen(int index)
      {
      if (index <= 2) // Pdf, png and svg
            pageStack->setCurrentIndex(index);
      else if (index <= 6) { // Audio formats share their page (because they share many settings)
            pageStack->setCurrentWidget(audioPage);
            mp3BitRateLabel->setVisible(index == 3);
            mp3BitRate->setVisible(index == 3);
            mp3kBitSLabel->setVisible(index == 3);
            }
      else // And others have their own page again
            pageStack->setCurrentIndex(index - 3);
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void ExportDialog::setType(const QString& type)
      {
      if (type == "pdf")
            fileTypeComboBox->setCurrentIndex(0);
      else if (type == "png")
            fileTypeComboBox->setCurrentIndex(1);
      else if (type == "svg")
            fileTypeComboBox->setCurrentIndex(2);
#ifdef USE_LAME
      else if (type == "mp3")
            fileTypeComboBox->setCurrentIndex(3);
#endif
#ifdef HAS_AUDIOFILE
      else if (type == "wav")
            fileTypeComboBox->setCurrentIndex(4);
      else if (type == "flac")
            fileTypeComboBox->setCurrentIndex(5);
      else if (type == "ogg")
            fileTypeComboBox->setCurrentIndex(6);
#endif
      else if (type == "midi")
            fileTypeComboBox->setCurrentIndex(7);
      else if (type == "musicxml")
            fileTypeComboBox->setCurrentIndex(8);
      else if (type == "uncompressed")
            fileTypeComboBox->setCurrentIndex(9);
      else // Default
            return setType("pdf");
      }

//---------------------------------------------------------
//   showExportDialog
//---------------------------------------------------------

void MuseScore::showExportDialog(const QString& type)
      {
      if (!exportDialog)
            exportDialog = new ExportDialog(cs, this);
      else
            exportDialog->setScore(cs);
      
      if (type.size() > 0)
            exportDialog->setType(type);
      exportDialog->exec();
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ExportDialog::accept()
{
      // TODO: Do the actual exporting
      QDialog::accept();
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void ExportDialog::showEvent(QShowEvent* event)
      {
      loadValues();
      loadScoreAndPartsList();
      selectScore();

      QDialog::showEvent(event);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void ExportDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }
}
