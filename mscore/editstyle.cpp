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
      cs = s;

      const char* styles[] = {
            QT_TRANSLATE_NOOP("EditStyleBase", "Continuous"),
            QT_TRANSLATE_NOOP("EditStyleBase", "Dashed"),
            QT_TRANSLATE_NOOP("EditStyleBase", "Dotted"),
            QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dotted"),
            QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dot-dotted")
            };
      int data = 1;
      voltaLineStyle->clear();
      ottavaLineStyle->clear();
      pedalLineStyle->clear();
      for (const char* p : styles) {
            QString trs = qApp->translate("EditStyleBase", p);
            voltaLineStyle->addItem(trs, data);
            ottavaLineStyle->addItem(trs, data);
            pedalLineStyle->addItem(trs, data);
            ++data;
            }

      styleWidgets = {
      { StyleIdx::voltaLineStyle,         false, voltaLineStyle,           resetVoltaLineStyle },
      { StyleIdx::ottavaLineStyle,        false, ottavaLineStyle,          resetOttavaLineStyle },
      { StyleIdx::pedalLineStyle,         false, pedalLineStyle,           resetPedalLineStyle },

      { StyleIdx::staffUpperBorder,        false, staffUpperBorder,        0 },
      { StyleIdx::staffLowerBorder,        false, staffLowerBorder,        0 },
      { StyleIdx::staffDistance,           false, staffDistance,           0 },
      { StyleIdx::akkoladeDistance,        false, akkoladeDistance,        0 },
      { StyleIdx::minSystemDistance,       false, minSystemDistance,       0 },
      { StyleIdx::maxSystemDistance,       false, maxSystemDistance,       0 },
      { StyleIdx::lyricsDistance,          false, lyricsDistance,          0 },
      { StyleIdx::lyricsMinBottomDistance, false, lyricsMinBottomDistance, 0 },
      { StyleIdx::systemFrameDistance,     false, systemFrameDistance,     0 },
      { StyleIdx::frameSystemDistance,     false, frameSystemDistance,     0 },
      { StyleIdx::minMeasureWidth,         false, minMeasureWidth_2,       resetMinMeasureWidth },
      { StyleIdx::measureSpacing,          false, measureSpacing,          resetMeasureSpacing },

      { StyleIdx::barWidth,                false, barWidth,                0 },
      { StyleIdx::endBarWidth,             false, endBarWidth,             0 },
      { StyleIdx::endBarDistance,          false, endBarDistance,          0 },
      { StyleIdx::doubleBarWidth,          false, doubleBarWidth,          0 },
      { StyleIdx::doubleBarDistance,       false, doubleBarDistance,       0 },
      { StyleIdx::barGraceDistance,        false, barGraceDistance,        resetBarGraceDistance },
      { StyleIdx::useStandardNoteNames,    false, useStandardNoteNames,    0 },
      { StyleIdx::useGermanNoteNames,      false, useGermanNoteNames,      0 },
      { StyleIdx::useFullGermanNoteNames,  false, useFullGermanNoteNames,  0 },
      { StyleIdx::useSolfeggioNoteNames,   false, useSolfeggioNoteNames,   0 },
      { StyleIdx::useFrenchNoteNames,      false, useFrenchNoteNames,      0 },
      { StyleIdx::automaticCapitalization, false, automaticCapitalization, 0 },

      { StyleIdx::lowerCaseMinorChords,    false, lowerCaseMinorChords,    0 },

      { StyleIdx::lowerCaseBassNotes,      false, lowerCaseBassNotes,      0 },
      { StyleIdx::allCapsNoteNames,        false, allCapsNoteNames,        0 },
      { StyleIdx::concertPitch,            false, concertPitch,            0 },
      { StyleIdx::createMultiMeasureRests, false, multiMeasureRests,       0 },
      { StyleIdx::minEmptyMeasures,        false, minEmptyMeasures,        0 },
      { StyleIdx::minMMRestWidth,          false, minMeasureWidth,         0 },
      { StyleIdx::hideEmptyStaves,         false, hideEmptyStaves,         0 },
      { StyleIdx::dontHideStavesInFirstSystem, false, dontHideStavesInFirstSystem,             0 },
      { StyleIdx::hideInstrumentNameIfOneInstrument, false, hideInstrumentNameIfOneInstrument, 0 },
      { StyleIdx::accidentalNoteDistance,  false, accidentalNoteDistance,  0 },
      { StyleIdx::accidentalDistance,      false, accidentalDistance,      0 },

      { StyleIdx::minNoteDistance,         false, minNoteDistance,         resetMinNoteDistance },
      { StyleIdx::barNoteDistance,         false, barNoteDistance,         resetBarNoteDistance },
      { StyleIdx::barAccidentalDistance,   false, barAccidentalDistance,   resetBarAccidentalDistance },
      { StyleIdx::multiMeasureRestMargin,  false, multiMeasureRestMargin,  resetMultiMeasureRestMargin },
      { StyleIdx::noteBarDistance,         false, noteBarDistance,         resetNoteBarDistance },
      { StyleIdx::clefLeftMargin,          false, clefLeftMargin,          resetClefLeftMargin },
      { StyleIdx::keysigLeftMargin,        false, keysigLeftMargin,        resetKeysigLeftMargin },
      { StyleIdx::timesigLeftMargin,       false, timesigLeftMargin,       resetTimesigLeftMargin },
      { StyleIdx::clefKeyRightMargin,      false, clefKeyRightMargin,      resetClefKeyRightMargin },
      { StyleIdx::clefKeyDistance,         false, clefKeyDistance,         resetClefKeyDistance },
      { StyleIdx::clefTimesigDistance,     false, clefTimesigDistance,     resetClefTimesigDistance },
      { StyleIdx::keyTimesigDistance,      false, keyTimesigDistance,      resetKeyTimesigDistance },


      { StyleIdx::clefBarlineDistance,     false, clefBarlineDistance,     resetClefBarlineDistance },
      { StyleIdx::staffLineWidth,          false, staffLineWidth,          resetStaffLineWidth },
      { StyleIdx::beamWidth,               false, beamWidth,               0 },
      { StyleIdx::beamMinLen,              false, beamMinLen,              0 },
      { StyleIdx::hairpinY,                false, hairpinY,                resetHairpinY },
      { StyleIdx::hairpinLineWidth,        false, hairpinLineWidth,        resetHairpinLineWidth },
      { StyleIdx::hairpinHeight,           false, hairpinHeight,           resetHairpinHeight },
      { StyleIdx::hairpinContHeight,       false, hairpinContinueHeight,   resetHairpinContinueHeight },
      { StyleIdx::dotNoteDistance,         false, noteDotDistance,         0 },
      { StyleIdx::dotDotDistance,          false, dotDotDistance,          0 },
      { StyleIdx::stemWidth,               false, stemWidth,               0 },
      { StyleIdx::ledgerLineWidth,         false, ledgerLineWidth,         0 },
      { StyleIdx::ledgerLineLength,        false, ledgerLineLength,        0 },
      { StyleIdx::shortStemProgression,    false, shortStemProgression,    0 },
      { StyleIdx::shortestStem,            false, shortestStem,            0 },
      { StyleIdx::ArpeggioNoteDistance,    false, arpeggioNoteDistance,    0 },
      { StyleIdx::ArpeggioLineWidth,       false, arpeggioLineWidth,       0 },
      { StyleIdx::ArpeggioHookLen,         false, arpeggioHookLen,         0 },
      { StyleIdx::SlurEndWidth,            false, slurEndLineWidth,        0 },
      { StyleIdx::SlurMidWidth,            false, slurMidLineWidth,        0 },
      { StyleIdx::SlurDottedWidth,         false, slurDottedLineWidth,     0 },
      { StyleIdx::MinTieLength,            false, minTieLength,            0 },
      { StyleIdx::bracketWidth,            false, bracketWidth,            0 },
      { StyleIdx::bracketDistance,         false, bracketDistance,         0 },
      { StyleIdx::akkoladeWidth,           false, akkoladeWidth,           0 },
      { StyleIdx::akkoladeBarDistance,     false, akkoladeBarDistance,     0 },
      { StyleIdx::dividerLeft,             false, dividerLeft,             0 },
      { StyleIdx::dividerLeftX,            false, dividerLeftX,            0 },
      { StyleIdx::dividerLeftY,            false, dividerLeftY,            0 },
      { StyleIdx::dividerRight,            false, dividerRight,            0 },
      { StyleIdx::dividerRightX,           false, dividerRightX,           0 },
      { StyleIdx::dividerRightY,           false, dividerRightY,           0 },
      { StyleIdx::propertyDistanceHead,    false, propertyDistanceHead,    0 },
      { StyleIdx::propertyDistanceStem,    false, propertyDistanceStem,    0 },
      { StyleIdx::propertyDistance,        false, propertyDistance,        0 },
      { StyleIdx::voltaY,                  false, voltaY,                  resetVoltaY },
      { StyleIdx::voltaHook,               false, voltaHook,               resetVoltaHook },
      { StyleIdx::voltaLineWidth,          false, voltaLineWidth,          resetVoltaLineWidth },
      { StyleIdx::ottavaY,                 false, ottavaY,                 resetOttavaY },
      { StyleIdx::ottavaHook,              false, ottavaHook,              resetOttavaHook },
      { StyleIdx::ottavaLineWidth,         false, ottavaLineWidth,         resetOttavaLineWidth },
      { StyleIdx::pedalY,                  false, pedalY,                  0 },
      { StyleIdx::pedalLineWidth,          false, pedalLineWidth,          0 },
      { StyleIdx::trillY,                  false, trillY,                  0 },
      { StyleIdx::harmonyY,                false, harmonyY,                0 },
      { StyleIdx::harmonyFretDist,         false, harmonyFretDist,         0 },
      { StyleIdx::minHarmonyDistance,      false, minHarmonyDistance,      0 },
      { StyleIdx::maxHarmonyBarDistance,   false, maxHarmonyBarDistance,   0 },
      { StyleIdx::tupletVHeadDistance,     false, tupletVHeadDistance,     0 },
      { StyleIdx::tupletVStemDistance,     false, tupletVStemDistance,     0 },
      { StyleIdx::tupletStemLeftDistance,  false, tupletStemLeftDistance,  0 },
      { StyleIdx::tupletStemRightDistance, false, tupletStemRightDistance, 0 },
      { StyleIdx::tupletNoteLeftDistance,  false, tupletNoteLeftDistance,  0 },
      { StyleIdx::tupletNoteRightDistance, false, tupletNoteRightDistance, 0 },
      { StyleIdx::tupletBracketWidth,      false, tupletBracketWidth,      0 },
      { StyleIdx::lyricsLineHeight,        true,  lyricsLineHeight,             0 },
      { StyleIdx::repeatBarTips,           false, showRepeatBarTips,            0 },
      { StyleIdx::startBarlineSingle,      false, showStartBarlineSingle,       0 },
      { StyleIdx::startBarlineMultiple,    false, showStartBarlineMultiple,     0 },
      { StyleIdx::dividerLeftSym,          false, dividerLeftSym,               0 },
      { StyleIdx::dividerRightSym,         false, dividerRightSym,              0 },
      { StyleIdx::showMeasureNumber,       false, showMeasureNumber,            0 },
      { StyleIdx::showMeasureNumberOne,    false, showFirstMeasureNumber,       0 },
      { StyleIdx::measureNumberInterval,   false, intervalMeasureNumber,        0 },
      { StyleIdx::measureNumberSystem,     false, showEverySystemMeasureNumber, 0 },
      { StyleIdx::measureNumberAllStaffs,  false, showAllStaffsMeasureNumber,   0 },
      { StyleIdx::beamDistance,            true,  beamDistance,                 0 },
      { StyleIdx::beamNoSlope,             false, beamNoSlope,                  0 },
      { StyleIdx::graceNoteMag,            true,  graceNoteSize,                0 },
      { StyleIdx::smallStaffMag,           true,  smallStaffSize,               0 },
      { StyleIdx::smallNoteMag,            true,  smallNoteSize,                0 },
      { StyleIdx::smallClefMag,            true,  smallClefSize,                0 },
      { StyleIdx::lyricsDashMinLength,     false, lyricsDashMinLength,          0 },
      { StyleIdx::lyricsDashMaxLength,     false, lyricsDashMaxLength,          0 },
      { StyleIdx::lyricsDashForce,         false, lyricsDashForce,              0 },
      { StyleIdx::lastSystemFillLimit,     true,  lastSystemFillThreshold,      0 },
      { StyleIdx::genClef,                 false, genClef,                      0 },
      { StyleIdx::genKeysig,               false, genKeysig,                    0 },
      { StyleIdx::genCourtesyTimesig,      false, genCourtesyTimesig,           0 },
      { StyleIdx::genCourtesyKeysig,       false, genCourtesyKeysig,            0 },
      { StyleIdx::genCourtesyClef,         false, genCourtesyClef,              0 },
      { StyleIdx::swingRatio,              false, swingBox,                     0 },
      { StyleIdx::chordsXmlFile,           false, chordsXmlFile,                0 },
      { StyleIdx::dotMag,                  true,  dotMag,                       0 },
      { StyleIdx::articulationMag,         true,  articulationMag,              0 },
      { StyleIdx::shortenStem,             false, shortenStem,                  0 },
      { StyleIdx::showHeader,              false, showHeader,                   0 },
      { StyleIdx::headerFirstPage,         false, showHeaderFirstPage,          0 },
      { StyleIdx::headerOddEven,           false, headerOddEven,                0 },
      { StyleIdx::evenHeaderL,             false, evenHeaderL,                  0 },
      { StyleIdx::evenHeaderC,             false, evenHeaderC,                  0 },
      { StyleIdx::evenHeaderR,             false, evenHeaderR,                  0 },
      { StyleIdx::oddHeaderL,              false, oddHeaderL,                   0 },
      { StyleIdx::oddHeaderC,              false, oddHeaderC,                   0 },
      { StyleIdx::oddHeaderR,              false, oddHeaderR,                   0 },
      { StyleIdx::showFooter,              false, showFooter,                   0 },
      { StyleIdx::footerFirstPage,         false, showFooterFirstPage,          0 },
      { StyleIdx::footerOddEven,           false, footerOddEven,                0 },
      { StyleIdx::evenFooterL,             false, evenFooterL,                  0 },
      { StyleIdx::evenFooterC,             false, evenFooterC,                  0 },
      { StyleIdx::evenFooterR,             false, evenFooterR,                  0 },
      { StyleIdx::oddFooterL,              false, oddFooterL,                   0 },
      { StyleIdx::oddFooterC,              false, oddFooterC,                   0 },
      { StyleIdx::oddFooterR,              false, oddFooterR,                   0 },

      { StyleIdx::ottavaNumbersOnly,       false, ottavaNumbersOnly,            resetOttavaNumbersOnly },
      { StyleIdx::capoPosition,            false, capoPosition,                 0 },
      { StyleIdx::fretNumMag,              true,  fretNumMag,                   0 },
      { StyleIdx::fretY,                   false, fretY,                        0 },
      { StyleIdx::tupletMaxSlope,          false, tupletMaxSlope,               0 },
      { StyleIdx::tupletOufOfStaff,        false, tupletOutOfStaff,             0 },
      { StyleIdx::barreLineWidth,          false, barreLineWidth,               0 },
      { StyleIdx::fretMag,                 false, fretMag,                      0 },
      { StyleIdx::scaleBarlines,           false, scaleBarlines,                0 },
      { StyleIdx::crossMeasureValues,      false, crossMeasureValues,           0 },
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

      dividerLeftSym->addItem( tr("System Divider"),            Sym::symNames[int(SymId::systemDivider)]);
      dividerLeftSym->addItem( tr("Long System Divider"),       Sym::symNames[int(SymId::systemDividerLong)]);
      dividerLeftSym->addItem( tr("Extra Long System Divider"), Sym::symNames[int(SymId::systemDividerExtraLong)]);
      dividerRightSym->addItem(tr("System Divider"),            Sym::symNames[int(SymId::systemDivider)]);
      dividerRightSym->addItem(tr("Long System Divider"),       Sym::symNames[int(SymId::systemDividerLong)]);
      dividerRightSym->addItem(tr("Extra Long System Divider"), Sym::symNames[int(SymId::systemDividerExtraLong)]);

      // figured bass init
      QList<QString> fbFontNames = FiguredBass::fontNames();
      for (const QString& family: fbFontNames)
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
            + tr("meta data tag, see below")
            + QString("</i></td></tr></table><p>")
            + tr("Available meta data tags and their current values:")
            + QString("</p><table>");
      // show all tags for current score/part, see also Score::init()
      if (cs->parentScore()) {
            QMapIterator<QString, QString> j(cs->parentScore()->metaTags());
            while (j.hasNext()) {
                  j.next();
                  toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(j.key()).arg(j.value());
                  }
            }
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
      connect(lyricsDashMinLength, SIGNAL(valueChanged(double)), SLOT(lyricsDashMinLengthValueChanged(double)));
      connect(lyricsDashMaxLength, SIGNAL(valueChanged(double)), SLOT(lyricsDashMaxLengthValueChanged(double)));

      QSignalMapper* mapper  = new QSignalMapper(this);     // reset style signals
      QSignalMapper* mapper2 = new QSignalMapper(this);     // value change signals

      for (const StyleWidget& sw : styleWidgets) {
            const char* type = MStyle::valueType(sw.idx);

            if (!strcmp("Direction", type)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  Direction::fillComboBox(cb);
                  }
            if (sw.reset) {
                  sw.reset->setIcon(*icons[int(Icons::reset_ICON)]);
                  sw.reset->setToolTip(tr("reset to default"));
                  connect(sw.reset, SIGNAL(clicked()), mapper, SLOT(map()));
                  mapper->setMapping(sw.reset, int(sw.idx));
                  }
            if (qobject_cast<QSpinBox*>(sw.widget))
                  connect(qobject_cast<QSpinBox*>(sw.widget), SIGNAL(valueChanged(int)), mapper2, SLOT(map()));
            else if (qobject_cast<QDoubleSpinBox*>(sw.widget))
                  connect(qobject_cast<QDoubleSpinBox*>(sw.widget), SIGNAL(valueChanged(double)), mapper2, SLOT(map()));
            else if (qobject_cast<QComboBox*>(sw.widget))
                  connect(qobject_cast<QComboBox*>(sw.widget), SIGNAL(currentIndexChanged(int)), mapper2, SLOT(map()));

            mapper2->setMapping(sw.widget, int(sw.idx));
            }

      connect(mapper, SIGNAL(mapped(int)), SLOT(resetStyleValue(int)));
      connect(mapper2, SIGNAL(mapped(int)), SLOT(valueChanged(int)));

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
                        cs->setLayoutAll();
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
//   getValue
//---------------------------------------------------------

void EditStyle::getValue(StyleIdx idx)
      {
      const StyleWidget& sw = styleWidget(idx);
      const char* type = MStyle::valueType(sw.idx);

      if (!strcmp("Ms::Spatium", type)) {
            QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(sw.widget);
            lstyle.set(sw.idx, QVariant(Spatium(sb->value() * (sw.showPercent ? 0.01 : 1.0))));
            }

      else if (!strcmp("double", type)) {
            if (sw.showPercent)
                  lstyle.set(sw.idx, qobject_cast<QSpinBox*>(sw.widget)->value() * 0.01);
            else
                  lstyle.set(sw.idx, qobject_cast<QDoubleSpinBox*>(sw.widget)->value());
            }
      else if (!strcmp("bool", type)) {
                  if (qobject_cast<QCheckBox*>(sw.widget))
                        lstyle.set(sw.idx, qobject_cast<QCheckBox*>(sw.widget)->isChecked());
                  else if (qobject_cast<QGroupBox*>(sw.widget))
                        lstyle.set(sw.idx, qobject_cast<QGroupBox*>(sw.widget)->isChecked());
                  else if (qobject_cast<QRadioButton*>(sw.widget))
                        lstyle.set(sw.idx, qobject_cast<QRadioButton*>(sw.widget)->isChecked());
                  else
                        qFatal("unhandled bool");
            }
      else if (!strcmp("int", type)) {
            if (qobject_cast<QComboBox*>(sw.widget)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  lstyle.set(sw.idx, cb->currentData().toInt());
                  }
            else if (qobject_cast<QSpinBox*>(sw.widget))
                  lstyle.set(sw.idx, qobject_cast<QSpinBox*>(sw.widget)->value() / (sw.showPercent ? 100 : 1));
            else
                  abort();
            }
      else if (!strcmp("QString", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb) {
                  printf("%s DIRECTION===widget %s %s\n", MStyle::valueName(sw.idx),
                  qPrintable(sw.widget->objectName()),
                  sw.widget->metaObject()->className());
                        ;
                  }
            else {
                  QTextEdit* te = qobject_cast<QTextEdit*>(sw.widget);
                  if (!te)
                        abort();
                  lstyle.set(sw.idx, te->toPlainText());
                  }
            }
      else if (!strcmp("Ms::Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb)
                  lstyle.set(sw.idx, Direction(cb->currentIndex()));
            else
                  abort();
            }
      else {
            qFatal("EditStyle::getValue: unhandled type <%s>", type);
            }
      }

//---------------------------------------------------------
//   getValues
//---------------------------------------------------------

void EditStyle::getValues()
      {
      for (const StyleWidget& sw : styleWidgets)
            getValue(sw.idx);

      //TODO: convert the rest:

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
      if (lstyle.value(StyleIdx::chordDescriptionFile).toString() != chordDescriptionFile->text()) {
            ChordList* cl = new ChordList();
            if (lstyle.value(StyleIdx::chordsXmlFile).toBool())
                  cl->read("chords.xml");
            cl->read(chordDescriptionFile->text());
            lstyle.setChordList(cl, customChords);
            lstyle.set(StyleIdx::chordDescriptionFile, chordDescriptionFile->text());
            }


      lstyle.set(StyleIdx::tabClef, int(clefTab1->isChecked() ? ClefType::TAB : ClefType::TAB2));


      int idx1 = musicalSymbolFont->itemData(musicalSymbolFont->currentIndex()).toInt();
      lstyle.set(StyleIdx::MusicalSymbolFont, ScoreFont::scoreFonts().at(idx1).name());

      QString tf = musicalTextFont->itemData(musicalTextFont->currentIndex()).toString();
      lstyle.set(StyleIdx::MusicalTextFont, tf);

      Text t(cs);
      t.setTextStyleType(TextStyleType::HEADER);
      t.setTextStyleType(TextStyleType::FOOTER);

      lstyle.set(StyleIdx::fretNumPos,              radioFretNumLeft->isChecked() ? 0 : 1);

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

      lstyle.set(StyleIdx::keySigNaturals,  radioKeySigNatNone->isChecked() ? int(KeySigNatural::NONE) :
                  (radioKeySigNatBefore->isChecked() ? int(KeySigNatural::BEFORE) : int(KeySigNatural::AFTER)) );

      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      for (const StyleWidget& sw : styleWidgets) {
            const char* type = MStyle::valueType(sw.idx);
            if (sw.reset)
                  sw.reset->setEnabled(!lstyle.isDefault(sw.idx));

            if (!strcmp("Ms::Spatium", type)) {
                  if (sw.showPercent)
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(int(lstyle.value(sw.idx).value<Spatium>().val() * 100.0));
                  else
                        qobject_cast<QDoubleSpinBox*>(sw.widget)->setValue(lstyle.value(sw.idx).value<Spatium>().val());
                  }
            else if (!strcmp("double", type)) {
                  if (sw.showPercent)
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(int(lstyle.value(sw.idx).toDouble() * 100.0));
                  else
                        qobject_cast<QDoubleSpinBox*>(sw.widget)->setValue(lstyle.value(sw.idx).toDouble());
                  }
            else if (!strcmp("bool", type)) {
                  if (qobject_cast<QCheckBox*>(sw.widget))
                        qobject_cast<QCheckBox*>(sw.widget)->setChecked(lstyle.value(sw.idx).toBool());
                  else if (qobject_cast<QGroupBox*>(sw.widget))
                        qobject_cast<QGroupBox*>(sw.widget)->setChecked(lstyle.value(sw.idx).toBool());
                  else if (qobject_cast<QRadioButton*>(sw.widget))
                        qobject_cast<QRadioButton*>(sw.widget)->setChecked(lstyle.value(sw.idx).toBool());
                  else
                        qFatal("unhandled bool");
                  }
            else if (!strcmp("int", type)) {
                  if (qobject_cast<QComboBox*>(sw.widget)) {
                        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                        cb->setCurrentIndex(cb->findData(lstyle.value(sw.idx)));
                        }
                  else if (qobject_cast<QSpinBox*>(sw.widget)) {
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(lstyle.value(sw.idx).toInt()
                           * (sw.showPercent ? 100 : 1));
                        }
                  else
                        abort();
                  }
            else if (!strcmp("QString", type)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  if (cb)
                        ;
                  else {
                        QTextEdit* te = qobject_cast<QTextEdit*>(sw.widget);
                        if (!te)
                              abort();
                        te->setPlainText(lstyle.value(sw.idx).toString());
                        }
                  }
            else if (!strcmp("Ms::Direction", type)) {
                        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                        if (cb)
                              cb->setCurrentIndex(int(lstyle.value(sw.idx).value<Direction>()));
                        else
                              abort();
                  }
            else {
                  qFatal("EditStyle::setValues: unhandled type <%s>", type);
                  }
            }

      //TODO: convert the rest:

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

      dontHideStavesInFirstSystem->setEnabled(hideEmptyStaves->isChecked());

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

      QString mfont(lstyle.value(StyleIdx::MusicalSymbolFont).toString());
      int idx = 0;
      for (const auto& i : ScoreFont::scoreFonts()) {
            if (i.name().toLower() == mfont.toLower()) {
                  musicalSymbolFont->setCurrentIndex(idx);
                  break;
                  }
            ++idx;
            }
      musicalTextFont->clear();
      // CAUTION: the second element, the itemdata, is a font family name!
      // It's also stored in score file as the musicalTextFont
      musicalTextFont->addItem("Bravura Text", "Bravura Text");
      musicalTextFont->addItem("Emmentaler Text", "MScore Text");
      musicalTextFont->addItem("Gonville Text", "Gootville Text");
      musicalTextFont->addItem("MuseJazz", "MuseJazz");
      QString tfont(lstyle.value(StyleIdx::MusicalTextFont).toString());
      idx = musicalTextFont->findData(tfont);
      musicalTextFont->setCurrentIndex(idx);

      toggleHeaderOddEven(lstyle.value(StyleIdx::headerOddEven).toBool());

      toggleFooterOddEven(lstyle.value(StyleIdx::footerOddEven).toBool());

      radioFretNumLeft->setChecked(lstyle.value(StyleIdx::fretNumPos).toInt() == 0);
      radioFretNumRight->setChecked(lstyle.value(StyleIdx::fretNumPos).toInt() == 1);

      clefTab1->setChecked(lstyle.value(StyleIdx::tabClef).toInt() == int(ClefType::TAB));
      clefTab2->setChecked(lstyle.value(StyleIdx::tabClef).toInt() == int(ClefType::TAB2));

      radioKeySigNatNone->setChecked  (lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::NONE));
      radioKeySigNatBefore->setChecked(lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::BEFORE));
      radioKeySigNatAfter->setChecked (lstyle.value(StyleIdx::keySigNaturals).toInt() == int(KeySigNatural::AFTER));
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
//   lyricsDashMin/MaxLengthValueChanged
//
//    Ensure lyricsDashMinLength <= lyricsDashMaxLength
//---------------------------------------------------------

void EditStyle::lyricsDashMaxLengthValueChanged(double val)
      {
      double otherVal = lyricsDashMinLength->value();
      if (otherVal > val)
            lyricsDashMaxLength->setValue(otherVal);
      }

void EditStyle::lyricsDashMinLengthValueChanged(double val)
      {
      double otherVal = lyricsDashMaxLength->value();
      if (otherVal < val)
            lyricsDashMaxLength->setValue(otherVal);
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int row)
      {
      pageList->setCurrentRow(row);
      }

//---------------------------------------------------------
//   styleWidget
//---------------------------------------------------------

const StyleWidget& EditStyle::styleWidget(StyleIdx idx) const
      {
      for (const StyleWidget& sw : styleWidgets) {
            if (sw.idx == idx)
                  return sw;
            }
      __builtin_unreachable();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditStyle::valueChanged(int i)
      {
      StyleIdx idx = (StyleIdx)i;
      getValue(idx);

      const StyleWidget& sw = styleWidget(idx);
      if (sw.reset)
            sw.reset->setEnabled(!lstyle.isDefault(idx));
      }

//---------------------------------------------------------
//   resetStyleValue
//---------------------------------------------------------

void EditStyle::resetStyleValue(int i)
      {
      StyleIdx id = (StyleIdx)i;
      lstyle.set(id, MScore::defaultStyle()->value(id));
      setValues();
      }
}

