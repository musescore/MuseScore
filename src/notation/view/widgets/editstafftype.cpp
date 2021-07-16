/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "editstafftype.h"
#include "libmscore/part.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"

#include "widgetstatestore.h"

#include "notationerrors.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

const char* g_groupNames[Ms::STAFF_GROUP_MAX] = {
    QT_TRANSLATE_NOOP("staff group header name", "STANDARD STAFF"),
    QT_TRANSLATE_NOOP("staff group header name", "PERCUSSION STAFF"),
    QT_TRANSLATE_NOOP("staff group header name", "TABLATURE STAFF")
};

//---------------------------------------------------------
//   noteHeadSchemes
//---------------------------------------------------------

Ms::NoteHead::Scheme noteHeadSchemes[] = {
    Ms::NoteHead::Scheme::HEAD_NORMAL,
    Ms::NoteHead::Scheme::HEAD_PITCHNAME,
    Ms::NoteHead::Scheme::HEAD_PITCHNAME_GERMAN,
    Ms::NoteHead::Scheme::HEAD_SOLFEGE,
    Ms::NoteHead::Scheme::HEAD_SOLFEGE_FIXED,
    Ms::NoteHead::Scheme::HEAD_SHAPE_NOTE_4,
    Ms::NoteHead::Scheme::HEAD_SHAPE_NOTE_7_AIKIN,
    Ms::NoteHead::Scheme::HEAD_SHAPE_NOTE_7_FUNK,
    Ms::NoteHead::Scheme::HEAD_SHAPE_NOTE_7_WALKER
};

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

EditStaffType::EditStaffType(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("EditStaffType");
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi(this);

    // tab page configuration
    QList<QString> fontNames = Ms::StaffType::fontNames(false);
    foreach (const QString& fn, fontNames) {   // fill fret font name combo
        fretFontName->addItem(fn);
    }
    fretFontName->setCurrentIndex(0);
    fontNames = Ms::StaffType::fontNames(true);
    foreach (const QString& fn, fontNames) {  // fill duration font name combo
        durFontName->addItem(fn);
    }
    durFontName->setCurrentIndex(0);

    for (auto i : noteHeadSchemes) {
        noteHeadScheme->addItem(Ms::NoteHead::scheme2userName(i), Ms::NoteHead::scheme2name(i));
    }

    // load a sample standard score in preview
    Ms::MasterScore* sc = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle();
    if (loadScore(sc, ":/view/resources/data/std_sample.mscx")) {
        standardPreview->setScore(sc);
    } else {
        Q_ASSERT_X(false, "EditStaffType::EditStaffType", "Error in opening sample standard file for preview");
    }

    // load a sample tabulature score in preview
    sc = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle();
    if (loadScore(sc, ":/view/resources/data/tab_sample.mscx")) {
        tabPreview->setScore(sc);
    } else {
        Q_ASSERT_X(false, "EditStaffType::EditStaffType", "Error in opening sample tab file for preview");
    }
    tabPreview->adjustSize();

    connect(name,           SIGNAL(textEdited(const QString&)), SLOT(nameEdited(const QString&)));
    connect(lines,          SIGNAL(valueChanged(int)),          SLOT(updatePreview()));
    connect(lineDistance,   SIGNAL(valueChanged(double)),       SLOT(updatePreview()));
    connect(showBarlines,   SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(genClef,        SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(genTimesig,     SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(noteHeadScheme, SIGNAL(currentIndexChanged(int)),   SLOT(updatePreview()));

    connect(genKeysigPitched,           SIGNAL(toggled(bool)),  SLOT(updatePreview()));
    connect(showLedgerLinesPitched,     SIGNAL(toggled(bool)),  SLOT(updatePreview()));
    connect(stemlessPitched,            SIGNAL(toggled(bool)),  SLOT(updatePreview()));
    connect(genKeysigPercussion,        SIGNAL(toggled(bool)),  SLOT(updatePreview()));
    connect(showLedgerLinesPercussion,  SIGNAL(toggled(bool)),  SLOT(updatePreview()));
    connect(stemlessPercussion,         SIGNAL(toggled(bool)),  SLOT(updatePreview()));

    connect(noteValuesSymb,       SIGNAL(toggled(bool)),              SLOT(tabStemsToggled(bool)));
    connect(noteValuesStems,      SIGNAL(toggled(bool)),              SLOT(tabStemsToggled(bool)));
    connect(valuesRepeatNever,    SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(valuesRepeatSystem,   SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(valuesRepeatMeasure,  SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(valuesRepeatAlways,   SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(stemBesideRadio,      SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(stemThroughRadio,     SIGNAL(toggled(bool)),              SLOT(tabStemThroughToggled(bool)));
    connect(stemAboveRadio,       SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(stemBelowRadio,       SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(minimShortRadio,      SIGNAL(toggled(bool)),              SLOT(tabMinimShortToggled(bool)));
    connect(minimSlashedRadio,    SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(showRests,            SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(durFontName,          SIGNAL(currentIndexChanged(int)),   SLOT(durFontNameChanged(int)));
    connect(durFontSize,          SIGNAL(valueChanged(double)),       SLOT(updatePreview()));
    connect(durY,                 SIGNAL(valueChanged(double)),       SLOT(updatePreview()));
    connect(fretFontName,         SIGNAL(currentIndexChanged(int)),   SLOT(fretFontNameChanged(int)));
    connect(fretFontSize,         SIGNAL(valueChanged(double)),       SLOT(updatePreview()));
    connect(fretY,                SIGNAL(valueChanged(double)),       SLOT(updatePreview()));

    connect(linesThroughRadio,    SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(onLinesRadio,         SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(showTabFingering,     SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(upsideDown,           SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(numbersRadio,         SIGNAL(toggled(bool)),              SLOT(updatePreview()));
    connect(showBackTied,         SIGNAL(toggled(bool)),              SLOT(updatePreview()));

    connect(templateReset,        SIGNAL(clicked()),                  SLOT(resetToTemplateClicked()));
    connect(addToTemplates,       SIGNAL(clicked()),                  SLOT(addToTemplatesClicked()));
//      connect(groupCombo,           SIGNAL(currentIndexChanged(int)),   SLOT(staffGroupChanged(int)));
    addToTemplates->setVisible(false);

    WidgetStateStore::restoreGeometry(this);
}

void EditStaffType::setStaffType(const Ms::StaffType* stafftype)
{
    this->staffType = *stafftype;

    setValues();
}

void EditStaffType::setInstrument(const Instrument& instrument)
{
    // template combo

    templateCombo->clear();
    // standard group also as fall-back (but excluded by percussion)
    bool bStandard    = !(instrument.drumset != nullptr);
    bool bPerc        = (instrument.drumset != nullptr);
    bool bTab = (instrument.stringData.frettedStrings() > 0);
    int idx           = 0;
    for (const Ms::StaffType& t : Ms::StaffType::presets()) {
        if ((t.group() == Ms::StaffGroup::STANDARD && bStandard)
            || (t.group() == Ms::StaffGroup::PERCUSSION && bPerc)
            || (t.group() == Ms::StaffGroup::TAB && bTab && t.lines() <= instrument.stringData.frettedStrings())) {
            templateCombo->addItem(t.name(), idx);
        }
        idx++;
    }
    templateCombo->setCurrentIndex(-1);
}

mu::Ret EditStaffType::loadScore(Ms::MasterScore* score, const mu::io::path& path)
{
    Ms::ScoreLoad sl;

    return doLoadScore(score, path);
}

mu::Ret EditStaffType::doLoadScore(Ms::MasterScore* score, const mu::io::path& path) const
{
    QFileInfo fi(path.toQString());
    score->setName(fi.completeBaseName());
    score->setImportedFilePath(fi.filePath());
    score->setMetaTag("originalFormat", fi.suffix().toLower());

    if (compat::loadMsczOrMscx(score, path.toQString()) != Ms::Score::FileError::FILE_NO_ERROR) {
        return make_ret(Ret::Code::UnknownError);
    }

    score->connectTies();

    for (Ms::Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    score->setSoloMute();
    for (Ms::Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(Ms::LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }
    score->updateChannel();
    //score->updateExpressive(MuseScore::synthesizer("Fluid"));
    score->setSaved(true);
    score->update();

    if (!score->sanityCheck(QString())) {
        return make_ret(Err::FileCorrupted, path);
    }

    return make_ret(Ret::Code::Ok);
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStaffType::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

//---------------------------------------------------------
//   staffGroupChanged
//---------------------------------------------------------
/*
void EditStaffType::staffGroupChanged(int n)
      {
      int groupIdx = groupCombo->itemData(groupCombo->currentIndex()).toInt();
      StaffGroup group = StaffGroup(groupIdx);
      staffType = *StaffType::getDefaultPreset(group); // overwrite with default
      setValues();
      }
*/
//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStaffType::setValues()
{
    blockSignals(true);

    Ms::StaffGroup group = staffType.group();
    int i = int(group);
    stack->setCurrentIndex(i);
    groupName->setText(qApp->translate("staff group header name", g_groupNames[i]));
//      groupCombo->setCurrentIndex(i);

    name->setText(staffType.name());
    lines->setValue(staffType.lines());
    lineDistance->setValue(staffType.lineDistance().val());
    genClef->setChecked(staffType.genClef());
    showBarlines->setChecked(staffType.showBarlines());
    genTimesig->setChecked(staffType.genTimesig());

    switch (group) {
    case Ms::StaffGroup::STANDARD:
        genKeysigPitched->setChecked(staffType.genKeysig());
        showLedgerLinesPitched->setChecked(staffType.showLedgerLines());
        stemlessPitched->setChecked(staffType.stemless());
        noteHeadScheme->setCurrentIndex(int(staffType.noteHeadScheme()));
        break;

    case Ms::StaffGroup::TAB:
    {
        upsideDown->setChecked(staffType.upsideDown());
        showTabFingering->setChecked(staffType.showTabFingering());
        int idx = fretFontName->findText(staffType.fretFontName(), Qt::MatchFixedString);
        if (idx == -1) {
            idx = 0;                      // if name not found, use first name
        }
        fretFontName->setCurrentIndex(idx);
        fretFontSize->setValue(staffType.fretFontSize());
        fretY->setValue(staffType.fretFontUserY());

        numbersRadio->setChecked(staffType.useNumbers());
        lettersRadio->setChecked(!staffType.useNumbers());
        onLinesRadio->setChecked(staffType.onLines());
        aboveLinesRadio->setChecked(!staffType.onLines());
        linesThroughRadio->setChecked(staffType.linesThrough());
        linesBrokenRadio->setChecked(!staffType.linesThrough());
        showBackTied->setChecked(staffType.showBackTied());

        idx = durFontName->findText(staffType.durationFontName(), Qt::MatchFixedString);
        if (idx == -1) {
            idx = 0;                      // if name not found, use first name
        }
        durFontName->setCurrentIndex(idx);
        durFontSize->setValue(staffType.durationFontSize());
        durY->setValue(staffType.durationFontUserY());
        // convert combined values of genDurations and slashStyle/stemless into noteValuesx radio buttons
        // Sbove/Below, Beside/Through and minim are only used if stems-and-beams
        // but set them from stt values anyway, to ensure preset matching
        stemAboveRadio->setChecked(!staffType.stemsDown());
        stemBelowRadio->setChecked(staffType.stemsDown());
        stemBesideRadio->setChecked(!staffType.stemThrough());
        stemThroughRadio->setChecked(staffType.stemThrough());
        Ms::TablatureMinimStyle minimStyle = staffType.minimStyle();
        minimNoneRadio->setChecked(minimStyle == Ms::TablatureMinimStyle::NONE);
        minimShortRadio->setChecked(minimStyle == Ms::TablatureMinimStyle::SHORTER);
        minimSlashedRadio->setChecked(minimStyle == Ms::TablatureMinimStyle::SLASHED);
        Ms::TablatureSymbolRepeat symRepeat = staffType.symRepeat();
        valuesRepeatNever->setChecked(symRepeat == Ms::TablatureSymbolRepeat::NEVER);
        valuesRepeatSystem->setChecked(symRepeat == Ms::TablatureSymbolRepeat::SYSTEM);
        valuesRepeatMeasure->setChecked(symRepeat == Ms::TablatureSymbolRepeat::MEASURE);
        valuesRepeatAlways->setChecked(symRepeat == Ms::TablatureSymbolRepeat::ALWAYS);
        if (staffType.genDurations()) {
            noteValuesNone->setChecked(false);
            noteValuesSymb->setChecked(true);
            noteValuesStems->setChecked(false);
        } else {
            if (staffType.stemless()) {
                noteValuesNone->setChecked(true);
                noteValuesSymb->setChecked(false);
                noteValuesStems->setChecked(false);
            } else {
                noteValuesNone->setChecked(false);
                noteValuesSymb->setChecked(false);
                noteValuesStems->setChecked(true);
            }
        }
        showRests->setChecked(staffType.showRests());
        // adjust compatibility across different settings
        tabStemThroughCompatibility(stemThroughRadio->isChecked());
        tabMinimShortCompatibility(minimShortRadio->isChecked());
        tabStemsCompatibility(noteValuesStems->isChecked());
    }
    break;

    case Ms::StaffGroup::PERCUSSION:
        genKeysigPercussion->setChecked(staffType.genKeysig());
        showLedgerLinesPercussion->setChecked(staffType.showLedgerLines());
        stemlessPercussion->setChecked(staffType.stemless());
        break;
    }
    updatePreview();
    blockSignals(false);
}

//---------------------------------------------------------
//   nameEdited
//---------------------------------------------------------

void EditStaffType::nameEdited(const QString& /*s*/)
{
//      staffTypeList->currentItem()->setText(s);
}

//=========================================================
//   PERCUSSION PAGE METHODS
//=========================================================

//=========================================================
//   TABULATURE PAGE METHODS
//=========================================================

//---------------------------------------------------------
//   Tabulature duration / fret font name changed
//
//    set depending parameters
//---------------------------------------------------------

void EditStaffType::durFontNameChanged(int idx)
{
    qreal size, yOff;
    if (Ms::StaffType::fontData(true, idx, 0, 0, &size, &yOff)) {
        durFontSize->setValue(size);
        durY->setValue(yOff);
    }
    updatePreview();
}

void EditStaffType::fretFontNameChanged(int idx)
{
    qreal size, yOff;
    if (Ms::StaffType::fontData(false, idx, 0, 0, &size, &yOff)) {
        fretFontSize->setValue(size);
        fretY->setValue(yOff);
    }
    updatePreview();
}

//---------------------------------------------------------
//   Tabulature note stems toggled
//
//    enable / disable all controls related to stems
//---------------------------------------------------------

void EditStaffType::tabStemsToggled(bool checked)
{
    tabStemsCompatibility(checked);
    updatePreview();
}

//---------------------------------------------------------
//   Tabulature "minim short" toggled
//
//    contra-toggle "stems through"
//---------------------------------------------------------

void EditStaffType::tabMinimShortToggled(bool checked)
{
    tabMinimShortCompatibility(checked);
    updatePreview();
}

//---------------------------------------------------------
//   Tabulature "stems through" toggled
//---------------------------------------------------------

void EditStaffType::tabStemThroughToggled(bool checked)
{
    tabStemThroughCompatibility(checked);
    updatePreview();
}

//---------------------------------------------------------
//   setFromDlg
//
//    initializes a StaffType from dlg controls
//---------------------------------------------------------

void EditStaffType::setFromDlg()
{
    staffType.setName(name->text());
    staffType.setLines(lines->value());
    staffType.setLineDistance(Ms::Spatium(lineDistance->value()));
    staffType.setGenClef(genClef->isChecked());
    staffType.setShowBarlines(showBarlines->isChecked());
    staffType.setGenTimesig(genTimesig->isChecked());
    if (staffType.group() == Ms::StaffGroup::STANDARD) {
        staffType.setGenKeysig(genKeysigPitched->isChecked());
        staffType.setShowLedgerLines(showLedgerLinesPitched->isChecked());
        staffType.setStemless(stemlessPitched->isChecked());
        staffType.setNoteHeadScheme(Ms::NoteHead::name2scheme(noteHeadScheme->currentData().toString()));
    }
    if (staffType.group() == Ms::StaffGroup::PERCUSSION) {
        staffType.setGenKeysig(genKeysigPercussion->isChecked());
        staffType.setShowLedgerLines(showLedgerLinesPercussion->isChecked());
        staffType.setStemless(stemlessPercussion->isChecked());
    }
    staffType.setDurationFontName(durFontName->currentText());
    staffType.setDurationFontSize(durFontSize->value());
    staffType.setDurationFontUserY(durY->value());
    staffType.setFretFontName(fretFontName->currentText());
    staffType.setFretFontSize(fretFontSize->value());
    staffType.setFretFontUserY(fretY->value());
    staffType.setLinesThrough(linesThroughRadio->isChecked());
    staffType.setShowBackTied(showBackTied->isChecked());
    staffType.setMinimStyle(minimNoneRadio->isChecked() ? Ms::TablatureMinimStyle::NONE
                            : (minimShortRadio->isChecked() ? Ms::TablatureMinimStyle::SHORTER : Ms::TablatureMinimStyle::
                               SLASHED));
    staffType.setSymbolRepeat(valuesRepeatNever->isChecked() ? Ms::TablatureSymbolRepeat::NEVER
                              : (valuesRepeatSystem->isChecked() ? Ms::TablatureSymbolRepeat::SYSTEM
                                 : valuesRepeatMeasure->isChecked() ? Ms::TablatureSymbolRepeat::MEASURE
                                 : Ms::TablatureSymbolRepeat::ALWAYS));
    staffType.setOnLines(onLinesRadio->isChecked());
    staffType.setShowRests(showRests->isChecked());
    staffType.setUpsideDown(upsideDown->isChecked());
    staffType.setShowTabFingering(showTabFingering->isChecked());
    staffType.setUseNumbers(numbersRadio->isChecked());
    //note values
    staffType.setStemsDown(stemBelowRadio->isChecked());
    staffType.setStemsThrough(stemThroughRadio->isChecked());
    if (staffType.group() == Ms::StaffGroup::TAB) {
        staffType.setGenKeysig(false);
        staffType.setStemless(true);                       // assume no note values
        staffType.setGenDurations(false);                  //    "     "
        if (noteValuesSymb->isChecked()) {
            staffType.setGenDurations(true);
        }
        if (noteValuesStems->isChecked()) {
            staffType.setStemless(false);
        }
    }
}

//---------------------------------------------------------
//   Block preview signals
//---------------------------------------------------------

void EditStaffType::blockSignals(bool block)
{
    stack->blockSignals(block);
//      groupCombo->blockSignals(block);
    lines->blockSignals(block);
    lineDistance->blockSignals(block);
    showBarlines->blockSignals(block);
    genClef->blockSignals(block);
    genTimesig->blockSignals(block);
    noteValuesSymb->blockSignals(block);
    noteValuesStems->blockSignals(block);
    durFontName->blockSignals(block);
    durFontSize->blockSignals(block);
    durY->blockSignals(block);
    fretFontName->blockSignals(block);
    fretFontSize->blockSignals(block);
    fretY->blockSignals(block);

    numbersRadio->blockSignals(block);
    linesThroughRadio->blockSignals(block);
    onLinesRadio->blockSignals(block);
    showBackTied->blockSignals(block);

    upsideDown->blockSignals(block);
    showTabFingering->blockSignals(block);
    valuesRepeatNever->blockSignals(block);
    valuesRepeatSystem->blockSignals(block);
    valuesRepeatMeasure->blockSignals(block);
    valuesRepeatAlways->blockSignals(block);
    stemAboveRadio->blockSignals(block);
    stemBelowRadio->blockSignals(block);
    stemBesideRadio->blockSignals(block);
    stemThroughRadio->blockSignals(block);
    minimShortRadio->blockSignals(block);
    minimSlashedRadio->blockSignals(block);
    showRests->blockSignals(block);

    showLedgerLinesPercussion->blockSignals(block);
    genKeysigPercussion->blockSignals(block);
    stemlessPercussion->blockSignals(block);
}

//---------------------------------------------------------
//   Tabulature note stems compatibility
//
//    Enable / disable all stem-related controls according to "Stems and beams" is checked/unchecked
//---------------------------------------------------------

void EditStaffType::tabStemsCompatibility(bool checked)
{
    valuesRepeatNever->setEnabled(noteValuesSymb->isChecked());
    valuesRepeatSystem->setEnabled(noteValuesSymb->isChecked());
    valuesRepeatMeasure->setEnabled(noteValuesSymb->isChecked());
    valuesRepeatAlways->setEnabled(noteValuesSymb->isChecked());
    stemAboveRadio->setEnabled(checked && !stemThroughRadio->isChecked());
    stemBelowRadio->setEnabled(checked && !stemThroughRadio->isChecked());
    stemBesideRadio->setEnabled(checked);
    stemThroughRadio->setEnabled(checked && !minimShortRadio->isChecked());
    minimNoneRadio->setEnabled(checked);
    minimShortRadio->setEnabled(checked && !stemThroughRadio->isChecked());
    minimSlashedRadio->setEnabled(checked);
}

//---------------------------------------------------------
//   Tabulature "minim short" compatibility
//
//    Setting "short minim" stems is incompatible with "stems through":
//    if checked and "stems through" is checked, move check to "stems beside"
//---------------------------------------------------------

void EditStaffType::tabMinimShortCompatibility(bool checked)
{
    if (checked) {
        if (stemThroughRadio->isChecked()) {
            stemThroughRadio->setChecked(false);
            stemBesideRadio->setChecked(true);
        }
    }
    // disable / enable "stems through" according "minim short" is checked / unchecked
    stemThroughRadio->setEnabled(!checked && noteValuesStems->isChecked());
}

//---------------------------------------------------------
//   Tabulature "stems through" compatibility
//
//    Setting "stems through" is incompatible with "minim short":
//    if checking and "minim short" is checked, move check to "minim slashed"
//    It also make "Stems above" and "Stems below" meaningless: disable them
//---------------------------------------------------------

void EditStaffType::tabStemThroughCompatibility(bool checked)
{
    if (checked) {
        if (minimShortRadio->isChecked()) {
            minimShortRadio->setChecked(false);
            minimSlashedRadio->setChecked(true);
        }
    }
    // disable / enable "minim short" and "stems above/below" according "stems through" is checked / unchecked
    bool enab = !checked && noteValuesStems->isChecked();
    minimShortRadio->setEnabled(enab);
    stemAboveRadio->setEnabled(enab);
    stemBelowRadio->setEnabled(enab);
}

//---------------------------------------------------------
//    updatePreview
///   update staff type in preview score
//---------------------------------------------------------

void EditStaffType::updatePreview()
{
    setFromDlg();
    Ms::ExampleView* preview = nullptr;
    if (staffType.group() == Ms::StaffGroup::TAB) {
        preview = tabPreview;
    } else if (staffType.group() == Ms::StaffGroup::STANDARD) {
        preview = standardPreview;
    }
    if (preview) {
        preview->score()->staff(0)->setStaffType(Ms::Fraction(0, 1), staffType);
        preview->score()->doLayout();
        preview->updateAll();
        preview->update();
    }
}

//---------------------------------------------------------
//   createUniqueStaffTypeName
///  create unique new name for StaffType
//---------------------------------------------------------

QString EditStaffType::createUniqueStaffTypeName(Ms::StaffGroup group)
{
    QString sn;
    for (int idx = 1;; ++idx) {
        switch (group) {
        case Ms::StaffGroup::STANDARD:
            sn = QString("Standard-%1 [*]").arg(idx);
            break;
        case Ms::StaffGroup::PERCUSSION:
            sn = QString("Perc-%1 [*]").arg(idx);
            break;
        case Ms::StaffGroup::TAB:
            sn = QString("Tab-%1 [*]").arg(idx);
            break;
        }
        bool found = false;
        for (const Ms::StaffType& st : Ms::StaffType::presets()) {
            if (st.name() == sn) {
                found = true;
                break;
            }
        }
        if (!found) {
            break;
        }
    }
    return sn;
}

//---------------------------------------------------------
//   savePresets
//---------------------------------------------------------

void EditStaffType::savePresets()
{
    qDebug("savePresets");
}

//---------------------------------------------------------
//   loadPresets
//---------------------------------------------------------

void EditStaffType::loadPresets()
{
    qDebug("loadPresets");
}

void EditStaffType::resetToTemplateClicked()
{
    int idx = templateCombo->itemData(templateCombo->currentIndex()).toInt();
    if (idx >= 0) {
        staffType = *(Ms::StaffType::preset(Ms::StaffTypes(idx)));
        setValues();
    }
}

//---------------------------------------------------------
//   addToTemplates
//---------------------------------------------------------

void EditStaffType::addToTemplatesClicked()
{
    qDebug("not implemented: add to templates");
}
