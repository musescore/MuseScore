/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "engraving/dom/part.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stringdata.h"

#include "engraving/types/typesconv.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"

#include "ui/view/widgetstatestore.h"

#include "notationerrors.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::ui;

//---------------------------------------------------------
//   noteHeadSchemes
//---------------------------------------------------------

mu::engraving::NoteHeadScheme noteHeadSchemes[] = {
    mu::engraving::NoteHeadScheme::HEAD_NORMAL,
    mu::engraving::NoteHeadScheme::HEAD_PITCHNAME,
    mu::engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN,
    mu::engraving::NoteHeadScheme::HEAD_SOLFEGE,
    mu::engraving::NoteHeadScheme::HEAD_SOLFEGE_FIXED,
    mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_4,
    mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,
    mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,
    mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER
};

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

EditStaffType::EditStaffType(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setObjectName("EditStaffType");
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi(this);

    // tab page configuration
    std::vector<String> fontNames = mu::engraving::StaffType::fontNames(false);
    for (const String& fn : fontNames) {   // fill fret font name combo
        fretFontName->addItem(fn.toQString());
    }
    fretFontName->setCurrentIndex(0);
    fontNames = mu::engraving::StaffType::fontNames(true);
    for (const String& fn : fontNames) {  // fill duration font name combo
        durFontName->addItem(fn.toQString());
    }
    durFontName->setCurrentIndex(0);

    for (auto i : noteHeadSchemes) {
        noteHeadScheme->addItem(TConv::translatedUserName(i), static_cast<int>(i));
    }

    // load a sample standard score in preview
    mu::engraving::MasterScore* sc = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle(iocContext());
    if (loadScore(sc, ":/view/resources/data/std_sample.mscx")) {
        standardPreview->setScore(sc);
    } else {
        Q_ASSERT_X(false, "EditStaffType::EditStaffType", "Error in opening sample standard file for preview");
    }

    // load a sample tablature score in preview
    sc = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle(iocContext());
    if (loadScore(sc, ":/view/resources/data/tab_sample.mscx")) {
        tabPreview->setScore(sc);
    } else {
        Q_ASSERT_X(false, "EditStaffType::EditStaffType", "Error in opening sample tab file for preview");
    }
    tabPreview->adjustSize();

    connect(name, &QLineEdit::textEdited, this, &EditStaffType::nameEdited);

    connect(lines,        &QSpinBox::valueChanged,       this, &EditStaffType::updatePreview);
    connect(lineDistance, &QDoubleSpinBox::valueChanged, this, &EditStaffType::updatePreview);

    connect(showBarlines, &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(genClef,      &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(genTimesig,   &QCheckBox::toggled, this, &EditStaffType::updatePreview);

    connect(noteHeadScheme, &QComboBox::currentIndexChanged, this, &EditStaffType::updatePreview);

    connect(genKeysigPitched,          &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(showLedgerLinesPitched,    &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(stemlessPitched,           &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(genKeysigPercussion,       &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(showLedgerLinesPercussion, &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(stemlessPercussion,        &QCheckBox::toggled, this, &EditStaffType::updatePreview);

    connect(noteValuesSymb,      &QRadioButton::toggled, this, &EditStaffType::tabStemsToggled);
    connect(noteValuesStems,     &QRadioButton::toggled, this, &EditStaffType::tabStemsToggled);
    connect(valuesRepeatNever,   &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(valuesRepeatSystem,  &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(valuesRepeatMeasure, &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(valuesRepeatAlways,  &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(stemBesideRadio,     &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(stemThroughRadio,    &QRadioButton::toggled, this, &EditStaffType::tabStemThroughToggled);
    connect(stemAboveRadio,      &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(stemBelowRadio,      &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(minimShortRadio,     &QRadioButton::toggled, this, &EditStaffType::tabMinimShortToggled);
    connect(minimSlashedRadio,   &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(showRests,           &QRadioButton::toggled, this, &EditStaffType::updatePreview);

    connect(durFontName, &QComboBox::currentIndexChanged, this, &EditStaffType::durFontNameChanged);
    connect(durFontSize, &QDoubleSpinBox::valueChanged, this, &EditStaffType::updatePreview);
    connect(durY,        &QDoubleSpinBox::valueChanged, this, &EditStaffType::updatePreview);
    connect(fretFontName, &QComboBox::currentIndexChanged, this, &EditStaffType::fretFontNameChanged);
    connect(fretFontSize, &QDoubleSpinBox::valueChanged, this, &EditStaffType::updatePreview);
    connect(fretY,        &QDoubleSpinBox::valueChanged, this, &EditStaffType::updatePreview);

    connect(linesThroughRadio, &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(onLinesRadio,      &QRadioButton::toggled, this, &EditStaffType::updatePreview);
    connect(showTabFingering,  &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(upsideDown,        &QCheckBox::toggled, this, &EditStaffType::updatePreview);
    connect(numbersRadio,      &QCheckBox::toggled, this, &EditStaffType::updatePreview);

    connect(templateReset,  &QPushButton::clicked, this, &EditStaffType::resetToTemplateClicked);
    connect(addToTemplates, &QPushButton::clicked, this, &EditStaffType::addToTemplatesClicked);

    //connect(groupCombo, &QComboBox::currentIndexChanged, this, &EditStaffType::staffGroupChanged);

    addToTemplates->setVisible(false);

    WidgetStateStore::restoreGeometry(this);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

void EditStaffType::setStaffType(const mu::engraving::StaffType* stafftype)
{
    this->staffType = *stafftype;

    setValues();
}

void EditStaffType::setInstrument(const Instrument& instrument)
{
    // template combo

    templateCombo->clear();
    // standard group also as fall-back (but excluded by percussion)
    bool bStandard    = !(instrument.drumset() != nullptr);
    bool bPerc        = (instrument.drumset() != nullptr);
    bool bTab = (instrument.stringData()->frettedStrings() > 0);
    int idx           = 0;
    for (const mu::engraving::StaffType& t : mu::engraving::StaffType::presets()) {
        if ((t.group() == mu::engraving::StaffGroup::STANDARD && bStandard)
            || (t.group() == mu::engraving::StaffGroup::PERCUSSION && bPerc)
            || (t.group() == mu::engraving::StaffGroup::TAB && bTab && t.lines() <= instrument.stringData()->frettedStrings())) {
            templateCombo->addItem(t.name(), idx);
        }
        idx++;
    }
    templateCombo->setCurrentIndex(-1);
}

Ret EditStaffType::loadScore(mu::engraving::MasterScore* score, const muse::io::path_t& path)
{
    mu::engraving::ScoreLoad sl;

    Ret ret = compat::loadMsczOrMscx(score, path.toQString());
    if (!ret) {
        return ret;
    }

    score->connectTies();

    for (mu::engraving::Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    for (mu::engraving::Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->setLayoutAll();
    }
    score->updateChannel();
    score->setSaved(true);
    score->update();

    return score->sanityCheck();
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

    mu::engraving::StaffGroup group = staffType.group();
    int i = int(group);
    stack->setCurrentIndex(i);
    groupName->setText(TConv::translatedUserName(group));
//      groupCombo->setCurrentIndex(i);

    name->setText(staffType.name());
    lines->setValue(staffType.lines());
    lineDistance->setValue(staffType.lineDistance().val());
    genClef->setChecked(staffType.genClef());
    showBarlines->setChecked(staffType.showBarlines());
    genTimesig->setChecked(staffType.genTimesig());

    switch (group) {
    case mu::engraving::StaffGroup::STANDARD:
        genKeysigPitched->setChecked(staffType.genKeysig());
        showLedgerLinesPitched->setChecked(staffType.showLedgerLines());
        stemlessPitched->setChecked(staffType.stemless());
        noteHeadScheme->setCurrentIndex(int(staffType.noteHeadScheme()));
        break;

    case mu::engraving::StaffGroup::TAB:
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

        idx = durFontName->findText(staffType.durationFontName(), Qt::MatchFixedString);
        if (idx == -1) {
            idx = 0;                      // if name not found, use first name
        }
        durFontName->setCurrentIndex(idx);
        durFontSize->setValue(staffType.durationFontSize());
        durY->setValue(staffType.durationFontUserY());
        // convert combined values of genDurations and slashStyle/stemless into noteValuesx radio buttons
        // Above/Below, Beside/Through and minim are only used if stems-and-beams
        // but set them from stt values anyway, to ensure preset matching
        stemAboveRadio->setChecked(!staffType.stemsDown());
        stemBelowRadio->setChecked(staffType.stemsDown());
        stemBesideRadio->setChecked(!staffType.stemThrough());
        stemThroughRadio->setChecked(staffType.stemThrough());
        mu::engraving::TablatureMinimStyle minimStyle = staffType.minimStyle();
        minimNoneRadio->setChecked(minimStyle == mu::engraving::TablatureMinimStyle::NONE);
        minimShortRadio->setChecked(minimStyle == mu::engraving::TablatureMinimStyle::SHORTER);
        minimSlashedRadio->setChecked(minimStyle == mu::engraving::TablatureMinimStyle::SLASHED);
        mu::engraving::TablatureSymbolRepeat symRepeat = staffType.symRepeat();
        valuesRepeatNever->setChecked(symRepeat == mu::engraving::TablatureSymbolRepeat::NEVER);
        valuesRepeatSystem->setChecked(symRepeat == mu::engraving::TablatureSymbolRepeat::SYSTEM);
        valuesRepeatMeasure->setChecked(symRepeat == mu::engraving::TablatureSymbolRepeat::MEASURE);
        valuesRepeatAlways->setChecked(symRepeat == mu::engraving::TablatureSymbolRepeat::ALWAYS);
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

    case mu::engraving::StaffGroup::PERCUSSION:
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
    if (mu::engraving::StaffType::fontData(true, idx, 0, 0, &size, &yOff)) {
        durFontSize->setValue(size);
        durY->setValue(yOff);
    }
    updatePreview();
}

void EditStaffType::fretFontNameChanged(int idx)
{
    qreal size, yOff;
    if (mu::engraving::StaffType::fontData(false, idx, 0, 0, &size, &yOff)) {
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
    staffType.setLineDistance(mu::engraving::Spatium(lineDistance->value()));
    staffType.setGenClef(genClef->isChecked());
    staffType.setShowBarlines(showBarlines->isChecked());
    staffType.setGenTimesig(genTimesig->isChecked());
    if (staffType.group() == mu::engraving::StaffGroup::STANDARD) {
        staffType.setGenKeysig(genKeysigPitched->isChecked());
        staffType.setShowLedgerLines(showLedgerLinesPitched->isChecked());
        staffType.setStemless(stemlessPitched->isChecked());
        staffType.setNoteHeadScheme(static_cast<NoteHeadScheme>(noteHeadScheme->currentData().toInt()));
    }
    if (staffType.group() == mu::engraving::StaffGroup::PERCUSSION) {
        staffType.setGenKeysig(genKeysigPercussion->isChecked());
        staffType.setShowLedgerLines(showLedgerLinesPercussion->isChecked());
        staffType.setStemless(stemlessPercussion->isChecked());
    }
    if (staffType.group() == mu::engraving::StaffGroup::TAB) {
        staffType.setDurationFontName(durFontName->currentText());
        staffType.setDurationFontSize(durFontSize->value());
        staffType.setDurationFontUserY(durY->value());
        staffType.setFretFontName(fretFontName->currentText());
        staffType.setFretFontSize(fretFontSize->value());
        staffType.setFretFontUserY(fretY->value());
        staffType.setLinesThrough(linesThroughRadio->isChecked());
        staffType.setMinimStyle(minimNoneRadio->isChecked() ? mu::engraving::TablatureMinimStyle::NONE
                                : (minimShortRadio->isChecked() ? mu::engraving::TablatureMinimStyle::SHORTER : mu::engraving::
                                   TablatureMinimStyle::
                                   SLASHED));
        staffType.setSymbolRepeat(valuesRepeatNever->isChecked() ? mu::engraving::TablatureSymbolRepeat::NEVER
                                  : (valuesRepeatSystem->isChecked() ? mu::engraving::TablatureSymbolRepeat::SYSTEM
                                     : valuesRepeatMeasure->isChecked() ? mu::engraving::TablatureSymbolRepeat::MEASURE
                                     : mu::engraving::TablatureSymbolRepeat::ALWAYS));
        staffType.setOnLines(onLinesRadio->isChecked());
        staffType.setShowRests(showRests->isChecked());
        staffType.setUpsideDown(upsideDown->isChecked());
        staffType.setShowTabFingering(showTabFingering->isChecked());
        staffType.setUseNumbers(numbersRadio->isChecked());
        //note values
        staffType.setStemsDown(stemBelowRadio->isChecked());
        staffType.setStemsThrough(stemThroughRadio->isChecked());
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
    genClef->blockSignals(block);
    showBarlines->blockSignals(block);
    genTimesig->blockSignals(block);

    genKeysigPitched->blockSignals(block);
    showLedgerLinesPitched->blockSignals(block);
    stemlessPitched->blockSignals(block);
    noteHeadScheme->blockSignals(block);

    upsideDown->blockSignals(block);
    showTabFingering->blockSignals(block);

    fretFontName->blockSignals(block);
    fretFontSize->blockSignals(block);
    fretY->blockSignals(block);

    numbersRadio->blockSignals(block);
    lettersRadio->blockSignals(block);
    onLinesRadio->blockSignals(block);
    aboveLinesRadio->blockSignals(block);
    linesThroughRadio->blockSignals(block);
    linesBrokenRadio->blockSignals(block);

    durFontName->blockSignals(block);
    durFontSize->blockSignals(block);
    durY->blockSignals(block);

    stemAboveRadio->blockSignals(block);
    stemBelowRadio->blockSignals(block);
    stemBesideRadio->blockSignals(block);
    stemThroughRadio->blockSignals(block);

    minimNoneRadio->blockSignals(block);
    minimShortRadio->blockSignals(block);
    minimSlashedRadio->blockSignals(block);

    valuesRepeatNever->blockSignals(block);
    valuesRepeatSystem->blockSignals(block);
    valuesRepeatMeasure->blockSignals(block);
    valuesRepeatAlways->blockSignals(block);

    noteValuesNone->blockSignals(block);
    noteValuesSymb->blockSignals(block);
    noteValuesStems->blockSignals(block);

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
    ExampleView* preview = nullptr;
    if (staffType.group() == mu::engraving::StaffGroup::TAB) {
        preview = tabPreview;
    } else if (staffType.group() == mu::engraving::StaffGroup::STANDARD) {
        preview = standardPreview;
    }
    if (preview) {
        preview->score()->staff(0)->setStaffType(mu::engraving::Fraction(0, 1), staffType);
        preview->score()->doLayout();
        preview->updateAll();
        preview->update();
    }
}

//---------------------------------------------------------
//   createUniqueStaffTypeName
///  create unique new name for StaffType
//---------------------------------------------------------

QString EditStaffType::createUniqueStaffTypeName(mu::engraving::StaffGroup group)
{
    QString sn;
    for (int idx = 1;; ++idx) {
        switch (group) {
        case mu::engraving::StaffGroup::STANDARD:
            sn = QString("Standard-%1 [*]").arg(idx);
            break;
        case mu::engraving::StaffGroup::PERCUSSION:
            sn = QString("Perc-%1 [*]").arg(idx);
            break;
        case mu::engraving::StaffGroup::TAB:
            sn = QString("Tab-%1 [*]").arg(idx);
            break;
        }
        bool found = false;
        for (const mu::engraving::StaffType& st : mu::engraving::StaffType::presets()) {
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
    LOGD("savePresets");
}

//---------------------------------------------------------
//   loadPresets
//---------------------------------------------------------

void EditStaffType::loadPresets()
{
    LOGD("loadPresets");
}

void EditStaffType::resetToTemplateClicked()
{
    int idx = templateCombo->itemData(templateCombo->currentIndex()).toInt();
    if (idx >= 0) {
        staffType = *(mu::engraving::StaffType::preset(mu::engraving::StaffTypes(idx)));
        setValues();
    }
}

//---------------------------------------------------------
//   addToTemplates
//---------------------------------------------------------

void EditStaffType::addToTemplatesClicked()
{
    LOGD("not implemented: add to templates");
}
