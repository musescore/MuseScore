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

#include "editstaff.h"

#include "containers.h"
#include "translation.h"
#include "global/utils.h"

#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetutils.h"

#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/instrumentname.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/system.h"
#include "engraving/dom/text.h"
#include "engraving/dom/utils.h"
#include "engraving/editing/undo.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace mu::engraving;

static const QChar GO_UP_ICON = iconCodeToChar(IconCode::Code::ARROW_UP);
static const QChar GO_DOWN_ICON = iconCodeToChar(IconCode::Code::ARROW_DOWN);
static const QChar EDIT_ICON = iconCodeToChar(IconCode::Code::EDIT);

EditStaff::EditStaff(QWidget* parent)
    : QDialog(parent), muse::Contextable(muse::iocCtxForQWidget(this))
{
    setObjectName("EditStaff");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    initStaff();

    editStaffTypeDialog = new EditStaffType(this);
    editStaffTypeDialog->setWindowModality(Qt::WindowModal);

    connect(buttonBox,        &QDialogButtonBox::clicked, this, &EditStaff::bboxClicked);
    connect(changeInstrument, &QPushButton::clicked, this, &EditStaff::showReplaceInstrumentDialog);
    connect(changeStaffType,  &QPushButton::clicked, this, &EditStaff::showStaffTypeDialog);
    connect(minPitchASelect,  &QPushButton::clicked, this, &EditStaff::minPitchAClicked);
    connect(maxPitchASelect,  &QPushButton::clicked, this, &EditStaff::maxPitchAClicked);
    connect(minPitchPSelect,  &QPushButton::clicked, this, &EditStaff::minPitchPClicked);
    connect(maxPitchPSelect,  &QPushButton::clicked, this, &EditStaff::maxPitchPClicked);
    connect(editStringData,   &QPushButton::clicked, this, &EditStaff::editStringDataClicked);
    connect(nextButton,       &QPushButton::clicked, this, &EditStaff::gotoNextStaff);
    connect(previousButton,   &QPushButton::clicked, this, &EditStaff::gotoPreviousStaff);

    connect(showClef,         &QCheckBox::clicked, this, &EditStaff::showClefChanged);
    connect(showTimesig,      &QCheckBox::clicked, this, &EditStaff::showTimeSigChanged);
    connect(showBarlines,     &QCheckBox::clicked, this, &EditStaff::showBarlinesChanged);
    connect(invisible,        &QCheckBox::clicked, this, &EditStaff::invisibleChanged);

    connect(useDefaultName, &QRadioButton::clicked, this, [&]() {
        useCustomNameChanged(!useDefaultName->isChecked());
    });
    connect(useCustomName, &QRadioButton::clicked, this, [&]() {
        useCustomNameChanged(useCustomName->isChecked());
    });

    connect(useDefaultGroupName, &QRadioButton::clicked, this, [&]() {
        useCustomGroupNameChanged(!useDefaultGroupName->isChecked());
    });
    connect(useCustomGroupName, &QRadioButton::clicked, this, [&]() {
        useCustomGroupNameChanged(useCustomGroupName->isChecked());
    });

    connect(useDefaultIndividualName, &QRadioButton::clicked, this, [&]() {
        useCustomIndividualNameChanged(!useDefaultIndividualName->isChecked());
    });
    connect(useCustomIndividualName, &QRadioButton::clicked, this, [&]() {
        useCustomIndividualNameChanged(useCustomIndividualName->isChecked());
    });

    connect(color, &Awl::ColorLabel::colorChanged, this, &EditStaff::colorChanged);

    connect(mag, &QDoubleSpinBox::valueChanged, this, &EditStaff::magChanged);

    connect(iList, &QComboBox::currentIndexChanged, this, &EditStaff::transpositionChanged);

    connect(lines, &QSpinBox::valueChanged, this, &EditStaff::numOfLinesChanged);
    connect(lineDistance, &QDoubleSpinBox::valueChanged, this, &EditStaff::lineDistanceChanged);

    connect(longStaffName, &QTextEdit::textChanged, this, &EditStaff::longNameChanged);
    connect(shortStaffName, &QTextEdit::textChanged, this, &EditStaff::shortNameChanged);

    WidgetUtils::setWidgetIcon(nextButton, IconCode::Code::ARROW_DOWN);
    WidgetUtils::setWidgetIcon(previousButton, IconCode::Code::ARROW_UP);
    WidgetUtils::setWidgetIcon(minPitchASelect, IconCode::Code::EDIT);
    WidgetUtils::setWidgetIcon(maxPitchASelect, IconCode::Code::EDIT);
    WidgetUtils::setWidgetIcon(minPitchPSelect, IconCode::Code::EDIT);
    WidgetUtils::setWidgetIcon(maxPitchPSelect, IconCode::Code::EDIT);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

void EditStaff::setStaff(Staff* s, const Fraction& tick)
{
    if (m_staff != nullptr) {
        delete m_staff;
    }

    m_orgStaff = s;

    if (!m_orgStaff) {
        return;
    }

    Part* part = m_orgStaff->part();

    auto it = muse::findLessOrEqual(part->instruments(), tick.ticks());
    if (it == part->instruments().cend()) {
        return;
    }

    m_instrument = *it->second;
    m_orgInstrument = m_instrument;

    m_instrumentKey.instrumentId = m_instrument.id();
    m_instrumentKey.partId = part->id();
    m_instrumentKey.tick = Fraction::fromTicks(it->first);

    m_staff = engraving::Factory::createStaff(part);
    mu::engraving::StaffType* orgStaffType = m_orgStaff->staffType(tick);
    m_tick = Fraction::fromTicks(m_orgStaff->staffTypeRange(tick).first);
    mu::engraving::StaffType* stt = m_staff->setStaffType(m_tick, *orgStaffType);

    m_staff->setPart(part);
    m_staff->setDefaultClefType(m_orgStaff->defaultClefType());
    m_staff->setUserDist(m_orgStaff->userDist());
    m_staff->setCutaway(m_orgStaff->cutaway());
    m_staff->setHideWhenEmpty(m_orgStaff->hideWhenEmpty());
    m_staff->setShowIfEntireSystemEmpty(m_orgStaff->showIfEntireSystemEmpty());
    m_staff->setHideSystemBarLine(m_orgStaff->hideSystemBarLine());
    m_staff->setMergeMatchingRests(m_orgStaff->mergeMatchingRests());
    m_staff->setReflectTranspositionInLinkedTab(m_orgStaff->reflectTranspositionInLinkedTab());

    // set dlg controls
    spinExtraDistance->setValue(s->userDist().val());
    invisible->setChecked(stt->invisible());
    color->setColor(stt->color().toQColor());
    mag->setValue(stt->userMag() * 100.0);

    hideSystemBarLine->setChecked(m_staff->hideSystemBarLine());
    mergeMatchingRests->setCurrentIndex(static_cast<int>(m_staff->mergeMatchingRests()));
    noReflectTranspositionInLinkedTab->setChecked(!m_staff->reflectTranspositionInLinkedTab());

    updateStaffType(*stt);
    updateInstrument();
    updateNextPreviousButtons();
}

void EditStaff::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QDialog::showEvent(event);
}

void EditStaff::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

void EditStaff::updateStaffType(const mu::engraving::StaffType& staffType)
{
    lines->setValue(staffType.lines());
    lineDistance->setValue(staffType.lineDistance().val());
    showClef->setChecked(staffType.genClef());
    showTimesig->setChecked(staffType.genTimesig());
    showBarlines->setChecked(staffType.showBarlines());
    invisible->setChecked(staffType.invisible());
    staffGroupName->setText(staffType.translatedGroupName());

    longStaffName->setPlainText(TextBase::unEscape(staffType.longName()));
    shortStaffName->setPlainText(TextBase::unEscape(staffType.shortName()));
}

void EditStaff::updateInstrument()
{
    updateInterval(m_instrument.transpose());

    longName->setPlainText(m_instrument.nameAsPlainText());
    shortName->setPlainText(m_instrument.abbreviatureAsPlainText());
    instrumentNumber->setValue(m_instrument.number());

    const InstrumentLabel& instrLabel = m_instrument.instrumentLabel();
    showNumberLong->setChecked(instrLabel.showNumberLong());
    showNumberShort->setChecked(instrLabel.showNumberShort());
    showTranspositionLong->setChecked(instrLabel.showTranspositionLong());
    showTranspositionShort->setChecked(instrLabel.showTranspositionShort());
    transpositionLabel->setText(TextBase::unEscape(instrLabel.transposition()));

    useDefaultName->setChecked(!instrLabel.useCustomName());
    useCustomName->setChecked(instrLabel.useCustomName());
    customNameGroupbox->setEnabled(instrLabel.useCustomName());
    customLongName->setText(TextBase::unEscape(instrLabel.customNameLong()));
    customShortName->setText(TextBase::unEscape(instrLabel.customNameShort()));

    allowGroupName->setChecked(instrLabel.allowGroupName());
    useDefaultGroupName->setChecked(!instrLabel.useCustomGroupName());
    useCustomGroupName->setChecked(instrLabel.useCustomGroupName());
    customGroupNameGroupbox->setEnabled(instrLabel.useCustomGroupName());
    customLongNameGroup->setPlainText(TextBase::unEscape(instrLabel.customNameLongGroup()));
    customShortNameGroup->setPlainText(TextBase::unEscape(instrLabel.customNameShortGroup()));

    useDefaultIndividualName->setChecked(!instrLabel.useCustomIndividualName());
    useCustomIndividualName->setChecked(instrLabel.useCustomIndividualName());
    customIndividualNameGroupbox->setEnabled(instrLabel.useCustomIndividualName());
    customLongNameIndividual->setPlainText(TextBase::unEscape(instrLabel.customNameLongIndividual()));
    customShortNameIndividual->setPlainText(TextBase::unEscape(instrLabel.customNameShortIndividual()));

    const InstrumentTemplate* templ = mu::engraving::searchTemplate(m_instrument.id());
    if (templ) {
        instrumentName->setText(formatInstrumentTitle(templ->trackName, templ->trait));
    } else {
        instrumentName->setText(muse::qtrc("notation/editstaff", "Unknown"));
    }

    m_minPitchA = m_instrument.minPitchA();
    m_maxPitchA = m_instrument.maxPitchA();
    m_minPitchP = m_instrument.minPitchP();
    m_maxPitchP = m_instrument.maxPitchP();
    minPitchA->setText(midiCodeToStr(m_minPitchA));
    maxPitchA->setText(midiCodeToStr(m_maxPitchA));
    minPitchP->setText(midiCodeToStr(m_minPitchP));
    maxPitchP->setText(midiCodeToStr(m_maxPitchP));

    // only show string data controls if instrument has strings
    size_t numStr = m_instrument.stringData()->strings();
    stringDataFrame->setVisible(numStr > 0);
    numOfStrings->setText(QString::number(numStr));

    // show transp_PreferSharpFlat if instrument isn't non-transposing or octave-transposing
    bool showPreferSharpFlat = (iList->currentIndex() != 0) && (iList->currentIndex() != 25);
    transp_PreferSharpFlat->setVisible(showPreferSharpFlat);
    preferSharpFlat->setCurrentIndex(int(m_orgStaff->part()->preferSharpFlat()));
}

void EditStaff::updateInterval(const mu::engraving::Interval& iv)
{
    bool upFlag = !(iv.chromatic < 0 || iv.diatonic < 0);

    int chromatic = std::abs(iv.chromatic);
    int diatonic = std::abs(iv.diatonic);
    int oct = chromatic / 12;

    chromatic %= 12;
    diatonic %= 7;

    if (diatonic == 0 && chromatic == 11) {
        diatonic = 7;
    } else if (chromatic == 0 && diatonic == 6) {
        chromatic = 12;
        --oct;
    }

    size_t intervalIndex = muse::indexOf(Interval::allIntervals, Interval { diatonic, chromatic });
    IF_ASSERT_FAILED(intervalIndex != muse::nidx) {
        LOGD("EditStaff: unknown interval %d %d", diatonic, chromatic);
        intervalIndex = 0;
    }

    iList->setCurrentIndex(static_cast<int>(intervalIndex));
    up->setChecked(upFlag);
    down->setChecked(!upFlag);
    octave->setValue(oct);
}

void EditStaff::updateNextPreviousButtons()
{
    staff_idx_t staffIdx = m_orgStaff->idx();

    nextButton->setEnabled(staffIdx < (m_orgStaff->score()->nstaves() - 1));
    previousButton->setEnabled(staffIdx != 0);
}

void EditStaff::gotoNextStaff()
{
    apply();
    staff_idx_t nextStaffIndex = m_orgStaff->idx() + 1;
    Staff* nextStaff = m_orgStaff->score()->staff(nextStaffIndex);

    if (nextStaff) {
        setStaff(nextStaff, m_tick);
    }
}

void EditStaff::gotoPreviousStaff()
{
    apply();
    staff_idx_t previousStaffIndex = m_orgStaff->idx() - 1;
    Staff* prevStaff = m_orgStaff->score()->staff(previousStaffIndex);

    if (prevStaff) {
        setStaff(prevStaff, m_tick);
    }
}

void EditStaff::bboxClicked(QAbstractButton* button)
{
    QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
    switch (br) {
    case QDialogButtonBox::ApplyRole:
        apply();
        break;
    case QDialogButtonBox::AcceptRole:
        apply();
        done(1);
        break;
    case QDialogButtonBox::RejectRole:
        done(0);
        if (m_staff != nullptr) {
            delete m_staff;
        }
        break;

    default:
        LOGD("EditStaff: unknown button %d", int(br));
        break;
    }
}

void EditStaff::apply()
{
    size_t index = m_staff->score()->undoStack()->currentIndex();
    applyStaffProperties();
    applyPartProperties();
    m_staff->score()->undoStack()->mergeCommands(index);
}

void EditStaff::minPitchAClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.minPitchA());
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        minPitchA->setText(midiCodeToStr(newCode));
        m_minPitchA = newCode;
    }
}

void EditStaff::maxPitchAClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.maxPitchP());
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        maxPitchA->setText(midiCodeToStr(newCode));
        m_maxPitchA = newCode;
    }
}

void EditStaff::minPitchPClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.minPitchP());
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        minPitchP->setText(midiCodeToStr(newCode));
        m_minPitchP = newCode;
    }
}

void EditStaff::maxPitchPClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.maxPitchP());
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        maxPitchP->setText(midiCodeToStr(newCode));
        m_maxPitchP = newCode;
    }
}

void EditStaff::lineDistanceChanged()
{
    m_staff->staffType(m_tick)->setLineDistance(mu::engraving::Spatium(lineDistance->value()));
}

void EditStaff::numOfLinesChanged()
{
    m_staff->staffType(m_tick)->setLines(lines->value());
}

void EditStaff::showClefChanged()
{
    m_staff->staffType(m_tick)->setGenClef(showClef->isChecked());
}

void EditStaff::showTimeSigChanged()
{
    m_staff->staffType(m_tick)->setGenTimesig(showTimesig->isChecked());
}

void EditStaff::showBarlinesChanged()
{
    m_staff->staffType(m_tick)->setShowBarlines(showBarlines->isChecked());
}

void EditStaff::invisibleChanged()
{
    m_staff->staffType(m_tick)->setInvisible(invisible->isChecked());
}

void EditStaff::colorChanged()
{
    m_staff->staffType(m_tick)->setColor(color->color());
}

void EditStaff::magChanged(double newValue)
{
    m_staff->staffType(m_tick)->setUserMag(newValue / 100.0);
}

void EditStaff::transpositionChanged()
{
    // non-transposing or octave-transposing instrument
    // don't show transp_preferSharpFlat
    if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25)) {
        transp_PreferSharpFlat->setVisible(false);
    } else {
        transp_PreferSharpFlat->setVisible(true);
    }
}

void EditStaff::longNameChanged()
{
    m_staff->staffType(m_tick)->setLongName(longStaffName->toPlainText());
}

void EditStaff::shortNameChanged()
{
    m_staff->staffType(m_tick)->setShortName(shortStaffName->toPlainText());
}

void EditStaff::useCustomNameChanged(bool useCustom)
{
    useDefaultName->setChecked(!useCustom);
    useCustomName->setChecked(useCustom);
    customNameGroupbox->setEnabled(useCustom);
}

void EditStaff::useCustomGroupNameChanged(bool useCustomGroup)
{
    useDefaultGroupName->setChecked(!useCustomGroup);
    useCustomGroupName->setChecked(useCustomGroup);
    customGroupNameGroupbox->setEnabled(useCustomGroup);
}

void EditStaff::useCustomIndividualNameChanged(bool useCustomIndividual)
{
    useDefaultIndividualName->setChecked(!useCustomIndividual);
    useCustomIndividualName->setChecked(useCustomIndividual);
    customIndividualNameGroupbox->setEnabled(useCustomIndividual);
}

INotationPtr EditStaff::notation() const
{
    return globalContext()->currentNotation();
}

IMasterNotationPtr EditStaff::masterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPartsPtr EditStaff::notationParts() const
{
    return notation() ? notation()->parts() : nullptr;
}

INotationPartsPtr EditStaff::masterNotationParts() const
{
    return masterNotation() ? masterNotation()->parts() : nullptr;
}

void EditStaff::initStaff()
{
    const INotationPtr notation = this->notation();
    const INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    auto context = interaction ? interaction->hitElementContext() : INotationInteraction::HitElementContext();
    const EngravingItem* element = context.element;
    Staff* staff = context.staff;

    if (interaction && !element) {
        INotationSelectionPtr selection = interaction->selection();
        if (selection->isRange()) {
            INotationSelectionRangePtr range = selection->range();
            element = range->measureRange().endMeasure;
            staff = element->score()->staff(range->endStaffIndex() - 1);
        }
    }

    IF_ASSERT_FAILED(element) {
        return;
    }

    Fraction tick = { -1, 1 };
    if (element->isChordRest()) {
        tick = mu::engraving::toChordRest(element)->tick();
    } else if (element->isNote()) {
        tick = mu::engraving::toNote(element)->chord()->tick();
    } else if (element->isMeasure()) {
        tick = mu::engraving::toMeasure(element)->tick();
    } else if (element->isInstrumentName()) {
        const mu::engraving::System* system = mu::engraving::toSystem(mu::engraving::toInstrumentName(element)->explicitParent());
        const Measure* measure = system ? system->firstMeasure() : nullptr;
        staff = element->staff();

        if (measure) {
            tick = measure->tick();
        }
    }

    IF_ASSERT_FAILED(staff) {
        return;
    }

    setStaff(staff, tick);
}

Instrument EditStaff::instrument() const
{
    INotationPartsPtr notationParts = this->notationParts();
    if (!notationParts) {
        return Instrument();
    }

    const Part* part = notationParts->part(m_instrumentKey.partId);
    return part ? *part->instrument(m_instrumentKey.tick) : Instrument();
}

std::vector<InstrumentKey> EditStaff::otherInstrumentsInSameGroup() const
{
    Part* part = m_orgStaff->part();
    Score* score = m_orgStaff->score();

    Measure* measure = score->tick2measure(m_tick);
    IF_ASSERT_FAILED(measure) {
        return {};
    }

    System* system = measure->system();
    IF_ASSERT_FAILED(system) {
        return {};
    }

    const std::unordered_map<Part*, InstrumentName*>& partsWithGroupName = system->ldata()->partsWithGroupName();
    if (!muse::contains(partsWithGroupName, part)) {
        return {};
    }

    InstrumentName* groupName = partsWithGroupName.at(part);

    std::vector<InstrumentKey> result;
    for (const auto& pair : partsWithGroupName) {
        Part* p = pair.first;
        if (p == part) {
            continue;
        }

        InstrumentName* n = pair.second;
        if (n == groupName) {
            Instrument* instrument = p->instrument(m_tick);
            InstrumentKey key;
            key.instrumentId = instrument->id();
            key.partId = p->id();
            key.tick = Fraction::fromTicks(muse::findLessOrEqual(p->instruments(), m_tick.ticks())->first);
            result.push_back(key);
        }
    }

    return result;
}

void EditStaff::applyStaffProperties()
{
    StaffConfig config;
    config.visible = m_orgStaff->visible();

    config.userDistance = Spatium(spinExtraDistance->value());
    config.hideSystemBarline = hideSystemBarLine->isChecked();
    config.mergeMatchingRests = static_cast<AutoOnOff>(mergeMatchingRests->currentIndex());
    config.clefTypeList = m_staff->defaultClefType();
    config.staffType = *m_staff->staffType(m_tick);
    config.reflectTranspositionInLinkedTab = !noReflectTranspositionInLinkedTab->isChecked();

    notationParts()->setStaffConfig(m_orgStaff->id(), config, m_tick);
}

void EditStaff::applyPartProperties()
{
    Part* part    = m_orgStaff->part();

    String _sn = shortName->toPlainText();
    String _ln = longName->toPlainText();
    String _ssn = shortStaffName->toPlainText();
    String _lsn = longStaffName->toPlainText();
    if (!mu::engraving::Text::validateText(_sn) || !mu::engraving::Text::validateText(_ln)
        || !mu::engraving::Text::validateText(_ssn) || !mu::engraving::Text::validateText(_lsn)) {
        interactive()->warning(muse::trc("notation/staffpartproperties", "Invalid instrument name"),
                               muse::trc("notation/staffpartproperties", "The instrument name is invalid."));
        return;
    }

    QString sn = _sn;
    QString ln = _ln;
    QString ssn = _ssn;
    QString lsn = _lsn;
    shortName->setPlainText(sn);    // show the fixed text
    longName->setPlainText(ln);
    shortStaffName->setPlainText(ssn);
    longStaffName->setPlainText(lsn);

    int intervalIdx = iList->currentIndex();
    bool upFlag     = up->isChecked();

    mu::engraving::Interval interval = Interval::allIntervals[intervalIdx];
    interval.diatonic  += static_cast<int8_t>(octave->value() * 7);
    interval.chromatic += static_cast<int8_t>(octave->value() * 12);

    if (!upFlag) {
        interval.flip();
    }

    Instrument prevInstrument = m_instrument;

    m_instrument.setTranspose(interval);
    m_instrument.setMinPitchA(m_minPitchA);
    m_instrument.setMaxPitchA(m_maxPitchA);
    m_instrument.setMinPitchP(m_minPitchP);
    m_instrument.setMaxPitchP(m_maxPitchP);

    m_instrument.setShortName(String::fromQString(sn));
    m_instrument.setLongName(String::fromQString(ln));
    m_instrument.setNumber(instrumentNumber->value());

    InstrumentLabel& name = m_instrument.instrumentLabel();
    name.setShowNumberLong(showNumberLong->isChecked());
    name.setShowNumberShort(showNumberShort->isChecked());
    name.setShowTranspositionLong(showTranspositionLong->isChecked());
    name.setShowTranspositionShort(showTranspositionShort->isChecked());
    name.setTransposition(transpositionLabel->text());
    name.setUseCustomName(useCustomName->isChecked());
    name.setCustomNameLong(customLongName->toPlainText());
    name.setCustomNameShort(customShortName->toPlainText());
    name.setAllowGroupName(allowGroupName->isChecked());
    name.setUseCustomGroupName(useCustomGroupName->isChecked());
    name.setCustomNameLongGroup(customLongNameGroup->toPlainText());
    name.setCustomNameShortGroup(customShortNameGroup->toPlainText());
    name.setUseCustomIndividualName(useCustomIndividualName->isChecked());
    name.setCustomNameLongIndividual(customLongNameIndividual->toPlainText());
    name.setCustomNameShortIndividual(customShortNameIndividual->toPlainText());

    if (name.useCustomGroupName() && (name.customNameLongGroup().empty() || name.customNameShortGroup().empty())) {
        interactive()->warning(muse::trc("notation/staffpartproperties", "Invalid group name"),
                               muse::trc("notation/staffpartproperties", "Custom group names cannot be empty."));
        return;
    }

    size_t staffIdxInPart = muse::indexOf(part->staves(), m_orgStaff);
    DO_ASSERT(staffIdxInPart != muse::nidx);

    if (m_instrument.id() != prevInstrument.id()) {
        masterNotationParts()->replaceInstrument(m_instrumentKey, m_instrument);
    } else if (m_instrument != prevInstrument) {
        bool groupNameChanged = name.useCustomGroupName() != prevInstrument.instrumentLabel().useCustomGroupName()
                                || name.customNameLongGroup() != prevInstrument.instrumentLabel().customNameLongGroup()
                                || name.customNameShortGroup() != prevInstrument.instrumentLabel().customNameShortGroup();
        if (groupNameChanged) {
            notationParts()->setInstrumentGroupNameOptions(otherInstrumentsInSameGroup(),
                                                           name.useCustomGroupName(), name.customNameLongGroup(),
                                                           name.customNameShortGroup());
        }
        notationParts()->replaceInstrument(m_instrumentKey, m_instrument);
    }

    SharpFlat newSharpFlat = SharpFlat(preferSharpFlat->currentIndex());
    if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25)) {
        // instrument becomes non/octave-transposing, preferSharpFlat isn't useful anymore
        newSharpFlat = SharpFlat::NONE;
    }

    if (part->preferSharpFlat() != newSharpFlat) {
        notationParts()->setPartSharpFlat(m_instrumentKey.partId, newSharpFlat);
    }
}

void EditStaff::showReplaceInstrumentDialog()
{
    async::Promise<InstrumentTemplate> templ = selectInstrumentsScenario()->selectInstrument(m_instrumentKey);
    templ.onResolve(this, [this](const InstrumentTemplate& val) {
        const StaffType* staffType = val.staffTypePreset;
        if (!staffType) {
            staffType = StaffType::getDefaultPreset(StaffGroup::STANDARD);
        }

        m_instrument = Instrument::fromTemplate(&val);
        m_staff->setStaffType(m_tick, *staffType);

        updateInstrument();
        updateStaffType(*staffType);
    });
}

void EditStaff::editStringDataClicked()
{
    int frets = m_instrument.stringData()->frets();
    std::vector<mu::engraving::instrString> stringList = m_instrument.stringData()->stringList();

    EditStringData* esd = new EditStringData(this, stringList, frets);

    if (esd->exec()) {
        frets = esd->frets();
        stringList = esd->strings();

        mu::engraving::StringData stringData(frets, stringList);

        // update instrument pitch ranges as necessary
        if (stringList.size() > 0) {
            // get new string range bottom and top
            // as we have to choose an int size, INT16 are surely beyond midi pitch limits
            int oldHighestStringPitch     = INT16_MIN;
            int highestStringPitch        = INT16_MIN;
            int lowestStringPitch         = INT16_MAX;
            for (const mu::engraving::instrString& str : stringList) {
                if (str.pitch > highestStringPitch) {
                    highestStringPitch = str.pitch;
                }
                if (str.pitch < lowestStringPitch) {
                    lowestStringPitch  = str.pitch;
                }
            }
            // get old string range bottom
            for (const mu::engraving::instrString& str : m_instrument.stringData()->stringList()) {
                if (str.pitch > oldHighestStringPitch) {
                    oldHighestStringPitch = str.pitch;
                }
            }
            // if there were no string, arbitrarily set old top to maxPitchA
            if (oldHighestStringPitch == INT16_MIN) {
                oldHighestStringPitch = m_instrument.maxPitchA();
            }

            // range bottom is surely the pitch of the lowest string
            m_instrument.setMinPitchA(lowestStringPitch);
            m_instrument.setMinPitchP(lowestStringPitch);

            // range top should keep the same interval with the highest string it has now
            m_instrument.setMaxPitchA(m_instrument.maxPitchA() + highestStringPitch - oldHighestStringPitch);
            m_instrument.setMaxPitchP(m_instrument.maxPitchP() + highestStringPitch - oldHighestStringPitch);

            // update dlg controls
            minPitchA->setText(midiCodeToStr(m_instrument.minPitchA()));
            maxPitchA->setText(midiCodeToStr(m_instrument.maxPitchA()));
            minPitchP->setText(midiCodeToStr(m_instrument.minPitchP()));
            maxPitchP->setText(midiCodeToStr(m_instrument.maxPitchP()));
            // if no longer there is any string, leave everything as it is now
        }

        // update instrument data and dlg controls
        m_instrument.setStringData(stringData);
        numOfStrings->setText(QString::number(stringData.strings()));
    }
}

QString EditStaff::midiCodeToStr(int midiCode)
{
    return QString::fromStdString(muse::pitchToString(midiCode));
}

void EditStaff::showStaffTypeDialog()
{
    editStaffTypeDialog->setStaffType(m_staff->staffType(m_tick));
    editStaffTypeDialog->setInstrument(m_instrument);

    if (editStaffTypeDialog->exec()) {
        m_staff->setStaffType(m_tick, editStaffTypeDialog->getStaffType());
        updateStaffType(editStaffTypeDialog->getStaffType());
    }
}
