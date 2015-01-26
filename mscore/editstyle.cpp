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
#include "libmscore/clef.h"
#include "libmscore/excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      cs     = s;

      styleWidgets = {
            { StyleIdx::staffUpperBorder,        staffUpperBorder       },
            { StyleIdx::staffUpperBorder,        staffLowerBorder       },
            { StyleIdx::staffDistance,           staffDistance          },
            { StyleIdx::akkoladeDistance,        akkoladeDistance       },
            { StyleIdx::minSystemDistance,       minSystemDistance      },
            { StyleIdx::maxSystemDistance,       maxSystemDistance      },
            { StyleIdx::lyricsDistance,          lyricsDistance         },
            { StyleIdx::lyricsMinBottomDistance, lyricsMinBottomDistance },
            { StyleIdx::systemFrameDistance,     systemFrameDistance    },
            { StyleIdx::frameSystemDistance,     frameSystemDistance    },
            { StyleIdx::minMeasureWidth,         minMeasureWidth_2      },
            { StyleIdx::barWidth,                barWidth               },
            { StyleIdx::endBarWidth,             endBarWidth            },
            { StyleIdx::endBarDistance,          endBarDistance         },
            { StyleIdx::doubleBarWidth,          doubleBarWidth         },
            { StyleIdx::doubleBarDistance,       doubleBarDistance      },
            };

      buttonApplyToAllParts = buttonBox->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
      buttonApplyToAllParts->setEnabled(cs->parentScore() != nullptr);

      lstyle = *s->style();
      setModal(true);

      const QIcon &editIcon = *icons[int(Icons::edit_ICON)];
      chordDescriptionFileButton->setIcon(editIcon);
      const QIcon &resetIcon = *icons[int(Icons::reset_ICON)];
      resetHairpinY->setIcon(resetIcon);
      resetHairpinLineWidth->setIcon(resetIcon);
      resetHairpinHeight->setIcon(resetIcon);
      resetHairpinContinueHeight->setIcon(resetIcon);
      resetVoltaY->setIcon(resetIcon);
      resetVoltaHook->setIcon(resetIcon);
      resetVoltaLineWidth->setIcon(resetIcon);
      resetVoltaLineStyle->setIcon(resetIcon);
      resetOttavaY->setIcon(resetIcon);
      resetOttavaHook->setIcon(resetIcon);
      resetOttavaLineWidth->setIcon(resetIcon);
      resetOttavaLineStyle->setIcon(resetIcon);
      resetOttavaNumbersOnly->setIcon(resetIcon);

      pageList->setCurrentRow(0);

      //articulationTable->verticalHeader()->setVisible(false); // can get disabled in ui file
      articulationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      QStringList headers;
      headers << tr("Symbol") << tr("Anchor");
      articulationTable->setHorizontalHeaderLabels(headers);
      articulationTable->setColumnWidth(0, 200);
      articulationTable->setColumnWidth(1, 180);
      articulationTable->setRowCount(int(ArticulationType::ARTICULATIONS));

      accidentalsGroup->setVisible(false); // disable, not yet implemented

      musicalSymbolFont->clear();
      int idx = 0;
      for (auto i : ScoreFont::scoreFonts()) {
            musicalSymbolFont->addItem(i.name(), idx);
            ++idx;
            }

      for (int i = 0; i < int(ArticulationType::ARTICULATIONS); ++i) {
            ArticulationInfo* ai = &Articulation::articulationList[i];

            QPixmap ct = cs->scoreFont()->sym2pixmap(ai->upSym, 0.9);
            QIcon icon(ct);
            QTableWidgetItem* item = new QTableWidgetItem(icon, qApp->translate("articulation", ai->description.toUtf8().constData()));

            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            articulationTable->setItem(i, 0, item);

            QComboBox* cb = new QComboBox();
            cb->addItem(tr("Above Staff"), int(ArticulationAnchor::TOP_STAFF));
            cb->addItem(tr("Below Staff"), int(ArticulationAnchor::BOTTOM_STAFF));
            cb->addItem(tr("Chord Automatic"), int(ArticulationAnchor::CHORD));
            cb->addItem(tr("Above Chord"), int(ArticulationAnchor::TOP_CHORD));
            cb->addItem(tr("Below Chord"), int(ArticulationAnchor::BOTTOM_CHORD));
            articulationTable->setCellWidget(i, 1, cb);
            }

      // figured bass init
      QList<QString> fbFontNames = FiguredBass::fontNames();
      foreach(const QString& family, fbFontNames)
            comboFBFont->addItem(family);
      comboFBFont->setCurrentIndex(0);
      connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

      setValues();

      // keep in sync with implementation in Page::replaceTextMacros (page.cpp)
      // jumping thru hoops here to make the job of translators easier, yet have a nice display
      QString toolTipHeaderFooter
            = QString("<html><head></head><body><p><b>")
            + tr("Special symbols in header/footer")
            + QString("</b></p>")
            + QString("<table><tr><td>$p</td><td>-</td><td><i>")
            + tr("page number, except on first page")
            + QString("</i></td></tr><tr><td>$N</td><td>-</td><td><i>")
            + tr("page number, if there is more than one page")
            + QString("</i></td></tr><tr><td>$P</td><td>-</td><td><i>")
            + tr("page number, on all pages")
            + QString("</i></td></tr><tr><td>$n</td><td>-</td><td><i>")
            + tr("number of pages")
            + QString("</i></td></tr><tr><td>$f</td><td>-</td><td><i>")
            + tr("file name")
            + QString("</i></td></tr><tr><td>$F</td><td>-</td><td><i>")
            + tr("file path+name")
            + QString("</i></td></tr><tr><td>$d</td><td>-</td><td><i>")
            + tr("current date")
            + QString("</i></td></tr><tr><td>$D</td><td>-</td><td><i>")
            + tr("creation date")
            + QString("</i></td></tr><tr><td>$m</td><td>-</td><td><i>")
            + tr("last modification time")
            + QString("</i></td></tr><tr><td>$M</td><td>-</td><td><i>")
            + tr("last modification date")
            + QString("</i></td></tr><tr><td>$C</td><td>-</td><td><i>")
            + tr("copyright, on first page only")
            + QString("</i></td></tr><tr><td>$c</td><td>-</td><td><i>")
            + tr("copyright, on all pages")
            + QString("</i></td></tr><tr><td>$$</td><td>-</td><td><i>")
            + tr("the $ sign itself")
            + QString("</i></td></tr><tr><td>$:tag:</td><td>-</td><td><i>")
            + tr("meta data tag")
            + QString("</i></td></tr></table><p>")
            + tr("Available tags and their current values:")
            + QString("</p><table>");
      // shown all tags for curent score, see also Score::init()
      QMapIterator<QString, QString> i(cs->metaTags());
      while (i.hasNext()) {
            i.next();
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(i.key()).arg(i.value());
            }
      toolTipHeaderFooter += QString("</table></body></html>");
      showHeader->setToolTip(toolTipHeaderFooter);
      showFooter->setToolTip(toolTipHeaderFooter);
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(headerOddEven, SIGNAL(toggled(bool)), SLOT(toggleHeaderOddEven(bool)));
      connect(footerOddEven, SIGNAL(toggled(bool)), SLOT(toggleFooterOddEven(bool)));
      connect(chordDescriptionFileButton, SIGNAL(clicked()), SLOT(selectChordDescriptionFile()));
      connect(chordsStandard, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));
      connect(chordsJazz, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));
      connect(chordsCustom, SIGNAL(toggled(bool)), SLOT(setChordStyle(bool)));
      connect(SwingOff, SIGNAL(toggled(bool)), SLOT(setSwingParams(bool)));
      connect(swingEighth, SIGNAL(toggled(bool)), SLOT(setSwingParams(bool)));
      connect(swingSixteenth, SIGNAL(toggled(bool)), SLOT(setSwingParams(bool)));
      connect(hideEmptyStaves, SIGNAL(clicked(bool)), dontHideStavesInFirstSystem, SLOT(setEnabled(bool)));

      QSignalMapper* mapper = new QSignalMapper(this);

#define CR(W, ID) connect(W, SIGNAL(clicked()), mapper, SLOT(map())); mapper->setMapping(W, int(ID));
      CR(resetVoltaY,                StyleIdx::voltaY);
      CR(resetVoltaHook,             StyleIdx::voltaHook);
      CR(resetVoltaLineWidth,        StyleIdx::voltaLineWidth);
      CR(resetVoltaLineStyle,        StyleIdx::voltaLineStyle);

      CR(resetOttavaY,               StyleIdx::ottavaY);
      CR(resetOttavaHook,            StyleIdx::ottavaHook);
      CR(resetOttavaLineWidth,       StyleIdx::ottavaLineWidth);
      CR(resetOttavaLineStyle,       StyleIdx::ottavaLineStyle);
      CR(resetOttavaNumbersOnly,     StyleIdx::ottavaNumbersOnly);

      CR(resetHairpinY,              StyleIdx::hairpinY);
      CR(resetHairpinLineWidth,      StyleIdx::hairpinLineWidth);
      CR(resetHairpinHeight,         StyleIdx::hairpinHeight);
      CR(resetHairpinContinueHeight, StyleIdx::hairpinContHeight);
#undef CR
      connect(mapper, SIGNAL(mapped(int)), SLOT(resetStyleValue(int)));
      resize(904, 577); // override designer values
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
            case QDialogButtonBox::Cancel:
                  if(cs->undo() && cs->undo()->current()) {
                        cs->undo()->current()->unwind();
                        cs->setLayoutAll(true);
                        }
                  done(0);
                  break;
            case QDialogButtonBox::NoButton:
            default:
                  if (b == buttonApplyToAllParts)
                        applyToAllParts();
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
      cs->deselectAll();
      cs->undo(new ChangeStyle(cs, lstyle));
      cs->update();
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void EditStyle::applyToAllParts()
      {
      getValues();
      for (Excerpt* e : cs->rootScore()->excerpts()) {
            e->partScore()->undo(new ChangeStyle(e->partScore(), lstyle));
            e->partScore()->update();
            }
      }

//---------------------------------------------------------
//   getValues
//---------------------------------------------------------

void EditStyle::getValues()
      {
      for (const StyleWidget sw : styleWidgets) {
            StyleValueType svt = MStyle::valueType(sw.idx);
            switch (svt) {
                  case StyleValueType::SPATIUM:
                        lstyle.set(sw.idx, Spatium(qobject_cast<QDoubleSpinBox*>(sw.widget)->value()));
                        break;
                  case StyleValueType::DOUBLE:
                  case StyleValueType::BOOL:
                  case StyleValueType::INT:
                  case StyleValueType::DIRECTION:
                  case StyleValueType::STRING:
                        break;
                  };
            }

      //TODO: convert the rest:

      lstyle.set(StyleIdx::lyricsLineHeight,        Spatium(lyricsLineHeight->value() * .01));

      lstyle.set(StyleIdx::repeatBarTips,           showRepeatBarTips->isChecked());
      lstyle.set(StyleIdx::startBarlineSingle,      showStartBarlineSingle->isChecked());
      lstyle.set(StyleIdx::startBarlineMultiple,    showStartBarlineMultiple->isChecked());

      lstyle.set(StyleIdx::measureSpacing,          measureSpacing->value());
      lstyle.set(StyleIdx::minNoteDistance,         Spatium(minNoteDistance->value()));
      lstyle.set(StyleIdx::barNoteDistance,         Spatium(barNoteDistance->value()));
      lstyle.set(StyleIdx::barAccidentalDistance,   Spatium(barAccidentalDistance->value()));
      lstyle.set(StyleIdx::multiMeasureRestMargin,  Spatium(multiMeasureRestMargin->value()));
      lstyle.set(StyleIdx::noteBarDistance,         Spatium(noteBarDistance->value()));
      lstyle.set(StyleIdx::showMeasureNumber,       showMeasureNumber->isChecked());
      lstyle.set(StyleIdx::showMeasureNumberOne,    showFirstMeasureNumber->isChecked());
      lstyle.set(StyleIdx::measureNumberInterval,   intervalMeasureNumber->value());
      lstyle.set(StyleIdx::measureNumberSystem,     showEverySystemMeasureNumber->isChecked());
      lstyle.set(StyleIdx::measureNumberAllStaffs,  showAllStaffsMeasureNumber->isChecked());
      lstyle.set(StyleIdx::clefLeftMargin,          Spatium(clefLeftMargin->value()));
      lstyle.set(StyleIdx::keysigLeftMargin,        Spatium(keysigLeftMargin->value()));
      lstyle.set(StyleIdx::timesigLeftMargin,       Spatium(timesigLeftMargin->value()));
      lstyle.set(StyleIdx::clefKeyRightMargin,      Spatium(clefKeyRightMargin->value()));
      lstyle.set(StyleIdx::clefBarlineDistance,     Spatium(clefBarlineDistance->value()));
      lstyle.set(StyleIdx::staffLineWidth,          Spatium(staffLineWidth->value()));
      lstyle.set(StyleIdx::beamWidth,               Spatium(beamWidth->value()));
      lstyle.set(StyleIdx::beamDistance,            beamDistance->value() / 100.0);
      lstyle.set(StyleIdx::beamMinLen,              Spatium(beamMinLen->value()));
      lstyle.set(StyleIdx::beamNoSlope,             beamNoSlope->isChecked());

      lstyle.set(StyleIdx::graceNoteMag,            graceNoteSize->value() / 100.0);
      lstyle.set(StyleIdx::smallStaffMag,           smallStaffSize->value() / 100.0);
      lstyle.set(StyleIdx::smallNoteMag,            smallNoteSize->value() / 100.0);
      lstyle.set(StyleIdx::smallClefMag,            smallClefSize->value() / 100.0);
      lstyle.set(StyleIdx::lastSystemFillLimit,     lastSystemFillThreshold->value() / 100.0);
      lstyle.set(StyleIdx::hairpinY,                Spatium(hairpinY->value()));
      lstyle.set(StyleIdx::hairpinLineWidth,        Spatium(hairpinLineWidth->value()));
      lstyle.set(StyleIdx::hairpinHeight,           Spatium(hairpinHeight->value()));
      lstyle.set(StyleIdx::hairpinContHeight,       Spatium(hairpinContinueHeight->value()));
      lstyle.set(StyleIdx::genClef,                 genClef->isChecked());
      lstyle.set(StyleIdx::genKeysig,               genKeysig->isChecked());
      lstyle.set(StyleIdx::genTimesig,              genTimesig->isChecked());
      lstyle.set(StyleIdx::genCourtesyTimesig,      genCourtesyTimesig->isChecked());
      lstyle.set(StyleIdx::genCourtesyKeysig,       genCourtesyKeysig->isChecked());
      lstyle.set(StyleIdx::genCourtesyClef,         genCourtesyClef->isChecked());
      lstyle.set(StyleIdx::swingRatio,              swingBox->value());
      if (swingEighth->isChecked())
            lstyle.set(StyleIdx::swingUnit, QString(TDuration(TDuration::DurationType::V_EIGHTH).name()));
      else if (swingSixteenth->isChecked())
            lstyle.set(StyleIdx::swingUnit, QString(TDuration(TDuration::DurationType::V_16TH).name()));
      else if (SwingOff->isChecked())
            lstyle.set(StyleIdx::swingUnit, QString(TDuration(TDuration::DurationType::V_ZERO).name()));
      bool customChords = false;
      if (chordsStandard->isChecked())
            lstyle.set(StyleIdx::chordStyle, QString("std"));
      else if (chordsJazz->isChecked())
            lstyle.set(StyleIdx::chordStyle, QString("jazz"));
      else {
            lstyle.set(StyleIdx::chordStyle, QString("custom"));
            customChords = true;
            }
      lstyle.set(StyleIdx::chordsXmlFile, chordsXmlFile->isChecked());
      if (lstyle.value(StyleIdx::chordDescriptionFile).toString() != chordDescriptionFile->text()) {
            ChordList* cl = new ChordList();
            if (lstyle.value(StyleIdx::chordsXmlFile).toBool())
                  cl->read("chords.xml");
            cl->read(chordDescriptionFile->text());
            lstyle.setChordList(cl, customChords);
            lstyle.set(StyleIdx::chordDescriptionFile, chordDescriptionFile->text());
            }

      lstyle.set(StyleIdx::useStandardNoteNames,    useStandardNoteNames->isChecked());
      lstyle.set(StyleIdx::useGermanNoteNames,      useGermanNoteNames->isChecked());
      lstyle.set(StyleIdx::useFullGermanNoteNames,  useFullGermanNoteNames->isChecked());
      lstyle.set(StyleIdx::useSolfeggioNoteNames,   useSolfeggioNoteNames->isChecked());
      lstyle.set(StyleIdx::useFrenchNoteNames,      useFrenchNoteNames->isChecked());
      lstyle.set(StyleIdx::automaticCapitalization, automaticCapitalization->isChecked());
      lstyle.set(StyleIdx::lowerCaseMinorChords,    lowerCaseMinorChords->isChecked());
      lstyle.set(StyleIdx::lowerCaseBassNotes,      lowerCaseBassNotes->isChecked());
      lstyle.set(StyleIdx::allCapsNoteNames,        allCapsNoteNames->isChecked());

      lstyle.set(StyleIdx::concertPitch,            concertPitch->isChecked());
      lstyle.set(StyleIdx::createMultiMeasureRests, multiMeasureRests->isChecked());
      lstyle.set(StyleIdx::minEmptyMeasures,        minEmptyMeasures->value());
      lstyle.set(StyleIdx::minMMRestWidth,          Spatium(minMeasureWidth->value()));
      lstyle.set(StyleIdx::hideEmptyStaves,         hideEmptyStaves->isChecked());
      lstyle.set(StyleIdx::dontHideStavesInFirstSystem, dontHideStavesInFirstSystem->isChecked());
      lstyle.set(StyleIdx::hideInstrumentNameIfOneInstrument, hideInstrumentNameIfOneInstrument->isChecked());

      lstyle.set(StyleIdx::accidentalNoteDistance,  Spatium(accidentalNoteDistance->value()));
      lstyle.set(StyleIdx::accidentalDistance,      Spatium(accidentalDistance->value()));
      lstyle.set(StyleIdx::dotMag,                  dotMag->value() / 100.0);
      lstyle.set(StyleIdx::dotNoteDistance,         Spatium(noteDotDistance->value()));
      lstyle.set(StyleIdx::dotDotDistance,          Spatium(dotDotDistance->value()));
      lstyle.set(StyleIdx::stemWidth,               Spatium(stemWidth->value()));
      lstyle.set(StyleIdx::ledgerLineWidth,         Spatium(ledgerLineWidth->value()));
      lstyle.set(StyleIdx::ledgerLineLength,        Spatium(ledgerLineLength->value()));

      lstyle.set(StyleIdx::bracketWidth,            Spatium(bracketWidth->value()));
      lstyle.set(StyleIdx::bracketDistance,         Spatium(bracketDistance->value()));
      lstyle.set(StyleIdx::akkoladeWidth,           Spatium(akkoladeWidth->value()));
      lstyle.set(StyleIdx::akkoladeBarDistance,     Spatium(akkoladeBarDistance->value()));

      lstyle.set(StyleIdx::propertyDistanceHead,    Spatium(propertyDistanceHead->value()));
      lstyle.set(StyleIdx::propertyDistanceStem,    Spatium(propertyDistanceStem->value()));
      lstyle.set(StyleIdx::propertyDistance,        Spatium(propertyDistance->value()));
      lstyle.set(StyleIdx::articulationMag,         articulationMag->value() / 100.0);

      lstyle.set(StyleIdx::shortenStem,             shortenStem->isChecked());
      lstyle.set(StyleIdx::shortStemProgression,    Spatium(shortStemProgression->value()));
      lstyle.set(StyleIdx::shortestStem,            Spatium(shortestStem->value()));

      lstyle.set(StyleIdx::ArpeggioNoteDistance,    Spatium(arpeggioNoteDistance->value()));
      lstyle.set(StyleIdx::ArpeggioLineWidth,       Spatium(arpeggioLineWidth->value()));
      lstyle.set(StyleIdx::ArpeggioHookLen,         Spatium(arpeggioHookLen->value()));

      lstyle.set(StyleIdx::FixMeasureNumbers,       fixNumberMeasures->value());
      lstyle.set(StyleIdx::FixMeasureWidth,         fixMeasureWidth->isChecked());

      lstyle.set(StyleIdx::SlurEndWidth,            Spatium(slurEndLineWidth->value()));
      lstyle.set(StyleIdx::SlurMidWidth,            Spatium(slurMidLineWidth->value()));
      lstyle.set(StyleIdx::SlurDottedWidth,         Spatium(slurDottedLineWidth->value()));
      lstyle.set(StyleIdx::MinTieLength,            Spatium(minTieLength->value()));

      int idx1 = musicalSymbolFont->itemData(musicalSymbolFont->currentIndex()).toInt();
      lstyle.set(StyleIdx::MusicalSymbolFont, ScoreFont::scoreFonts().at(idx1).name());

      QString tf = musicalTextFont->itemData(musicalTextFont->currentIndex()).toString();
      lstyle.set(StyleIdx::MusicalTextFont, tf);

      lstyle.set(StyleIdx::showHeader,      showHeader->isChecked());
      lstyle.set(StyleIdx::headerFirstPage, showHeaderFirstPage->isChecked());
      lstyle.set(StyleIdx::headerOddEven,   headerOddEven->isChecked());

      Text t(cs);
      t.setTextStyleType(TextStyleType::HEADER);

      lstyle.set(StyleIdx::evenHeaderL, evenHeaderL->toPlainText());
      lstyle.set(StyleIdx::evenHeaderC, evenHeaderC->toPlainText());
      lstyle.set(StyleIdx::evenHeaderR, evenHeaderR->toPlainText());
      lstyle.set(StyleIdx::oddHeaderL,  oddHeaderL->toPlainText());
      lstyle.set(StyleIdx::oddHeaderC,  oddHeaderC->toPlainText());
      lstyle.set(StyleIdx::oddHeaderR,  oddHeaderR->toPlainText());

      lstyle.set(StyleIdx::showFooter,      showFooter->isChecked());
      lstyle.set(StyleIdx::footerFirstPage, showFooterFirstPage->isChecked());
      lstyle.set(StyleIdx::footerOddEven,   footerOddEven->isChecked());

      t.setTextStyleType(TextStyleType::FOOTER);
      lstyle.set(StyleIdx::evenFooterL, evenFooterL->toPlainText());
      lstyle.set(StyleIdx::evenFooterC, evenFooterC->toPlainText());
      lstyle.set(StyleIdx::evenFooterR, evenFooterR->toPlainText());
      lstyle.set(StyleIdx::oddFooterL,  oddFooterL->toPlainText());
      lstyle.set(StyleIdx::oddFooterC,  oddFooterC->toPlainText());
      lstyle.set(StyleIdx::oddFooterR,  oddFooterR->toPlainText());

      // figured bass
      int         idx = comboFBFont->currentIndex();
      QString     family;
      if(FiguredBass::fontData(idx, &family, 0, 0, 0))
            lstyle.set(StyleIdx::figuredBassFontFamily, family);
      qreal size = doubleSpinFBSize->value();
      qreal vPos = doubleSpinFBVertPos->value();
      lstyle.set(StyleIdx::figuredBassFontSize,   size);
      lstyle.set(StyleIdx::figuredBassYOffset,    vPos);
      lstyle.set(StyleIdx::figuredBassLineHeight, ((double)spinFBLineHeight->value()) / 100.0);
      lstyle.set(StyleIdx::figuredBassAlignment,  radioFBTop->isChecked() ? 0 : 1);
      lstyle.set(StyleIdx::figuredBassStyle,      radioFBModern->isChecked() ? 0 : 1);
      // copy to text style data relevant to it (LineHeight and Style are not in text style);
      // offsetType is necessarily OFFSET_SPATIUM
      const TextStyle fbOld = lstyle.textStyle(TextStyleType::FIGURED_BASS);
      if (family != fbOld.family() || size != fbOld.size()
         || vPos != fbOld.offset().y() || fbOld.offsetType() != OffsetType::SPATIUM)
            {
            TextStyle fbNew(fbOld);
            fbNew.setFamily(family);
            fbNew.setSize(size);
            fbNew.setYoff(vPos);
            fbNew.setOffsetType(OffsetType::SPATIUM);
            lstyle.setTextStyle(fbNew);
            }

      for (int i = 0; i < int(ArticulationType::ARTICULATIONS); ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            lstyle.setArticulationAnchor(i, ArticulationAnchor(cb->itemData(cb->currentIndex()).toInt()));
            }

      lstyle.set(StyleIdx::voltaY,                  Spatium(voltaY->value()));
      lstyle.set(StyleIdx::voltaHook,               Spatium(voltaHook->value()));
      lstyle.set(StyleIdx::voltaLineWidth,          Spatium(voltaLineWidth->value()));
      lstyle.set(StyleIdx::voltaLineStyle,          voltaLineStyle->currentIndex() + 1);

      lstyle.set(StyleIdx::ottavaY,                 Spatium(ottavaY->value()));
      lstyle.set(StyleIdx::ottavaHook,              Spatium(ottavaHook->value()));
      lstyle.set(StyleIdx::ottavaLineWidth,         Spatium(ottavaLineWidth->value()));
      lstyle.set(StyleIdx::ottavaLineStyle,         ottavaLineStyle->currentIndex() + 1);
      lstyle.set(StyleIdx::ottavaNumbersOnly,       ottavaNumbersOnly->isChecked());

      lstyle.set(StyleIdx::pedalY,                  Spatium(pedalY->value()));
      lstyle.set(StyleIdx::pedalLineWidth,          Spatium(pedalLineWidth->value()));
      lstyle.set(StyleIdx::pedalLineStyle,          pedalLineStyle->currentIndex() + 1);
      lstyle.set(StyleIdx::trillY,                  Spatium(trillY->value()));
      lstyle.set(StyleIdx::harmonyY,                Spatium(harmonyY->value()));
      lstyle.set(StyleIdx::harmonyFretDist,         Spatium(harmonyFretDist->value()));
      lstyle.set(StyleIdx::minHarmonyDistance,      Spatium(minHarmonyDistance->value()));
      lstyle.set(StyleIdx::maxHarmonyBarDistance,   Spatium(maxHarmonyBarDistance->value()));

      lstyle.set(StyleIdx::capoPosition,            capoPosition->value());

      lstyle.set(StyleIdx::fretNumMag,              fretNumMag->value()*0.01);
      lstyle.set(StyleIdx::fretNumPos,              radioFretNumLeft->isChecked() ? 0 : 1);
      lstyle.set(StyleIdx::fretY,                   fretY->value());

      lstyle.set(StyleIdx::tabClef, int(clefTab1->isChecked() ? ClefType::TAB : ClefType::TAB2));

      lstyle.set(StyleIdx::crossMeasureValues,      crossMeasureValues->isChecked());
      lstyle.set(StyleIdx::keySigNaturals,          radioKeySigNatNone->isChecked() ? int(KeySigNatural::NONE) :
                  (radioKeySigNatBefore->isChecked() ? int(KeySigNatural::BEFORE) : int(KeySigNatural::AFTER)) );

      lstyle.set(StyleIdx::tupletMaxSlope,           tupletMaxSlope->value());
      lstyle.set(StyleIdx::tupletOufOfStaff,         tupletOutOfStaff->isChecked());
      lstyle.set(StyleIdx::tupletVHeadDistance,      Spatium(tupletVHeadDistance->value()));
      lstyle.set(StyleIdx::tupletVStemDistance,      Spatium(tupletVStemDistance->value()));
      lstyle.set(StyleIdx::tupletStemLeftDistance,   Spatium(tupletStemLeftDistance->value()));
      lstyle.set(StyleIdx::tupletStemRightDistance,  Spatium(tupletStemRightDistance->value()));
      lstyle.set(StyleIdx::tupletNoteLeftDistance,   Spatium(tupletNoteLeftDistance->value()));
      lstyle.set(StyleIdx::tupletNoteRightDistance,  Spatium(tupletNoteRightDistance->value()));

      lstyle.set(StyleIdx::barreLineWidth,           barreLineWidth->value());
      lstyle.set(StyleIdx::fretMag,                  fretMag->value());
      lstyle.set(StyleIdx::scaleBarlines,            scaleBarlines->isChecked());
      }

//---------------------------------------------------------
//   setHeaderText
//---------------------------------------------------------

void EditStyle::setHeaderText(StyleIdx idx, QTextEdit* te)
      {
      QString s = lstyle.value(idx).toString();
//      s = Text::convertToHtml(s, cs->textStyle(TextStyleType::HEADER));
//      te->setHtml(s);
      te->setPlainText(s);
      }

//---------------------------------------------------------
//   setFooterText
//---------------------------------------------------------

void EditStyle::setFooterText(StyleIdx idx, QTextEdit* te)
      {
      QString s = lstyle.value(idx).toString();
//      s = Text::convertToHtml(s, cs->textStyle(TextStyleType::FOOTER));
//      te->setHtml(s);
      te->setPlainText(s);
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      for (const StyleWidget sw : styleWidgets) {
            StyleValueType svt = MStyle::valueType(sw.idx);
            switch (svt) {
                  case StyleValueType::SPATIUM:
                        qobject_cast<QDoubleSpinBox*>(sw.widget)->setValue(lstyle.value(sw.idx).toDouble());
                        break;
                  case StyleValueType::DOUBLE:
                  case StyleValueType::BOOL:
                  case StyleValueType::INT:
                  case StyleValueType::DIRECTION:
                  case StyleValueType::STRING:
                        break;
                  };
            }

      //TODO: convert the rest:

      lyricsLineHeight->setValue(lstyle.value(StyleIdx::lyricsLineHeight).toDouble() * 100.0);

      showRepeatBarTips->setChecked(lstyle.value(StyleIdx::repeatBarTips).toBool());
      showStartBarlineSingle->setChecked(lstyle.value(StyleIdx::startBarlineSingle).toBool());
      showStartBarlineMultiple->setChecked(lstyle.value(StyleIdx::startBarlineMultiple).toBool());

      measureSpacing->setValue(lstyle.value(StyleIdx::measureSpacing).toDouble());
      minNoteDistance->setValue(lstyle.value(StyleIdx::minNoteDistance).toDouble());
      barNoteDistance->setValue(lstyle.value(StyleIdx::barNoteDistance).toDouble());
      barAccidentalDistance->setValue(lstyle.value(StyleIdx::barAccidentalDistance).toDouble());
      multiMeasureRestMargin->setValue(lstyle.value(StyleIdx::multiMeasureRestMargin).toDouble());
      noteBarDistance->setValue(lstyle.value(StyleIdx::noteBarDistance).toDouble());

      showMeasureNumber->setChecked(lstyle.value(StyleIdx::showMeasureNumber).toBool());
      showFirstMeasureNumber->setChecked(lstyle.value(StyleIdx::showMeasureNumberOne).toBool());
      intervalMeasureNumber->setValue(lstyle.value(StyleIdx::measureNumberInterval).toInt());
      showIntervalMeasureNumber->setChecked(!lstyle.value(StyleIdx::measureNumberSystem).toBool());
      showAllStaffsMeasureNumber->setChecked(lstyle.value(StyleIdx::measureNumberAllStaffs).toBool());
      showEverySystemMeasureNumber->setChecked(lstyle.value(StyleIdx::measureNumberSystem).toBool());

      clefLeftMargin->setValue(lstyle.value(StyleIdx::clefLeftMargin).toDouble());
      keysigLeftMargin->setValue(lstyle.value(StyleIdx::keysigLeftMargin).toDouble());
      timesigLeftMargin->setValue(lstyle.value(StyleIdx::timesigLeftMargin).toDouble());
      clefKeyRightMargin->setValue(lstyle.value(StyleIdx::clefKeyRightMargin).toDouble());
      clefBarlineDistance->setValue(lstyle.value(StyleIdx::clefBarlineDistance).toDouble());
      staffLineWidth->setValue(lstyle.value(StyleIdx::staffLineWidth).toDouble());

      beamWidth->setValue(lstyle.value(StyleIdx::beamWidth).toDouble());
      beamDistance->setValue(lstyle.value(StyleIdx::beamDistance).toDouble() * 100.0);
      beamMinLen->setValue(lstyle.value(StyleIdx::beamMinLen).toDouble());
      beamNoSlope->setChecked(lstyle.value(StyleIdx::beamNoSlope).toBool());

      graceNoteSize->setValue(lstyle.value(StyleIdx::graceNoteMag).toDouble() * 100.0);
      smallStaffSize->setValue(lstyle.value(StyleIdx::smallStaffMag).toDouble() * 100.0);
      smallNoteSize->setValue(lstyle.value(StyleIdx::smallNoteMag).toDouble() * 100.0);
      smallClefSize->setValue(lstyle.value(StyleIdx::smallClefMag).toDouble() * 100.0);
      lastSystemFillThreshold->setValue(lstyle.value(StyleIdx::lastSystemFillLimit).toDouble() * 100.0);

      hairpinY->setValue(lstyle.value(StyleIdx::hairpinY).toDouble());
      hairpinLineWidth->setValue(lstyle.value(StyleIdx::hairpinLineWidth).toDouble());
      hairpinHeight->setValue(lstyle.value(StyleIdx::hairpinHeight).toDouble());
      hairpinContinueHeight->setValue(lstyle.value(StyleIdx::hairpinContHeight).toDouble());

      genClef->setChecked(lstyle.value(StyleIdx::genClef).toBool());
      genKeysig->setChecked(lstyle.value(StyleIdx::genKeysig).toBool());
      genTimesig->setChecked(lstyle.value(StyleIdx::genTimesig).toBool());
      genCourtesyTimesig->setChecked(lstyle.value(StyleIdx::genCourtesyTimesig).toBool());
      genCourtesyKeysig->setChecked(lstyle.value(StyleIdx::genCourtesyKeysig).toBool());
      genCourtesyClef->setChecked(lstyle.value(StyleIdx::genCourtesyClef).toBool());
      swingBox->setValue(lstyle.value(StyleIdx::swingRatio).toInt());
      QString unit(lstyle.value(StyleIdx::swingUnit).toString());
      if (unit == TDuration(TDuration::DurationType::V_EIGHTH).name()) {
            swingEighth->setChecked(true);
            swingBox->setEnabled(true);
            }
      else if (unit == TDuration(TDuration::DurationType::V_16TH).name()) {
            swingSixteenth->setChecked(true);
            swingBox->setEnabled(true);
            }
      else if (unit == TDuration(TDuration::DurationType::V_ZERO).name()) {
            SwingOff->setChecked(true);
            swingBox->setEnabled(false);
      }
      QString s(lstyle.value(StyleIdx::chordDescriptionFile).toString());
      chordDescriptionFile->setText(s);
      chordsXmlFile->setChecked(lstyle.value(StyleIdx::chordsXmlFile).toBool());
      QString cstyle(lstyle.value(StyleIdx::chordStyle).toString());
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
      useStandardNoteNames->setChecked(lstyle.value(StyleIdx::useStandardNoteNames).toBool());
      useGermanNoteNames->setChecked(lstyle.value(StyleIdx::useGermanNoteNames).toBool());
      useFullGermanNoteNames->setChecked(lstyle.value(StyleIdx::useFullGermanNoteNames).toBool());
      useSolfeggioNoteNames->setChecked(lstyle.value(StyleIdx::useSolfeggioNoteNames).toBool());
      useFrenchNoteNames->setChecked(lstyle.value(StyleIdx::useFrenchNoteNames).toBool());
      automaticCapitalization->setChecked(lstyle.value(StyleIdx::automaticCapitalization).toBool());
      lowerCaseMinorChords->setChecked(lstyle.value(StyleIdx::lowerCaseMinorChords).toBool());
      lowerCaseBassNotes->setChecked(lstyle.value(StyleIdx::lowerCaseBassNotes).toBool());
      allCapsNoteNames->setChecked(lstyle.value(StyleIdx::allCapsNoteNames).toBool());
      concertPitch->setChecked(lstyle.value(StyleIdx::concertPitch).toBool());

      multiMeasureRests->setChecked(lstyle.value(StyleIdx::createMultiMeasureRests).toBool());
      minEmptyMeasures->setValue(lstyle.value(StyleIdx::minEmptyMeasures).toInt());
      minMeasureWidth->setValue(lstyle.value(StyleIdx::minMMRestWidth).toDouble());
      hideEmptyStaves->setChecked(lstyle.value(StyleIdx::hideEmptyStaves).toBool());
      dontHideStavesInFirstSystem->setChecked(lstyle.value(StyleIdx::dontHideStavesInFirstSystem).toBool());
      dontHideStavesInFirstSystem->setEnabled(hideEmptyStaves->isChecked());
      hideInstrumentNameIfOneInstrument->setChecked(lstyle.value(StyleIdx::hideInstrumentNameIfOneInstrument).toBool());

      accidentalNoteDistance->setValue(lstyle.value(StyleIdx::accidentalNoteDistance).toDouble());
      accidentalDistance->setValue(lstyle.value(StyleIdx::accidentalDistance).toDouble());
      dotMag->setValue(lstyle.value(StyleIdx::dotMag).toDouble() * 100.0);
      noteDotDistance->setValue(lstyle.value(StyleIdx::dotNoteDistance).toDouble());
      dotDotDistance->setValue(lstyle.value(StyleIdx::dotDotDistance).toDouble());
      stemWidth->setValue(lstyle.value(StyleIdx::stemWidth).toDouble());
      ledgerLineWidth->setValue(lstyle.value(StyleIdx::ledgerLineWidth).toDouble());
      ledgerLineLength->setValue(lstyle.value(StyleIdx::ledgerLineLength).toDouble());

      bracketWidth->setValue(lstyle.value(StyleIdx::bracketWidth).toDouble());
      bracketDistance->setValue(lstyle.value(StyleIdx::bracketDistance).toDouble());
      akkoladeWidth->setValue(lstyle.value(StyleIdx::akkoladeWidth).toDouble());
      akkoladeBarDistance->setValue(lstyle.value(StyleIdx::akkoladeBarDistance).toDouble());

      propertyDistanceHead->setValue(lstyle.value(StyleIdx::propertyDistanceHead).toDouble());
      propertyDistanceStem->setValue(lstyle.value(StyleIdx::propertyDistanceStem).toDouble());
      propertyDistance->setValue(lstyle.value(StyleIdx::propertyDistance).toDouble());
      articulationMag->setValue(lstyle.value(StyleIdx::articulationMag).toDouble() * 100.0);

      shortenStem->setChecked(lstyle.value(StyleIdx::shortenStem).toBool());
      shortStemProgression->setValue(lstyle.value(StyleIdx::shortStemProgression).toDouble());
      shortestStem->setValue(lstyle.value(StyleIdx::shortestStem).toDouble());
      arpeggioNoteDistance->setValue(lstyle.value(StyleIdx::ArpeggioNoteDistance).toDouble());
      arpeggioLineWidth->setValue(lstyle.value(StyleIdx::ArpeggioLineWidth).toDouble());
      arpeggioHookLen->setValue(lstyle.value(StyleIdx::ArpeggioHookLen).toDouble());

      // figured bass
      for(int i = 0; i < comboFBFont->count(); i++)
            if(comboFBFont->itemText(i) == lstyle.value(StyleIdx::figuredBassFontFamily).toString()) {
                  comboFBFont->setCurrentIndex(i);
                  break;
            }
      doubleSpinFBSize->setValue(lstyle.value(StyleIdx::figuredBassFontSize).toDouble());
      doubleSpinFBVertPos->setValue(lstyle.value(StyleIdx::figuredBassYOffset).toDouble());
      spinFBLineHeight->setValue(lstyle.value(StyleIdx::figuredBassLineHeight).toDouble() * 100.0);
      radioFBTop->setChecked(lstyle.value(StyleIdx::figuredBassAlignment).toInt() == 0);
      radioFBBottom->setChecked(lstyle.value(StyleIdx::figuredBassAlignment).toInt() == 1);
      radioFBModern->setChecked(lstyle.value(StyleIdx::figuredBassStyle).toInt() == 0);
      radioFBHistoric->setChecked(lstyle.value(StyleIdx::figuredBassStyle).toInt() == 1);

      for (int i = 0; i < int(ArticulationType::ARTICULATIONS); ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            if (cb == 0)
                  continue;
            ArticulationAnchor st  = lstyle.articulationAnchor(i);
            int idx = 0;
            switch (st) {
                  case ArticulationAnchor::TOP_STAFF:       idx = 0;    break;
                  case ArticulationAnchor::BOTTOM_STAFF:    idx = 1;    break;
                  case ArticulationAnchor::CHORD:           idx = 2;    break;
                  case ArticulationAnchor::TOP_CHORD:       idx = 3;    break;
                  case ArticulationAnchor::BOTTOM_CHORD:    idx = 4;    break;
                  }
            cb->setCurrentIndex(idx);
            }

      fixNumberMeasures->setValue(lstyle.value(StyleIdx::FixMeasureNumbers).toInt());
      fixMeasureWidth->setChecked(lstyle.value(StyleIdx::FixMeasureWidth).toBool());

      slurEndLineWidth->setValue(lstyle.value(StyleIdx::SlurEndWidth).toDouble());
      slurMidLineWidth->setValue(lstyle.value(StyleIdx::SlurMidWidth).toDouble());
      slurDottedLineWidth->setValue(lstyle.value(StyleIdx::SlurDottedWidth).toDouble());
      minTieLength->setValue(lstyle.value(StyleIdx::MinTieLength).toDouble());

      QString mfont(lstyle.value(StyleIdx::MusicalSymbolFont).toString());
      int idx = 0;
      for (auto i : ScoreFont::scoreFonts()) {
            if (i.name().toLower() == mfont.toLower()) {
                  musicalSymbolFont->setCurrentIndex(idx);
                  break;
                  }
            ++idx;
            }
      musicalTextFont->clear();
      musicalTextFont->addItem("Emmentaler Text", "MScore Text");
      musicalTextFont->addItem("Gonville Text", "Gootville Text");
      musicalTextFont->addItem("Bravura Text", "Bravura Text");
      musicalTextFont->addItem("MuseJazz", "MuseJazz");
      QString tfont(lstyle.value(StyleIdx::MusicalTextFont).toString());
      idx = musicalTextFont->findData(tfont);
      musicalTextFont->setCurrentIndex(idx);

      showHeader->setChecked(lstyle.value(StyleIdx::showHeader).toBool());
      showHeaderFirstPage->setChecked(lstyle.value(StyleIdx::headerFirstPage).toBool());
      headerOddEven->setChecked(lstyle.value(StyleIdx::headerOddEven).toBool());
      toggleHeaderOddEven(lstyle.value(StyleIdx::headerOddEven).toBool());

      setHeaderText(StyleIdx::evenHeaderL, evenHeaderL);
      setHeaderText(StyleIdx::evenHeaderC, evenHeaderC);
      setHeaderText(StyleIdx::evenHeaderR, evenHeaderR);
      setHeaderText(StyleIdx::oddHeaderL, oddHeaderL);
      setHeaderText(StyleIdx::oddHeaderC, oddHeaderC);
      setHeaderText(StyleIdx::oddHeaderR, oddHeaderR);

      showFooter->setChecked(lstyle.value(StyleIdx::showFooter).toBool());
      showFooterFirstPage->setChecked(lstyle.value(StyleIdx::footerFirstPage).toBool());
      footerOddEven->setChecked(lstyle.value(StyleIdx::footerOddEven).toBool());
      toggleFooterOddEven(lstyle.value(StyleIdx::footerOddEven).toBool());

      setFooterText(StyleIdx::evenFooterL, evenFooterL);
      setFooterText(StyleIdx::evenFooterC, evenFooterC);
      setFooterText(StyleIdx::evenFooterR, evenFooterR);
      setFooterText(StyleIdx::oddFooterL, oddFooterL);
      setFooterText(StyleIdx::oddFooterC, oddFooterC);
      setFooterText(StyleIdx::oddFooterR, oddFooterR);

      voltaY->setValue(lstyle.value(StyleIdx::voltaY).toDouble());
      voltaHook->setValue(lstyle.value(StyleIdx::voltaHook).toDouble());
      voltaLineWidth->setValue(lstyle.value(StyleIdx::voltaLineWidth).toDouble());
      voltaLineStyle->setCurrentIndex(lstyle.value(StyleIdx::voltaLineStyle).toInt()-1);

      ottavaY->setValue(lstyle.value(StyleIdx::ottavaY).toDouble());
      ottavaHook->setValue(lstyle.value(StyleIdx::ottavaHook).toDouble());
      ottavaLineWidth->setValue(lstyle.value(StyleIdx::ottavaLineWidth).toDouble());
      ottavaLineStyle->setCurrentIndex(lstyle.value(StyleIdx::ottavaLineStyle).toInt()-1);
      ottavaNumbersOnly->setChecked(lstyle.value(StyleIdx::ottavaNumbersOnly).toBool());

      trillY->setValue(lstyle.value(StyleIdx::trillY).toDouble());
      harmonyY->setValue(lstyle.value(StyleIdx::harmonyY).toDouble());
      harmonyFretDist->setValue(lstyle.value(StyleIdx::harmonyFretDist).toDouble());
      minHarmonyDistance->setValue(lstyle.value(StyleIdx::minHarmonyDistance).toDouble());
      maxHarmonyBarDistance->setValue(lstyle.value(StyleIdx::maxHarmonyBarDistance).toDouble());
      capoPosition->setValue(lstyle.value(StyleIdx::capoPosition).toInt());
      fretNumMag->setValue(lstyle.value(StyleIdx::fretNumMag).toDouble()*100.0);
      radioFretNumLeft->setChecked(lstyle.value(StyleIdx::fretNumPos).toInt() == 0);
      radioFretNumRight->setChecked(lstyle.value(StyleIdx::fretNumPos).toInt() == 1);
      fretY->setValue(lstyle.value(StyleIdx::fretY).toDouble());
      pedalY->setValue(lstyle.value(StyleIdx::pedalY).toDouble());
      pedalLineWidth->setValue(lstyle.value(StyleIdx::pedalLineWidth).toDouble());
      pedalLineStyle->setCurrentIndex(lstyle.value(StyleIdx::pedalLineStyle).toInt()-1);

      clefTab1->setChecked(lstyle.value(StyleIdx::tabClef).toInt() == int(ClefType::TAB));
      clefTab2->setChecked(lstyle.value(StyleIdx::tabClef).toInt() == int(ClefType::TAB2));

      crossMeasureValues->setChecked(lstyle.value(StyleIdx::crossMeasureValues).toBool());

      radioKeySigNatNone->setChecked  (lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::NONE));
      radioKeySigNatBefore->setChecked(lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::BEFORE));
      radioKeySigNatAfter->setChecked (lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::AFTER));

      tupletMaxSlope->setValue(lstyle.value(StyleIdx::tupletMaxSlope).toDouble());
      tupletOutOfStaff->setChecked(lstyle.value(StyleIdx::tupletOufOfStaff).toBool());
      tupletVHeadDistance->setValue(lstyle.value(StyleIdx::tupletVHeadDistance).toDouble());
      tupletVStemDistance->setValue(lstyle.value(StyleIdx::tupletVStemDistance).toDouble());
      tupletStemLeftDistance->setValue(lstyle.value(StyleIdx::tupletStemLeftDistance).toDouble());
      tupletStemRightDistance->setValue(lstyle.value(StyleIdx::tupletStemRightDistance).toDouble());
      tupletNoteLeftDistance->setValue(lstyle.value(StyleIdx::tupletNoteLeftDistance).toDouble());
      tupletNoteRightDistance->setValue(lstyle.value(StyleIdx::tupletNoteRightDistance).toDouble());

      barreLineWidth->setValue(lstyle.value(StyleIdx::barreLineWidth).toDouble());
      fretMag->setValue(lstyle.value(StyleIdx::fretMag).toDouble());
      scaleBarlines->setChecked(lstyle.value(StyleIdx::scaleBarlines).toBool());
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

void EditStyle::setSwingParams(bool checked)
      {
      if( !checked)
            return;
      if (SwingOff->isChecked()) {
            lstyle.set(StyleIdx::swingUnit, TDuration(TDuration::DurationType::V_ZERO).name());
            swingBox->setEnabled(false);
            }
      else if (swingEighth->isChecked()) {
            lstyle.set(StyleIdx::swingUnit, TDuration(TDuration::DurationType::V_EIGHTH).name());
            swingBox->setEnabled(true);
            }
      else if (swingSixteenth->isChecked()) {
            lstyle.set(StyleIdx::swingUnit, TDuration(TDuration::DurationType::V_16TH).name());
            swingBox->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   setChordStyle
//---------------------------------------------------------

void EditStyle::setChordStyle(bool checked)
      {
      if (!checked)
            return;
      if (chordsStandard->isChecked()) {
            lstyle.set(StyleIdx::chordStyle, QString("std"));
            chordDescriptionFile->setText("chords_std.xml");
            lstyle.set(StyleIdx::chordsXmlFile, false);
            chordsXmlFile->setChecked(false);
            chordDescriptionGroup->setEnabled(false);
            }
      else if (chordsJazz->isChecked()) {
            lstyle.set(StyleIdx::chordStyle, QString("jazz"));
            chordDescriptionFile->setText("chords_jazz.xml");
            lstyle.set(StyleIdx::chordsXmlFile, false);
            chordsXmlFile->setChecked(false);
            chordDescriptionGroup->setEnabled(false);
            }
      else {
            lstyle.set(StyleIdx::chordStyle, QString("custom"));
            chordDescriptionGroup->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   toggleHeaderOddEven
//---------------------------------------------------------

void EditStyle::toggleHeaderOddEven(bool checked)
      {
      if (!showHeader->isChecked())
            return;
      labelEvenHeader->setEnabled(checked);
      evenHeaderL->setEnabled(checked);
      evenHeaderC->setEnabled(checked);
      evenHeaderR->setEnabled(checked);
      static QString odd  = labelOddHeader->text();  // save on 1st round
      static QString even = labelEvenHeader->text(); // save on 1st round
      if (checked)
            labelOddHeader->setText(odd); // restore
      else
            labelOddHeader->setText(odd + "\n" + even); // replace
      return;
      }

//---------------------------------------------------------
//   toggleFooterOddEven
//---------------------------------------------------------

void EditStyle::toggleFooterOddEven(bool checked)
      {
      if (!showFooter->isChecked())
            return;
      labelEvenFooter->setEnabled(checked);
      evenFooterL->setEnabled(checked);
      evenFooterC->setEnabled(checked);
      evenFooterR->setEnabled(checked);
      static QString odd  = labelOddFooter->text();  // save on 1st round
      static QString even = labelEvenFooter->text(); // save on 1st round
      if (checked)
            labelOddFooter->setText(odd); // restore
      else
            labelOddFooter->setText(odd + "\n" + even); // replace
      return;
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int row)
      {
      pageList->setCurrentRow(row);
      }

//---------------------------------------------------------
//   resetStyleValue
//---------------------------------------------------------

void EditStyle::resetStyleValue(int i)
      {
      StyleIdx id = (StyleIdx)i;
      qDebug("Reset %d dirty: %d", i, lstyle.value(id) != MScore::defaultStyle()->value(id));
//      if (lstyle.value(id) != MScore::defaultStyle()->value(id)) {
            lstyle.set(id, MScore::defaultStyle()->value(id));
//            }
      setValues();
      }

}

