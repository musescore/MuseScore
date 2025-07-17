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

#include "translation.h"
#include "global/utils.h"

#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetutils.h"

#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/part.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/text.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/instrumentname.h"
#include "engraving/dom/system.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace mu::engraving;

static const QChar GO_UP_ICON = iconCodeToChar(IconCode::Code::ARROW_UP);
static const QChar GO_DOWN_ICON = iconCodeToChar(IconCode::Code::ARROW_DOWN);
static const QChar EDIT_ICON = iconCodeToChar(IconCode::Code::EDIT);

EditStaff::EditStaff(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
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
    connect(isSmallCheckbox,  &QCheckBox::clicked, this, &EditStaff::isSmallChanged);

    connect(color, &Awl::ColorLabel::colorChanged, this, &EditStaff::colorChanged);

    connect(mag, &QDoubleSpinBox::valueChanged, this, &EditStaff::magChanged);

    connect(iList, &QComboBox::currentIndexChanged, this, &EditStaff::transpositionChanged);

    connect(lines, &QSpinBox::valueChanged, this, &EditStaff::numOfLinesChanged);
    connect(lineDistance, &QDoubleSpinBox::valueChanged, this, &EditStaff::lineDistanceChanged);

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
    mu::engraving::StaffType* stt = m_staff->setStaffType(Fraction(0, 1), *m_orgStaff->staffType(Fraction(0, 1)));

    m_staff->setUserDist(m_orgStaff->userDist());
    m_staff->setPart(part);
    m_staff->setCutaway(m_orgStaff->cutaway());
    m_staff->setHideWhenEmpty(m_orgStaff->hideWhenEmpty());
    m_staff->setShowIfEmpty(m_orgStaff->showIfEmpty());
    m_staff->setHideSystemBarLine(m_orgStaff->hideSystemBarLine());
    m_staff->setMergeMatchingRests(m_orgStaff->mergeMatchingRests());
    m_staff->setReflectTranspositionInLinkedTab(m_orgStaff->reflectTranspositionInLinkedTab());

    // get tick range for instrument
    auto i = part->instruments().upper_bound(tick.ticks());
    if (i == part->instruments().end()) {
        m_tickEnd = Fraction(-1, 1);
    } else {
        m_tickEnd = Fraction::fromTicks(i->first);
    }
    --i;
    if (i == part->instruments().begin()) {
        m_tickStart = Fraction(-1, 1);
    } else {
        m_tickStart = Fraction::fromTicks(i->first);
    }

    // set dlg controls
    spinExtraDistance->setValue(s->userDist().val());
    invisible->setChecked(stt->invisible());
    isSmallCheckbox->setChecked(stt->isSmall());
    color->setColor(stt->color().toQColor());
    mag->setValue(stt->userMag() * 100.0);

    cutaway->setChecked(m_staff->cutaway());
    hideMode->setCurrentIndex(int(m_staff->hideWhenEmpty()));
    showIfEmpty->setChecked(m_staff->showIfEmpty());
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
    isSmallCheckbox->setChecked(staffType.isSmall());
    staffGroupName->setText(staffType.translatedGroupName());
}

void EditStaff::updateInstrument()
{
    updateInterval(m_instrument.transpose());

    longName->setPlainText(m_instrument.nameAsPlainText());
    shortName->setPlainText(m_instrument.abbreviatureAsPlainText());
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
    int diatonic  = iv.diatonic;
    int chromatic = iv.chromatic;

    int oct = chromatic / 12;
    if (oct < 0) {
        oct = -oct;
    }

    bool upFlag = true;
    if (chromatic < 0 || diatonic < 0) {
        upFlag    = false;
        chromatic = -chromatic;
        diatonic  = -diatonic;
    }
    chromatic %= 12;
    diatonic  %= 7;

    int interval = mu::engraving::searchInterval(diatonic, chromatic);
    if (interval == -1) {
        LOGD("EditStaff: unknown interval %d %d", diatonic, chromatic);
        interval = 0;
    }
    iList->setCurrentIndex(interval);
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
        setStaff(nextStaff, m_tickStart);
    }
}

void EditStaff::gotoPreviousStaff()
{
    apply();
    staff_idx_t previousStaffIndex = m_orgStaff->idx() - 1;
    Staff* prevStaff = m_orgStaff->score()->staff(previousStaffIndex);

    if (prevStaff) {
        setStaff(prevStaff, m_tickStart);
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
    m_staff->staffType(Fraction(0, 1))->setLineDistance(mu::engraving::Spatium(lineDistance->value()));
}

void EditStaff::numOfLinesChanged()
{
    m_staff->staffType(Fraction(0, 1))->setLines(lines->value());
}

void EditStaff::showClefChanged()
{
    m_staff->staffType(Fraction(0, 1))->setGenClef(showClef->isChecked());
}

void EditStaff::showTimeSigChanged()
{
    m_staff->staffType(Fraction(0, 1))->setGenTimesig(showTimesig->isChecked());
}

void EditStaff::showBarlinesChanged()
{
    m_staff->staffType(Fraction(0, 1))->setShowBarlines(showBarlines->isChecked());
}

void EditStaff::invisibleChanged()
{
    m_staff->staffType(Fraction(0, 1))->setInvisible(invisible->isChecked());
}

void EditStaff::isSmallChanged()
{
    m_staff->staffType(Fraction(0, 1))->setSmall(isSmallCheckbox->isChecked());
}

void EditStaff::colorChanged()
{
    m_staff->staffType(Fraction(0, 1))->setColor(color->color());
}

void EditStaff::magChanged(double newValue)
{
    m_staff->staffType(Fraction(0, 1))->setUserMag(newValue / 100.0);
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

void EditStaff::applyStaffProperties()
{
    StaffConfig config;
    config.visible = m_orgStaff->visible();

    config.userDistance = Spatium(spinExtraDistance->value());
    config.cutaway = cutaway->isChecked();
    config.showIfEmpty = showIfEmpty->isChecked();
    config.hideSystemBarline = hideSystemBarLine->isChecked();
    config.mergeMatchingRests = static_cast<AutoOnOff>(mergeMatchingRests->currentIndex());
    config.hideMode = Staff::HideMode(hideMode->currentIndex());
    config.clefTypeList = m_instrument.clefType(m_orgStaff->rstaff());
    config.staffType = *m_staff->staffType(mu::engraving::Fraction(0, 1));
    config.reflectTranspositionInLinkedTab = !noReflectTranspositionInLinkedTab->isChecked();

    notationParts()->setStaffConfig(m_orgStaff->id(), config);
}

void EditStaff::applyPartProperties()
{
    Part* part    = m_orgStaff->part();

    String _sn = shortName->toPlainText();
    String _ln = longName->toPlainText();
    if (!mu::engraving::Text::validateText(_sn) || !mu::engraving::Text::validateText(_ln)) {
        interactive()->warning(muse::trc("notation/staffpartproperties", "Invalid instrument name"),
                               muse::trc("notation/staffpartproperties", "The instrument name is invalid."));
        return;
    }
    QString sn = _sn;
    QString ln = _ln;
    shortName->setPlainText(sn);    // show the fixed text
    longName->setPlainText(ln);

    int intervalIdx = iList->currentIndex();
    bool upFlag     = up->isChecked();

    mu::engraving::Interval interval  = mu::engraving::intervalList[intervalIdx];
    interval.diatonic  += static_cast<int8_t>(octave->value() * 7);
    interval.chromatic += static_cast<int8_t>(octave->value() * 12);

    if (!upFlag) {
        interval.flip();
    }

    m_instrument.setTranspose(interval);
    m_instrument.setMinPitchA(m_minPitchA);
    m_instrument.setMaxPitchA(m_maxPitchA);
    m_instrument.setMinPitchP(m_minPitchP);
    m_instrument.setMaxPitchP(m_maxPitchP);

    StaffNameList shortNames;
    if (sn.length() > 0) {
        shortNames.push_back(mu::engraving::StaffName(sn, 0));
    }
    m_instrument.setShortNames(shortNames);

    StaffNameList longNames;
    if (ln.length() > 0) {
        longNames.push_back(mu::engraving::StaffName(ln, 0));
    }
    m_instrument.setLongNames(longNames);

    if (m_instrument.id() != m_orgInstrument.id()) {
        masterNotationParts()->replaceInstrument(m_instrumentKey, m_instrument);
    } else {
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
        m_staff->setStaffType(Fraction(0, 1), *staffType);

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
    editStaffTypeDialog->setStaffType(m_staff->staffType(mu::engraving::Fraction(0, 1)));
    editStaffTypeDialog->setInstrument(m_instrument);

    if (editStaffTypeDialog->exec()) {
        m_staff->setStaffType(Fraction(0, 1), editStaffTypeDialog->getStaffType());
        updateStaffType(editStaffTypeDialog->getStaffType());
    }
}
