//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstyle.cpp 5637 2012-05-16 14:23:09Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "libmscore/score.h"
#include "scoreview.h"
#include "libmscore/style.h"
#include "editstyle.h"
#include "libmscore/articulation.h"
#include "libmscore/sym.h"
#include "icons.h"
#include "musescore.h"
#include "libmscore/undo.h"
#include "texteditor.h"
#include "libmscore/harmony.h"
#include "libmscore/chordlist.h"
#include "libmscore/figuredbass.h"

extern QString iconPath, iconGroup;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      cs     = s;
      lstyle = *s->style();
      setModal(true);

      chordDescriptionFileButton->setIcon(*icons[fileOpen_ICON]);

      stemGroups[0] = new QButtonGroup(this);
      stemGroups[0]->addButton(voice1Up);
      stemGroups[0]->addButton(voice1Down);

      stemGroups[1] = new QButtonGroup(this);
      stemGroups[1]->addButton(voice2Up);
      stemGroups[1]->addButton(voice2Down);

      stemGroups[2] = new QButtonGroup(this);
      stemGroups[2]->addButton(voice3Up);
      stemGroups[2]->addButton(voice3Down);

      stemGroups[3] = new QButtonGroup(this);
      stemGroups[3]->addButton(voice4Up);
      stemGroups[3]->addButton(voice4Down);

      pageList->setCurrentRow(0);

      articulationTable->verticalHeader()->setVisible(false);
      articulationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      QStringList headers;
      headers << tr("Symbol") << tr("Anchor");
      articulationTable->setHorizontalHeaderLabels(headers);
      articulationTable->setColumnWidth(0, 200);
      articulationTable->setColumnWidth(1, 180);
      articulationTable->setRowCount(ARTICULATIONS);

      for (int i = 0; i < ARTICULATIONS; ++i) {
            ArticulationInfo* ai = &Articulation::articulationList[i];

//            QIcon icon(iconPath + iconGroup + ai->name + ".svg");
//            QTableWidgetItem* item = new QTableWidgetItem(icon, qApp->translate("articulation", qPrintable(ai->name)));
            QTableWidgetItem* item = new QTableWidgetItem(qApp->translate("articulation", qPrintable(ai->name)));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            articulationTable->setItem(i, 0, item);

            QComboBox* cb = new QComboBox();
            cb->addItem(tr("TopStaff"), A_TOP_STAFF);
            cb->addItem(tr("BottomStaff"), A_BOTTOM_STAFF);
            cb->addItem(tr("Chord"), A_CHORD);
            articulationTable->setCellWidget(i, 1, cb);
            }
      QButtonGroup* bg = new QButtonGroup(this);
      bg->addButton(editEvenHeaderL, 0);
      bg->addButton(editEvenHeaderC, 1);
      bg->addButton(editEvenHeaderR, 2);
      bg->addButton(editOddHeaderL,  3);
      bg->addButton(editOddHeaderC,  4);
      bg->addButton(editOddHeaderR,  5);

      bg->addButton(editEvenFooterL, 6);
      bg->addButton(editEvenFooterC, 7);
      bg->addButton(editEvenFooterR, 8);
      bg->addButton(editOddFooterL,  9);
      bg->addButton(editOddFooterC, 10);
      bg->addButton(editOddFooterR, 11);

      // figured bass init
      QList<QString> fbFontNames = FiguredBass::fontNames();
      foreach(const QString& family, fbFontNames)
            comboFBFont->addItem(family);
      comboFBFont->setCurrentIndex(0);
      connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

      setValues();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(chordDescriptionFileButton, SIGNAL(clicked()), SLOT(selectChordDescriptionFile()));

      connect(bg, SIGNAL(buttonClicked(int)), SLOT(editTextClicked(int)));
      }

//---------------------------------------------------------
//   buttonClicked
//---------------------------------------------------------

void EditStyle::buttonClicked(QAbstractButton* b)
      {
      switch (buttonBox->standardButton(b)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
                  done(1);
                  break;
            default:
                  if(cs->undo() && cs->undo()->current()) {
                        cs->undo()->current()->unwind();
                        cs->setLayoutAll(true);
                        }
                  done(0);
                  break;
            }
      }

//---------------------------------------------------------
//   On comboFBFont currentIndex changed
//---------------------------------------------------------

void EditStyle::on_comboFBFont_currentIndexChanged(int index)
{
      qreal       size, lineHeight;

      if(FiguredBass::fontData(index, 0, 0, &size, &lineHeight)) {
            doubleSpinFBSize->setValue(size);
            spinFBLineHeight->setValue((int)(lineHeight * 100.0));
      }
}
//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStyle::apply()
      {
      getValues();
      cs->undo(new ChangeStyle(cs, lstyle));
      cs->setLayoutAll(true);
      cs->end2();
      cs->end();
      }

//---------------------------------------------------------
//   getValues
//---------------------------------------------------------

void EditStyle::getValues()
      {
      lstyle.set(ST_staffUpperBorder,        Spatium(staffUpperBorder->value()));
      lstyle.set(ST_staffLowerBorder,        Spatium(staffLowerBorder->value()));
      lstyle.set(ST_staffDistance,           Spatium(staffDistance->value()));
      lstyle.set(ST_akkoladeDistance,        Spatium(akkoladeDistance->value()));
      lstyle.set(ST_minSystemDistance,       Spatium(minSystemDistance->value()));
      lstyle.set(ST_maxSystemDistance,       Spatium(maxSystemDistance->value()));
      lstyle.set(ST_lyricsDistance,          Spatium(lyricsDistance->value()));
      lstyle.set(ST_lyricsMinBottomDistance, Spatium(lyricsMinBottomDistance->value()));
      lstyle.set(ST_lyricsLineHeight,        Spatium(lyricsLineHeight->value() * .01));
      lstyle.set(ST_systemFrameDistance,     Spatium(systemFrameDistance->value()));
      lstyle.set(ST_frameSystemDistance,     Spatium(frameSystemDistance->value()));
      lstyle.set(ST_minMeasureWidth,         Spatium(minMeasureWidth_2->value()));

      lstyle.set(ST_barWidth,                Spatium(barWidth->value()));
      lstyle.set(ST_endBarWidth,             Spatium(endBarWidth->value()));
      lstyle.set(ST_endBarDistance,          Spatium(endBarDistance->value()));
      lstyle.set(ST_doubleBarWidth,          Spatium(doubleBarWidth->value()));
      lstyle.set(ST_doubleBarDistance,       Spatium(doubleBarDistance->value()));

      lstyle.set(ST_repeatBarTips,           showRepeatBarTips->isChecked());
      lstyle.set(ST_startBarlineSingle,      showStartBarlineSingle->isChecked());
      lstyle.set(ST_startBarlineMultiple,    showStartBarlineMultiple->isChecked());

      lstyle.set(ST_measureSpacing,          measureSpacing->value());
      lstyle.set(ST_minNoteDistance,         Spatium(minNoteDistance->value()));
      lstyle.set(ST_barNoteDistance,         Spatium(barNoteDistance->value()));
      lstyle.set(ST_noteBarDistance,         Spatium(noteBarDistance->value()));
      lstyle.set(ST_showMeasureNumber,       showMeasureNumber->isChecked());
      lstyle.set(ST_showMeasureNumberOne,    showFirstMeasureNumber->isChecked());
      lstyle.set(ST_measureNumberInterval,   intervalMeasureNumber->value());
      lstyle.set(ST_measureNumberSystem,     showEverySystemMeasureNumber->isChecked());
      lstyle.set(ST_measureNumberAllStaffs,  showAllStaffsMeasureNumber->isChecked());
      lstyle.set(ST_clefLeftMargin,          Spatium(clefLeftMargin->value()));
      lstyle.set(ST_keysigLeftMargin,        Spatium(keysigLeftMargin->value()));
      lstyle.set(ST_timesigLeftMargin,       Spatium(timesigLeftMargin->value()));
      lstyle.set(ST_clefKeyRightMargin,      Spatium(clefKeyRightMargin->value()));
      lstyle.set(ST_clefBarlineDistance,     Spatium(clefBarlineDistance->value()));
      lstyle.set(ST_beginRepeatLeftMargin,   Spatium(beginRepeatLeftMargin->value()));
      lstyle.set(ST_staffLineWidth,          Spatium(staffLineWidth->value()));
      lstyle.set(ST_beamWidth,               Spatium(beamWidth->value()));
      lstyle.set(ST_beamDistance,            beamDistance->value());
      lstyle.set(ST_beamMinLen,              Spatium(beamMinLen->value()));
      lstyle.set(ST_beamMinSlope,            beamMinSlope->value());
      lstyle.set(ST_beamMaxSlope,            beamMaxSlope->value());
      lstyle.set(ST_graceNoteMag,            graceNoteSize->value() * 0.01);
      lstyle.set(ST_smallStaffMag,           smallStaffSize->value() * 0.01);
      lstyle.set(ST_smallNoteMag,            smallNoteSize->value() * 0.01);
      lstyle.set(ST_smallClefMag,            smallClefSize->value() * 0.01);
//      lstyle.set(ST_pageFillLimit,           pageFillThreshold->value() * 0.01);
      lstyle.set(ST_lastSystemFillLimit,     lastSystemFillThreshold->value() * 0.01);
      lstyle.set(ST_hairpinY,                Spatium(hairpinY->value()));
      lstyle.set(ST_hairpinWidth,            Spatium(hairpinLineWidth->value()));
      lstyle.set(ST_hairpinHeight,           Spatium(hairpinHeight->value()));
      lstyle.set(ST_hairpinContHeight,       Spatium(hairpinContinueHeight->value()));
      lstyle.set(ST_genClef,                 genClef->isChecked());
      lstyle.set(ST_genKeysig,               genKeysig->isChecked());
      lstyle.set(ST_genTimesig,              genTimesig->isChecked());
      lstyle.set(ST_genCourtesyTimesig,      genCourtesyTimesig->isChecked());
      lstyle.set(ST_genCourtesyKeysig,       genCourtesyKeysig->isChecked());
      lstyle.set(ST_genCourtesyClef,         genCourtesyClef->isChecked());

      lstyle.set(ST_useGermanNoteNames,      useGermanNoteNames->isChecked());

      if (lstyle.valueSt(ST_chordDescriptionFile) != chordDescriptionFile->text()) {
            ChordList* cl = new ChordList();
            cl->read("chords.xml");
            cl->read(chordDescriptionFile->text());
            lstyle.setChordList(cl);
            lstyle.set(ST_chordDescriptionFile, chordDescriptionFile->text());
            }

      lstyle.set(ST_concertPitch,            concertPitch->isChecked());
      lstyle.set(ST_createMultiMeasureRests, multiMeasureRests->isChecked());
      lstyle.set(ST_minEmptyMeasures,        minEmptyMeasures->value());
      lstyle.set(ST_minMMRestWidth,          Spatium(minMeasureWidth->value()));
      lstyle.set(ST_hideEmptyStaves,         hideEmptyStaves->isChecked());
      lstyle.set(ST_dontHideStavesInFirstSystem, dontHideStavesInFirstSystem->isChecked());

      lstyle.set(ST_accidentalNoteDistance,  Spatium(accidentalNoteDistance->value()));
      lstyle.set(ST_accidentalDistance,      Spatium(accidentalDistance->value()));
      lstyle.set(ST_dotNoteDistance,         Spatium(noteDotDistance->value()));
      lstyle.set(ST_dotDotDistance,          Spatium(dotDotDistance->value()));
      lstyle.set(ST_ledgerLineWidth,         Spatium(ledgerLineWidth->value()));
      lstyle.set(ST_ledgerLineLength,        Spatium(ledgerLineLength->value()));

      lstyle.set(ST_bracketWidth,            Spatium(bracketWidth->value()));
      lstyle.set(ST_bracketDistance,         Spatium(bracketDistance->value()));
      lstyle.set(ST_akkoladeWidth,           Spatium(akkoladeWidth->value()));
      lstyle.set(ST_akkoladeBarDistance,     Spatium(akkoladeBarDistance->value()));

      lstyle.set(ST_propertyDistanceHead,    Spatium(propertyDistanceHead->value()));
      lstyle.set(ST_propertyDistanceStem,    Spatium(propertyDistanceStem->value()));
      lstyle.set(ST_propertyDistance,        Spatium(propertyDistance->value()));
      lstyle.set(ST_stemDir1,                voice1Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir2,                voice2Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir3,                voice3Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir4,                voice4Up->isChecked() ? UP : DOWN);

      lstyle.set(ST_shortenStem,             shortenStem->isChecked());
      lstyle.set(ST_shortStemProgression,    Spatium(shortStemProgression->value()));
      lstyle.set(ST_shortestStem,            Spatium(shortestStem->value()));

      lstyle.set(ST_ArpeggioNoteDistance,    Spatium(arpeggioNoteDistance->value()));
      lstyle.set(ST_ArpeggioLineWidth,       Spatium(arpeggioLineWidth->value()));
      lstyle.set(ST_ArpeggioHookLen,         Spatium(arpeggioHookLen->value()));

      lstyle.set(ST_FixMeasureNumbers,       fixNumberMeasures->value());
      lstyle.set(ST_FixMeasureWidth,         fixMeasureWidth->isChecked());

      lstyle.set(ST_SlurEndWidth,            Spatium(slurEndLineWidth->value()));
      lstyle.set(ST_SlurMidWidth,            Spatium(slurMidLineWidth->value()));
      lstyle.set(ST_SlurDottedWidth,         Spatium(slurDottedLineWidth->value()));
      lstyle.set(ST_SlurBow,                 Spatium(slurBow->value()));

      lstyle.set(ST_MusicalSymbolFont,       musicalSymbolFont->currentText());

      lstyle.set(ST_showHeader,      showHeader->isChecked());
      lstyle.set(ST_headerStyled,    headerStyled->isChecked());
      lstyle.set(ST_headerFirstPage, showHeaderFirstPage->isChecked());
      lstyle.set(ST_headerOddEven,   headerOddEven->isChecked());
      if (headerStyled->isChecked()) {
            lstyle.set(ST_evenHeaderL, evenHeaderL->toPlainText());
            lstyle.set(ST_evenHeaderC, evenHeaderC->toPlainText());
            lstyle.set(ST_evenHeaderR, evenHeaderR->toPlainText());
            lstyle.set(ST_oddHeaderL,  oddHeaderL->toPlainText());
            lstyle.set(ST_oddHeaderC,  oddHeaderC->toPlainText());
            lstyle.set(ST_oddHeaderR,  oddHeaderR->toPlainText());
            }
      else {
            lstyle.set(ST_evenHeaderL, evenHeaderL->toHtml());
            lstyle.set(ST_evenHeaderC, evenHeaderC->toHtml());
            lstyle.set(ST_evenHeaderR, evenHeaderR->toHtml());
            lstyle.set(ST_oddHeaderL,  oddHeaderL->toHtml());
            lstyle.set(ST_oddHeaderC,  oddHeaderC->toHtml());
            lstyle.set(ST_oddHeaderR,  oddHeaderR->toHtml());
            }

      lstyle.set(ST_showFooter,      showFooter->isChecked());
      lstyle.set(ST_footerStyled,    footerStyled->isChecked());
      lstyle.set(ST_footerFirstPage, showFooterFirstPage->isChecked());
      lstyle.set(ST_footerOddEven,   footerOddEven->isChecked());
      if (footerStyled->isChecked()) {
            lstyle.set(ST_evenFooterL, evenFooterL->toPlainText());
            lstyle.set(ST_evenFooterC, evenFooterC->toPlainText());
            lstyle.set(ST_evenFooterR, evenFooterR->toPlainText());
            lstyle.set(ST_oddFooterL,  oddFooterL->toPlainText());
            lstyle.set(ST_oddFooterC,  oddFooterC->toPlainText());
            lstyle.set(ST_oddFooterR,  oddFooterR->toPlainText());
            }
      else {
            lstyle.set(ST_evenFooterL, evenFooterL->toHtml());
            lstyle.set(ST_evenFooterC, evenFooterC->toHtml());
            lstyle.set(ST_evenFooterR, evenFooterR->toHtml());
            lstyle.set(ST_oddFooterL,  oddFooterL->toHtml());
            lstyle.set(ST_oddFooterC,  oddFooterC->toHtml());
            lstyle.set(ST_oddFooterR,  oddFooterR->toHtml());
            }

      // figured bass
      int         idx = comboFBFont->currentIndex();
      QString     family;
      if(FiguredBass::fontData(idx, &family, 0, 0, 0))
            lstyle.set(ST_figuredBassFontFamily, family);
      qreal size = doubleSpinFBSize->value();
      lstyle.set(ST_figuredBassFontSize,   size);
      lstyle.set(ST_figuredBassYOffset,    doubleSpinFBVertPos->value());
      lstyle.set(ST_figuredBassLineHeight, ((double)spinFBLineHeight->value()) / 100.0);
      lstyle.set(ST_figuredBassStyle,      radioFBModern->isChecked() ? 0 : 1);
      // copy to text style data relevant to it (LineHeight and Style are not in
      // text style, and text style YOffset is not used by FB layout)
      const TextStyle fbOld = lstyle.textStyle(TEXT_STYLE_FIGURED_BASS);
      if(family != fbOld.family() || size != fbOld.size()) {
            TextStyle fbNew(fbOld);
            fbNew.setFamily(family);
            fbNew.setSize(size);
            lstyle.setTextStyle(fbNew);
      }

      for (int i = 0; i < ARTICULATIONS; ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            lstyle.setArticulationAnchor(i, ArticulationAnchor(cb->itemData(cb->currentIndex()).toInt()));
            }

//      lstyle.set(ST_warnPitchRange,  warnPitchRange->isChecked());

      lstyle.set(ST_voltaY,                  Spatium(voltaY->value()));
      lstyle.set(ST_voltaHook,               Spatium(voltaHook->value()));
      lstyle.set(ST_voltaLineWidth,          Spatium(voltaLineWidth->value()));

      lstyle.set(ST_ottavaY,                 Spatium(ottavaY->value()));
      lstyle.set(ST_ottavaHook,              Spatium(ottavaHook->value()));
      lstyle.set(ST_ottavaLineWidth,         Spatium(ottavaLineWidth->value()));

      lstyle.set(ST_tabClef, clefTab1->isChecked() ? CLEF_TAB : CLEF_TAB2);
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      staffUpperBorder->setValue(lstyle.value(ST_staffUpperBorder).toSpatium().val());
      staffLowerBorder->setValue(lstyle.value(ST_staffLowerBorder).toSpatium().val());
      staffDistance->setValue(lstyle.value(ST_staffDistance).toSpatium().val());
      akkoladeDistance->setValue(lstyle.value(ST_akkoladeDistance).toSpatium().val());
      minSystemDistance->setValue(lstyle.value(ST_minSystemDistance).toSpatium().val());
      maxSystemDistance->setValue(lstyle.value(ST_maxSystemDistance).toSpatium().val());
      lyricsDistance->setValue(lstyle.value(ST_lyricsDistance).toSpatium().val());
      lyricsMinBottomDistance->setValue(lstyle.value(ST_lyricsMinBottomDistance).toSpatium().val());
      lyricsLineHeight->setValue(lstyle.value(ST_lyricsLineHeight).toDouble() * 100.0);
      systemFrameDistance->setValue(lstyle.value(ST_systemFrameDistance).toSpatium().val());
      frameSystemDistance->setValue(lstyle.value(ST_frameSystemDistance).toSpatium().val());
      minMeasureWidth_2->setValue(lstyle.value(ST_minMeasureWidth).toSpatium().val());

      barWidth->setValue(lstyle.value(ST_barWidth).toSpatium().val());
      endBarWidth->setValue(lstyle.value(ST_endBarWidth).toSpatium().val());
      endBarDistance->setValue(lstyle.value(ST_endBarDistance).toSpatium().val());
      doubleBarWidth->setValue(lstyle.value(ST_doubleBarWidth).toSpatium().val());
      doubleBarDistance->setValue(lstyle.value(ST_doubleBarDistance).toSpatium().val());

      showRepeatBarTips->setChecked(lstyle.value(ST_repeatBarTips).toBool());
      showStartBarlineSingle->setChecked(lstyle.value(ST_startBarlineSingle).toBool());
      showStartBarlineMultiple->setChecked(lstyle.value(ST_startBarlineMultiple).toBool());

      measureSpacing->setValue(lstyle.value(ST_measureSpacing).toDouble());
      minNoteDistance->setValue(lstyle.value(ST_minNoteDistance).toSpatium().val());
      barNoteDistance->setValue(lstyle.value(ST_barNoteDistance).toSpatium().val());
      noteBarDistance->setValue(lstyle.value(ST_noteBarDistance).toSpatium().val());

      showMeasureNumber->setChecked(lstyle.value(ST_showMeasureNumber).toBool());
      showFirstMeasureNumber->setChecked(lstyle.value(ST_showMeasureNumberOne).toBool());
      intervalMeasureNumber->setValue(lstyle.value(ST_measureNumberInterval).toInt());
      showIntervalMeasureNumber->setChecked(!lstyle.value(ST_measureNumberSystem).toBool());
      showAllStaffsMeasureNumber->setChecked(lstyle.value(ST_measureNumberAllStaffs).toBool());
      showEverySystemMeasureNumber->setChecked(lstyle.value(ST_measureNumberSystem).toBool());

      clefLeftMargin->setValue(lstyle.value(ST_clefLeftMargin).toSpatium().val());
      keysigLeftMargin->setValue(lstyle.value(ST_keysigLeftMargin).toSpatium().val());
      timesigLeftMargin->setValue(lstyle.value(ST_timesigLeftMargin).toSpatium().val());
      clefKeyRightMargin->setValue(lstyle.value(ST_clefKeyRightMargin).toSpatium().val());
      clefBarlineDistance->setValue(lstyle.value(ST_clefBarlineDistance).toSpatium().val());
      beginRepeatLeftMargin->setValue(lstyle.value(ST_beginRepeatLeftMargin).toSpatium().val());
      staffLineWidth->setValue(lstyle.value(ST_staffLineWidth).toSpatium().val());

      beamWidth->setValue(lstyle.value(ST_beamWidth).toSpatium().val());
      beamDistance->setValue(lstyle.value(ST_beamDistance).toDouble());
      beamMinLen->setValue(lstyle.value(ST_beamMinLen).toSpatium().val());
      beamMinSlope->setValue(lstyle.value(ST_beamMinSlope).toDouble());
      beamMaxSlope->setValue(lstyle.value(ST_beamMaxSlope).toDouble());

      graceNoteSize->setValue(lstyle.value(ST_graceNoteMag).toDouble() * 100.0);
      smallStaffSize->setValue(lstyle.value(ST_smallStaffMag).toDouble() * 100.0);
      smallNoteSize->setValue(lstyle.value(ST_smallNoteMag).toDouble() * 100.0);
      smallClefSize->setValue(lstyle.value(ST_smallClefMag).toDouble() * 100.0);
//      pageFillThreshold->setValue(lstyle.value(ST_pageFillLimit).toDouble() * 100.0);
      lastSystemFillThreshold->setValue(lstyle.value(ST_lastSystemFillLimit).toDouble() * 100.0);

      hairpinY->setValue(lstyle.value(ST_hairpinY).toSpatium().val());
      hairpinLineWidth->setValue(lstyle.value(ST_hairpinWidth).toSpatium().val());
      hairpinHeight->setValue(lstyle.value(ST_hairpinHeight).toSpatium().val());
      hairpinContinueHeight->setValue(lstyle.value(ST_hairpinContHeight).toSpatium().val());

      genClef->setChecked(lstyle.value(ST_genClef).toBool());
      genKeysig->setChecked(lstyle.value(ST_genKeysig).toBool());
      genTimesig->setChecked(lstyle.value(ST_genTimesig).toBool());
      genCourtesyTimesig->setChecked(lstyle.value(ST_genCourtesyTimesig).toBool());
      genCourtesyKeysig->setChecked(lstyle.value(ST_genCourtesyKeysig).toBool());
      genCourtesyClef->setChecked(lstyle.value(ST_genCourtesyClef).toBool());

      useGermanNoteNames->setChecked(lstyle.value(ST_useGermanNoteNames).toBool());
      QString s(lstyle.value(ST_chordDescriptionFile).toString());
      chordDescriptionFile->setText(s);
      concertPitch->setChecked(lstyle.value(ST_concertPitch).toBool());

      multiMeasureRests->setChecked(lstyle.value(ST_createMultiMeasureRests).toBool());
      minEmptyMeasures->setValue(lstyle.value(ST_minEmptyMeasures).toInt());
      minMeasureWidth->setValue(lstyle.value(ST_minMMRestWidth).toSpatium().val());
      hideEmptyStaves->setChecked(lstyle.value(ST_hideEmptyStaves).toBool());
      dontHideStavesInFirstSystem->setChecked(lstyle.value(ST_dontHideStavesInFirstSystem).toBool());

      accidentalNoteDistance->setValue(lstyle.value(ST_accidentalNoteDistance).toSpatium().val());
      accidentalDistance->setValue(lstyle.value(ST_accidentalDistance).toSpatium().val());
      noteDotDistance->setValue(lstyle.value(ST_dotNoteDistance).toSpatium().val());
      dotDotDistance->setValue(lstyle.value(ST_dotDotDistance).toSpatium().val());
      ledgerLineWidth->setValue(lstyle.value(ST_ledgerLineWidth).toSpatium().val());
      ledgerLineLength->setValue(lstyle.value(ST_ledgerLineLength).toSpatium().val());

      bracketWidth->setValue(lstyle.value(ST_bracketWidth).toSpatium().val());
      bracketDistance->setValue(lstyle.value(ST_bracketDistance).toSpatium().val());
      akkoladeWidth->setValue(lstyle.value(ST_akkoladeWidth).toSpatium().val());
      akkoladeBarDistance->setValue(lstyle.value(ST_akkoladeBarDistance).toSpatium().val());

      propertyDistanceHead->setValue(lstyle.value(ST_propertyDistanceHead).toSpatium().val());
      propertyDistanceStem->setValue(lstyle.value(ST_propertyDistanceStem).toSpatium().val());
      propertyDistance->setValue(lstyle.value(ST_propertyDistance).toSpatium().val());

      voice1Up->setChecked(lstyle.value(ST_stemDir1).toDirection() == UP);
      voice2Up->setChecked(lstyle.value(ST_stemDir2).toDirection() == UP);
      voice3Up->setChecked(lstyle.value(ST_stemDir3).toDirection() == UP);
      voice4Up->setChecked(lstyle.value(ST_stemDir4).toDirection() == UP);

      voice1Down->setChecked(lstyle.value(ST_stemDir1).toDirection() != UP);
      voice2Down->setChecked(lstyle.value(ST_stemDir2).toDirection() != UP);
      voice3Down->setChecked(lstyle.value(ST_stemDir3).toDirection() != UP);
      voice4Down->setChecked(lstyle.value(ST_stemDir4).toDirection() != UP);

      shortenStem->setChecked(lstyle.value(ST_shortenStem).toBool());
      shortStemProgression->setValue(lstyle.value(ST_shortStemProgression).toSpatium().val());
      shortestStem->setValue(lstyle.value(ST_shortestStem).toSpatium().val());
      arpeggioNoteDistance->setValue(lstyle.value(ST_ArpeggioNoteDistance).toSpatium().val());
      arpeggioLineWidth->setValue(lstyle.value(ST_ArpeggioLineWidth).toSpatium().val());
      arpeggioHookLen->setValue(lstyle.value(ST_ArpeggioHookLen).toSpatium().val());

      // figured bass
      for(int i = 0; i < comboFBFont->count(); i++)
            if(comboFBFont->itemText(i) == lstyle.value(ST_figuredBassFontFamily).toString()) {
                  comboFBFont->setCurrentIndex(i);
                  break;
            }
      doubleSpinFBSize->setValue(lstyle.value(ST_figuredBassFontSize).toDouble());
      doubleSpinFBVertPos->setValue(lstyle.value(ST_figuredBassYOffset).toDouble());
      spinFBLineHeight->setValue(lstyle.value(ST_figuredBassLineHeight).toSpatium().val() * 100.0);
      radioFBModern->setChecked(lstyle.value(ST_figuredBassStyle).toInt() == 0);
      radioFBHistoric->setChecked(lstyle.value(ST_figuredBassStyle).toInt() == 1);

      for (int i = 0; i < ARTICULATIONS; ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            if (cb == 0)
                  continue;
            int st  = lstyle.articulationAnchor(i);
            int idx = 0;
            if (st == A_TOP_STAFF)
                  idx = 0;
            else if (st == A_BOTTOM_STAFF)
                  idx = 1;
            else if (st == A_CHORD)
                  idx = 2;
            cb->setCurrentIndex(idx);
            }
//      warnPitchRange->setChecked(lstyle[ST_warnPitchRange].toBool());

      fixNumberMeasures->setValue(lstyle.value(ST_FixMeasureNumbers).toInt());
      fixMeasureWidth->setChecked(lstyle.value(ST_FixMeasureWidth).toBool());

      slurEndLineWidth->setValue(lstyle.value(ST_SlurEndWidth).toSpatium().val());
      slurMidLineWidth->setValue(lstyle.value(ST_SlurMidWidth).toSpatium().val());
      slurDottedLineWidth->setValue(lstyle.value(ST_SlurDottedWidth).toSpatium().val());
      slurBow->setValue(lstyle.value(ST_SlurBow).toSpatium().val());
      musicalSymbolFont->setCurrentIndex(lstyle.value(ST_MusicalSymbolFont).toString() == "Emmentaler" ? 0 : 1);

      showHeader->setChecked(lstyle.value(ST_showHeader).toBool());
      headerStyled->setChecked(lstyle.value(ST_headerStyled).toBool());
      showHeaderFirstPage->setChecked(lstyle.value(ST_headerFirstPage).toBool());
      headerOddEven->setChecked(lstyle.value(ST_headerOddEven).toBool());
      if (headerStyled->isChecked()) {
            evenHeaderL->setPlainText(lstyle.value(ST_evenHeaderL).toString());
            evenHeaderC->setPlainText(lstyle.value(ST_evenHeaderC).toString());
            evenHeaderR->setPlainText(lstyle.value(ST_evenHeaderR).toString());
            oddHeaderL->setPlainText(lstyle.value(ST_oddHeaderL).toString());
            oddHeaderC->setPlainText(lstyle.value(ST_oddHeaderC).toString());
            oddHeaderR->setPlainText(lstyle.value(ST_oddHeaderR).toString());
            }
      else {
            evenHeaderL->setHtml(lstyle.value(ST_evenHeaderL).toString());
            evenHeaderC->setHtml(lstyle.value(ST_evenHeaderC).toString());
            evenHeaderR->setHtml(lstyle.value(ST_evenHeaderR).toString());
            oddHeaderL->setHtml(lstyle.value(ST_oddHeaderL).toString());
            oddHeaderC->setHtml(lstyle.value(ST_oddHeaderC).toString());
            oddHeaderR->setHtml(lstyle.value(ST_oddHeaderR).toString());
            }

      showFooter->setChecked(lstyle.value(ST_showFooter).toBool());
      footerStyled->setChecked(lstyle.value(ST_footerStyled).toBool());
      showFooterFirstPage->setChecked(lstyle.value(ST_footerFirstPage).toBool());
      footerOddEven->setChecked(lstyle.value(ST_footerOddEven).toBool());
      if (footerStyled->isChecked()) {
            evenFooterL->setPlainText(lstyle.value(ST_evenFooterL).toString());
            evenFooterC->setPlainText(lstyle.value(ST_evenFooterC).toString());
            evenFooterR->setPlainText(lstyle.value(ST_evenFooterR).toString());
            oddFooterL->setPlainText(lstyle.value(ST_oddFooterL).toString());
            oddFooterC->setPlainText(lstyle.value(ST_oddFooterC).toString());
            oddFooterR->setPlainText(lstyle.value(ST_oddFooterR).toString());
            }
      else {
            evenFooterL->setHtml(lstyle.value(ST_evenFooterL).toString());
            evenFooterC->setHtml(lstyle.value(ST_evenFooterC).toString());
            evenFooterR->setHtml(lstyle.value(ST_evenFooterR).toString());
            oddFooterL->setHtml(lstyle.value(ST_oddFooterL).toString());
            oddFooterC->setHtml(lstyle.value(ST_oddFooterC).toString());
            oddFooterR->setHtml(lstyle.value(ST_oddFooterR).toString());
            }

      voltaY->setValue(lstyle.value(ST_voltaY).toSpatium().val());
      voltaHook->setValue(lstyle.value(ST_voltaHook).toSpatium().val());
      voltaLineWidth->setValue(lstyle.value(ST_voltaLineWidth).toSpatium().val());

      ottavaY->setValue(lstyle.value(ST_ottavaY).toSpatium().val());
      ottavaHook->setValue(lstyle.value(ST_ottavaHook).toSpatium().val());
      ottavaLineWidth->setValue(lstyle.value(ST_ottavaLineWidth).toSpatium().val());

      clefTab1->setChecked(lstyle.value(ST_tabClef).toInt() == CLEF_TAB);
      clefTab2->setChecked(lstyle.value(ST_tabClef).toInt() == CLEF_TAB2);
      }

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
      {
      QString fn = mscore->getChordStyleFilename(true);
      if (fn.isEmpty())
            return;
      chordDescriptionFile->setText(fn);
      }

//---------------------------------------------------------
//   editTextClicked
//---------------------------------------------------------

void EditStyle::editTextClicked(int id)
      {
      QTextEdit* e = 0;
      switch (id) {
            case  0:  e = evenHeaderL; break;
            case  1:  e = evenHeaderC; break;
            case  2:  e = evenHeaderR; break;
            case  3:  e = oddHeaderL;  break;
            case  4:  e = oddHeaderC;  break;
            case  5:  e = oddHeaderR;  break;

            case  6:  e = evenFooterL; break;
            case  7:  e = evenFooterC; break;
            case  8:  e = evenFooterR; break;
            case  9:  e = oddFooterL;  break;
            case 10:  e = oddFooterC;  break;
            case 11:  e = oddFooterR;  break;
            }
      if (e == 0)
            return;
      bool styled = id < 6 ? headerStyled->isChecked() : footerStyled->isChecked();

      if (styled)
            e->setPlainText(editPlainText(e->toPlainText()));
      else
            e->setHtml(editHtml(e->toHtml()));
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int row)
      {
      pageList->setCurrentRow(row);
      }

