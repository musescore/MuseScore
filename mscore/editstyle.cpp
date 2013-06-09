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

namespace Ms {

extern QString iconPath, iconGroup;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
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

            QPixmap ct = sym2pixmap(&symbols[0][ai->upSym], 3.0);
            QIcon icon(ct);
            QTableWidgetItem* item = new QTableWidgetItem(icon, qApp->translate("articulation", qPrintable(ai->name)));

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
      connect(chordsStandard, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));
      connect(chordsJazz, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));
      connect(chordsCustom, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));

      connect(hideEmptyStaves, SIGNAL(clicked(bool)), dontHideStavesInFirstSystem, SLOT(setEnabled(bool)));

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
      qreal size, lineHeight;

      if (FiguredBass::fontData(index, 0, 0, &size, &lineHeight)) {
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
      cs->update();
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

      if (chordsStandard->isChecked())
            lstyle.set(ST_chordStyle, QString("std"));
      else if (chordsJazz->isChecked())
            lstyle.set(ST_chordStyle, QString("jazz"));
      else
            lstyle.set(ST_chordStyle, QString("custom"));
      lstyle.set(ST_chordsXmlFile, chordsXmlFile->isChecked());
      if (lstyle.valueSt(ST_chordDescriptionFile) != chordDescriptionFile->text()) {
            ChordList* cl = new ChordList();
            if (lstyle.valueB(ST_chordsXmlFile))
                  cl->read("chords.xml");
            cl->read(chordDescriptionFile->text());
            lstyle.setChordList(cl);
            lstyle.set(ST_chordDescriptionFile, chordDescriptionFile->text());
            }
      lstyle.set(ST_useGermanNoteNames,      useGermanNoteNames->isChecked());


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
      lstyle.set(ST_stemWidth,               Spatium(stemWidth->value()));
      lstyle.set(ST_ledgerLineWidth,         Spatium(ledgerLineWidth->value()));
      lstyle.set(ST_ledgerLineLength,        Spatium(ledgerLineLength->value()));

      lstyle.set(ST_bracketWidth,            Spatium(bracketWidth->value()));
      lstyle.set(ST_bracketDistance,         Spatium(bracketDistance->value()));
      lstyle.set(ST_akkoladeWidth,           Spatium(akkoladeWidth->value()));
      lstyle.set(ST_akkoladeBarDistance,     Spatium(akkoladeBarDistance->value()));

      lstyle.set(ST_propertyDistanceHead,    Spatium(propertyDistanceHead->value()));
      lstyle.set(ST_propertyDistanceStem,    Spatium(propertyDistanceStem->value()));
      lstyle.set(ST_propertyDistance,        Spatium(propertyDistance->value()));
      lstyle.set(ST_stemDir1,                voice1Up->isChecked() ? MScore::UP : MScore::DOWN);
      lstyle.set(ST_stemDir2,                voice2Up->isChecked() ? MScore::UP : MScore::DOWN);
      lstyle.set(ST_stemDir3,                voice3Up->isChecked() ? MScore::UP : MScore::DOWN);
      lstyle.set(ST_stemDir4,                voice4Up->isChecked() ? MScore::UP : MScore::DOWN);

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
      qreal vPos = doubleSpinFBVertPos->value();
      lstyle.set(ST_figuredBassFontSize,   size);
      lstyle.set(ST_figuredBassYOffset,    vPos);
      lstyle.set(ST_figuredBassLineHeight, ((double)spinFBLineHeight->value()) / 100.0);
      lstyle.set(ST_figuredBassAlignment,  radioFBTop->isChecked() ? 0 : 1);
      lstyle.set(ST_figuredBassStyle,      radioFBModern->isChecked() ? 0 : 1);
      // copy to text style data relevant to it (LineHeight and Style are not in text style);
      // offsetType is necessarily OFFSET_SPATIUM
      const TextStyle fbOld = lstyle.textStyle(TEXT_STYLE_FIGURED_BASS);
      if(family != fbOld.family() || size != fbOld.size()
                  || vPos != fbOld.offset().y() || fbOld.offsetType() != OFFSET_SPATIUM) {
            TextStyle fbNew(fbOld);
            fbNew.setFamily(family);
            fbNew.setSize(size);
            fbNew.setYoff(vPos);
            fbNew.setOffsetType(OFFSET_SPATIUM);
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

      lstyle.set(ST_pedalY,                  Spatium(pedalY->value()));
      lstyle.set(ST_trillY,                  Spatium(trillY->value()));
      lstyle.set(ST_harmonyY,                Spatium(harmonyY->value()));
      lstyle.set(ST_harmonyFretDist,         Spatium(harmonyFretDist->value()));
      lstyle.set(ST_minHarmonyDistance,      Spatium(minHarmonyDistance->value()));

      lstyle.set(ST_tabClef, clefTab1->isChecked() ? CLEF_TAB : CLEF_TAB2);

      lstyle.set(ST_crossMeasureValues,      crossMeasureValues->isChecked());
      lstyle.set(ST_keySigNaturals,          radioKeySigNatNone->isChecked() ? NAT_NONE :
                  (radioKeySigNatBefore->isChecked() ? NAT_BEFORE : NAT_AFTER) );
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      staffUpperBorder->setValue(lstyle.valueS(ST_staffUpperBorder).val());
      staffLowerBorder->setValue(lstyle.valueS(ST_staffLowerBorder).val());
      staffDistance->setValue(lstyle.valueS(ST_staffDistance).val());
      akkoladeDistance->setValue(lstyle.valueS(ST_akkoladeDistance).val());
      minSystemDistance->setValue(lstyle.valueS(ST_minSystemDistance).val());
      maxSystemDistance->setValue(lstyle.valueS(ST_maxSystemDistance).val());
      lyricsDistance->setValue(lstyle.valueS(ST_lyricsDistance).val());
      lyricsMinBottomDistance->setValue(lstyle.valueS(ST_lyricsMinBottomDistance).val());
      lyricsLineHeight->setValue(lstyle.value(ST_lyricsLineHeight).toDouble() * 100.0);
      systemFrameDistance->setValue(lstyle.valueS(ST_systemFrameDistance).val());
      frameSystemDistance->setValue(lstyle.valueS(ST_frameSystemDistance).val());
      minMeasureWidth_2->setValue(lstyle.valueS(ST_minMeasureWidth).val());

      barWidth->setValue(lstyle.valueS(ST_barWidth).val());
      endBarWidth->setValue(lstyle.valueS(ST_endBarWidth).val());
      endBarDistance->setValue(lstyle.valueS(ST_endBarDistance).val());
      doubleBarWidth->setValue(lstyle.valueS(ST_doubleBarWidth).val());
      doubleBarDistance->setValue(lstyle.valueS(ST_doubleBarDistance).val());

      showRepeatBarTips->setChecked(lstyle.valueB(ST_repeatBarTips));
      showStartBarlineSingle->setChecked(lstyle.valueB(ST_startBarlineSingle));
      showStartBarlineMultiple->setChecked(lstyle.valueB(ST_startBarlineMultiple));

      measureSpacing->setValue(lstyle.value(ST_measureSpacing).toDouble());
      minNoteDistance->setValue(lstyle.valueS(ST_minNoteDistance).val());
      barNoteDistance->setValue(lstyle.valueS(ST_barNoteDistance).val());
      noteBarDistance->setValue(lstyle.valueS(ST_noteBarDistance).val());

      showMeasureNumber->setChecked(lstyle.valueB(ST_showMeasureNumber));
      showFirstMeasureNumber->setChecked(lstyle.valueB(ST_showMeasureNumberOne));
      intervalMeasureNumber->setValue(lstyle.valueI(ST_measureNumberInterval));
      showIntervalMeasureNumber->setChecked(!lstyle.valueB(ST_measureNumberSystem));
      showAllStaffsMeasureNumber->setChecked(lstyle.valueB(ST_measureNumberAllStaffs));
      showEverySystemMeasureNumber->setChecked(lstyle.valueB(ST_measureNumberSystem));

      clefLeftMargin->setValue(lstyle.valueS(ST_clefLeftMargin).val());
      keysigLeftMargin->setValue(lstyle.valueS(ST_keysigLeftMargin).val());
      timesigLeftMargin->setValue(lstyle.valueS(ST_timesigLeftMargin).val());
      clefKeyRightMargin->setValue(lstyle.valueS(ST_clefKeyRightMargin).val());
      clefBarlineDistance->setValue(lstyle.valueS(ST_clefBarlineDistance).val());
//      beginRepeatLeftMargin->setValue(lstyle.valueS(ST_beginRepeatLeftMargin).val());
      staffLineWidth->setValue(lstyle.valueS(ST_staffLineWidth).val());

      beamWidth->setValue(lstyle.valueS(ST_beamWidth).val());
      beamDistance->setValue(lstyle.value(ST_beamDistance).toDouble());
      beamMinLen->setValue(lstyle.valueS(ST_beamMinLen).val());
      beamMinSlope->setValue(lstyle.value(ST_beamMinSlope).toDouble());
      beamMaxSlope->setValue(lstyle.value(ST_beamMaxSlope).toDouble());

      graceNoteSize->setValue(lstyle.value(ST_graceNoteMag).toDouble() * 100.0);
      smallStaffSize->setValue(lstyle.value(ST_smallStaffMag).toDouble() * 100.0);
      smallNoteSize->setValue(lstyle.value(ST_smallNoteMag).toDouble() * 100.0);
      smallClefSize->setValue(lstyle.value(ST_smallClefMag).toDouble() * 100.0);
//      pageFillThreshold->setValue(lstyle.value(ST_pageFillLimit).toDouble() * 100.0);
      lastSystemFillThreshold->setValue(lstyle.value(ST_lastSystemFillLimit).toDouble() * 100.0);

      hairpinY->setValue(lstyle.valueS(ST_hairpinY).val());
      hairpinLineWidth->setValue(lstyle.valueS(ST_hairpinWidth).val());
      hairpinHeight->setValue(lstyle.valueS(ST_hairpinHeight).val());
      hairpinContinueHeight->setValue(lstyle.valueS(ST_hairpinContHeight).val());

      genClef->setChecked(lstyle.valueB(ST_genClef));
      genKeysig->setChecked(lstyle.valueB(ST_genKeysig));
      genTimesig->setChecked(lstyle.valueB(ST_genTimesig));
      genCourtesyTimesig->setChecked(lstyle.valueB(ST_genCourtesyTimesig));
      genCourtesyKeysig->setChecked(lstyle.valueB(ST_genCourtesyKeysig));
      genCourtesyClef->setChecked(lstyle.valueB(ST_genCourtesyClef));

      QString s(lstyle.valueSt(ST_chordDescriptionFile));
      chordDescriptionFile->setText(s);
      chordsXmlFile->setChecked(lstyle.valueB(ST_chordsXmlFile));
      QString cstyle(lstyle.valueSt(ST_chordStyle));
      if (cstyle == "std") {
            chordsStandard->setChecked(true);
            chordDescriptionGroup->setEnabled(false);
            }
      else if (cstyle == "jazz") {
            chordsJazz->setChecked(true);
            chordDescriptionGroup->setEnabled(false);
            }
      else {
            chordsCustom->setChecked(true);
            chordDescriptionGroup->setEnabled(true);
            }
      useGermanNoteNames->setChecked(lstyle.valueB(ST_useGermanNoteNames));
      concertPitch->setChecked(lstyle.valueB(ST_concertPitch));

      multiMeasureRests->setChecked(lstyle.valueB(ST_createMultiMeasureRests));
      minEmptyMeasures->setValue(lstyle.valueI(ST_minEmptyMeasures));
      minMeasureWidth->setValue(lstyle.valueS(ST_minMMRestWidth).val());
      hideEmptyStaves->setChecked(lstyle.valueB(ST_hideEmptyStaves));
      dontHideStavesInFirstSystem->setChecked(lstyle.valueB(ST_dontHideStavesInFirstSystem));
      dontHideStavesInFirstSystem->setEnabled(hideEmptyStaves->isChecked());

      accidentalNoteDistance->setValue(lstyle.valueS(ST_accidentalNoteDistance).val());
      accidentalDistance->setValue(lstyle.valueS(ST_accidentalDistance).val());
      noteDotDistance->setValue(lstyle.valueS(ST_dotNoteDistance).val());
      dotDotDistance->setValue(lstyle.valueS(ST_dotDotDistance).val());
      stemWidth->setValue(lstyle.valueS(ST_stemWidth).val());
      ledgerLineWidth->setValue(lstyle.valueS(ST_ledgerLineWidth).val());
      ledgerLineLength->setValue(lstyle.valueS(ST_ledgerLineLength).val());

      bracketWidth->setValue(lstyle.valueS(ST_bracketWidth).val());
      bracketDistance->setValue(lstyle.valueS(ST_bracketDistance).val());
      akkoladeWidth->setValue(lstyle.valueS(ST_akkoladeWidth).val());
      akkoladeBarDistance->setValue(lstyle.valueS(ST_akkoladeBarDistance).val());

      propertyDistanceHead->setValue(lstyle.valueS(ST_propertyDistanceHead).val());
      propertyDistanceStem->setValue(lstyle.valueS(ST_propertyDistanceStem).val());
      propertyDistance->setValue(lstyle.valueS(ST_propertyDistance).val());

      voice1Up->setChecked(lstyle.value(ST_stemDir1).toDirection() == MScore::UP);
      voice2Up->setChecked(lstyle.value(ST_stemDir2).toDirection() == MScore::UP);
      voice3Up->setChecked(lstyle.value(ST_stemDir3).toDirection() == MScore::UP);
      voice4Up->setChecked(lstyle.value(ST_stemDir4).toDirection() == MScore::UP);

      voice1Down->setChecked(lstyle.value(ST_stemDir1).toDirection() != MScore::UP);
      voice2Down->setChecked(lstyle.value(ST_stemDir2).toDirection() != MScore::UP);
      voice3Down->setChecked(lstyle.value(ST_stemDir3).toDirection() != MScore::UP);
      voice4Down->setChecked(lstyle.value(ST_stemDir4).toDirection() != MScore::UP);

      shortenStem->setChecked(lstyle.valueB(ST_shortenStem));
      shortStemProgression->setValue(lstyle.valueS(ST_shortStemProgression).val());
      shortestStem->setValue(lstyle.valueS(ST_shortestStem).val());
      arpeggioNoteDistance->setValue(lstyle.valueS(ST_ArpeggioNoteDistance).val());
      arpeggioLineWidth->setValue(lstyle.valueS(ST_ArpeggioLineWidth).val());
      arpeggioHookLen->setValue(lstyle.valueS(ST_ArpeggioHookLen).val());

      // figured bass
      for(int i = 0; i < comboFBFont->count(); i++)
            if(comboFBFont->itemText(i) == lstyle.valueSt(ST_figuredBassFontFamily)) {
                  comboFBFont->setCurrentIndex(i);
                  break;
            }
      doubleSpinFBSize->setValue(lstyle.value(ST_figuredBassFontSize).toDouble());
      doubleSpinFBVertPos->setValue(lstyle.value(ST_figuredBassYOffset).toDouble());
      spinFBLineHeight->setValue(lstyle.valueS(ST_figuredBassLineHeight).val() * 100.0);
      radioFBTop->setChecked(lstyle.valueI(ST_figuredBassAlignment) == 0);
      radioFBBottom->setChecked(lstyle.valueI(ST_figuredBassAlignment) == 1);
      radioFBModern->setChecked(lstyle.valueI(ST_figuredBassStyle) == 0);
      radioFBHistoric->setChecked(lstyle.valueI(ST_figuredBassStyle) == 1);

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
//      warnPitchRange->setChecked(lstyleB[ST_warnPitchRange]);

      fixNumberMeasures->setValue(lstyle.valueI(ST_FixMeasureNumbers));
      fixMeasureWidth->setChecked(lstyle.valueB(ST_FixMeasureWidth));

      slurEndLineWidth->setValue(lstyle.valueS(ST_SlurEndWidth).val());
      slurMidLineWidth->setValue(lstyle.valueS(ST_SlurMidWidth).val());
      slurDottedLineWidth->setValue(lstyle.valueS(ST_SlurDottedWidth).val());
      slurBow->setValue(lstyle.valueS(ST_SlurBow).val());
      musicalSymbolFont->setCurrentIndex(lstyle.valueSt(ST_MusicalSymbolFont) == "Emmentaler" ? 0 : 1);

      showHeader->setChecked(lstyle.valueB(ST_showHeader));
      headerStyled->setChecked(lstyle.valueB(ST_headerStyled));
      showHeaderFirstPage->setChecked(lstyle.valueB(ST_headerFirstPage));
      headerOddEven->setChecked(lstyle.valueB(ST_headerOddEven));
      if (headerStyled->isChecked()) {
            evenHeaderL->setPlainText(lstyle.valueSt(ST_evenHeaderL));
            evenHeaderC->setPlainText(lstyle.valueSt(ST_evenHeaderC));
            evenHeaderR->setPlainText(lstyle.valueSt(ST_evenHeaderR));
            oddHeaderL->setPlainText(lstyle.valueSt(ST_oddHeaderL));
            oddHeaderC->setPlainText(lstyle.valueSt(ST_oddHeaderC));
            oddHeaderR->setPlainText(lstyle.valueSt(ST_oddHeaderR));
            }
      else {
            evenHeaderL->setHtml(lstyle.valueSt(ST_evenHeaderL));
            evenHeaderC->setHtml(lstyle.valueSt(ST_evenHeaderC));
            evenHeaderR->setHtml(lstyle.valueSt(ST_evenHeaderR));
            oddHeaderL->setHtml(lstyle.valueSt(ST_oddHeaderL));
            oddHeaderC->setHtml(lstyle.valueSt(ST_oddHeaderC));
            oddHeaderR->setHtml(lstyle.valueSt(ST_oddHeaderR));
            }

      showFooter->setChecked(lstyle.valueB(ST_showFooter));
      footerStyled->setChecked(lstyle.valueB(ST_footerStyled));
      showFooterFirstPage->setChecked(lstyle.valueB(ST_footerFirstPage));
      footerOddEven->setChecked(lstyle.valueB(ST_footerOddEven));
      if (footerStyled->isChecked()) {
            evenFooterL->setPlainText(lstyle.valueSt(ST_evenFooterL));
            evenFooterC->setPlainText(lstyle.valueSt(ST_evenFooterC));
            evenFooterR->setPlainText(lstyle.valueSt(ST_evenFooterR));
            oddFooterL->setPlainText(lstyle.valueSt(ST_oddFooterL));
            oddFooterC->setPlainText(lstyle.valueSt(ST_oddFooterC));
            oddFooterR->setPlainText(lstyle.valueSt(ST_oddFooterR));
            }
      else {
            evenFooterL->setHtml(lstyle.valueSt(ST_evenFooterL));
            evenFooterC->setHtml(lstyle.valueSt(ST_evenFooterC));
            evenFooterR->setHtml(lstyle.valueSt(ST_evenFooterR));
            oddFooterL->setHtml(lstyle.valueSt(ST_oddFooterL));
            oddFooterC->setHtml(lstyle.valueSt(ST_oddFooterC));
            oddFooterR->setHtml(lstyle.valueSt(ST_oddFooterR));
            }

      voltaY->setValue(lstyle.valueS(ST_voltaY).val());
      voltaHook->setValue(lstyle.valueS(ST_voltaHook).val());
      voltaLineWidth->setValue(lstyle.valueS(ST_voltaLineWidth).val());

      ottavaY->setValue(lstyle.valueS(ST_ottavaY).val());
      ottavaHook->setValue(lstyle.valueS(ST_ottavaHook).val());
      ottavaLineWidth->setValue(lstyle.valueS(ST_ottavaLineWidth).val());

      trillY->setValue(lstyle.valueS(ST_trillY).val());
      harmonyY->setValue(lstyle.valueS(ST_harmonyY).val());
      harmonyFretDist->setValue(lstyle.valueS(ST_harmonyFretDist).val());
      minHarmonyDistance->setValue(lstyle.valueS(ST_minHarmonyDistance).val());
      pedalY->setValue(lstyle.valueS(ST_pedalY).val());

      clefTab1->setChecked(lstyle.valueI(ST_tabClef) == CLEF_TAB);
      clefTab2->setChecked(lstyle.valueI(ST_tabClef) == CLEF_TAB2);

      crossMeasureValues->setChecked(lstyle.valueB(ST_crossMeasureValues));
      radioKeySigNatNone->setChecked  (lstyle.valueB(ST_keySigNaturals) == NAT_NONE);
      radioKeySigNatBefore->setChecked(lstyle.valueB(ST_keySigNaturals) == NAT_BEFORE);
      radioKeySigNatAfter->setChecked (lstyle.valueB(ST_keySigNaturals) == NAT_AFTER);
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
//   setChordStyle
//---------------------------------------------------------

void EditStyle::setChordStyle(bool checked)
      {
      if (!checked)
            return;
      if (chordsStandard->isChecked()) {
            lstyle.set(ST_chordStyle, QString("std"));
            chordDescriptionFile->setText("chords_std.xml");
            lstyle.set(ST_chordsXmlFile, false);
            chordsXmlFile->setChecked(false);
            chordDescriptionGroup->setEnabled(false);
            }
      else if (chordsJazz->isChecked()) {
            lstyle.set(ST_chordStyle, QString("jazz"));
            chordDescriptionFile->setText("chords_jazz.xml");
            lstyle.set(ST_chordsXmlFile, false);
            chordsXmlFile->setChecked(false);
            chordDescriptionGroup->setEnabled(false);
            }
      else {
            lstyle.set(ST_chordStyle, QString("custom"));
            chordDescriptionGroup->setEnabled(true);
            }
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
            e->setPlainText(editPlainText(e->toPlainText(), tr("Edit Plain Text")));
      else
            e->setHtml(editHtml(e->toHtml(), tr("Edit HTML Text")));
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int row)
      {
      pageList->setCurrentRow(row);
      }
}

