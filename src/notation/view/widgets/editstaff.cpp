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

#include "editstaff.h"

#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"
#include "libmscore/part.h"
#include "libmscore/masterscore.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/text.h"
#include "libmscore/utils.h"
#include "libmscore/undo.h"
#include "libmscore/iname.h"
#include "libmscore/system.h"

#include "translation.h"

#include "ui/view/iconcodes.h"
#include "widgetstatestore.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::ui;

static const QChar GO_UP_ICON = iconCodeToChar(IconCode::Code::ARROW_UP);
static const QChar GO_DOWN_ICON = iconCodeToChar(IconCode::Code::ARROW_DOWN);
static const QChar EDIT_ICON = iconCodeToChar(IconCode::Code::EDIT);

EditStaff::EditStaff(QWidget* parent)
    : QDialog(parent)
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

    connect(showClef,     &QCheckBox::clicked, this, &EditStaff::showClefChanged);
    connect(showTimesig,  &QCheckBox::clicked, this, &EditStaff::showTimeSigChanged);
    connect(showBarlines, &QCheckBox::clicked, this, &EditStaff::showBarlinesChanged);
    connect(invisible,    &QCheckBox::clicked, this, &EditStaff::invisibleChanged);

    connect(iList, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditStaff::transpositionChanged);

    connect(lines, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &EditStaff::numOfLinesChanged);
    connect(lineDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditStaff::lineDistanceChanged);

    nextButton->setText(GO_DOWN_ICON);
    previousButton->setText(GO_UP_ICON);
    minPitchASelect->setText(EDIT_ICON);
    maxPitchASelect->setText(EDIT_ICON);
    minPitchPSelect->setText(EDIT_ICON);
    maxPitchPSelect->setText(EDIT_ICON);

    WidgetStateStore::restoreGeometry(this);
}

EditStaff::EditStaff(const EditStaff& other)
    : QDialog(other.parentWidget())
{
}

int EditStaff::metaTypeId()
{
    return QMetaType::type("EditStaff");
}

void EditStaff::setStaff(Staff* s, const Fraction& tick)
{
    if (m_staff != nullptr) {
        delete m_staff;
    }

    m_orgStaff = s;

    Part* part = m_orgStaff->part();
    Ms::Score* score = part->score();

    m_instrument = *part->instrument(tick);
    m_orgInstrument = m_instrument;

    m_instrumentKey.instrumentId = m_instrument.id();
    m_instrumentKey.partId = part->id();
    m_instrumentKey.tick = tick;

    m_staff = new Ms::Staff(score);
    Ms::StaffType* stt = m_staff->setStaffType(Fraction(0, 1), *m_orgStaff->staffType(Fraction(0, 1)));
    stt->setInvisible(m_orgStaff->staffType(Fraction(0, 1))->invisible());
    stt->setColor(m_orgStaff->staffType(Fraction(0, 1))->color());
    stt->setUserMag(m_orgStaff->staffType(Fraction(0, 1))->userMag());

    m_staff->setUserDist(m_orgStaff->userDist());
    m_staff->setPart(part);
    m_staff->setHideWhenEmpty(m_orgStaff->hideWhenEmpty());
    m_staff->setShowIfEmpty(m_orgStaff->showIfEmpty());
    m_staff->setHideSystemBarLine(m_orgStaff->hideSystemBarLine());
    m_staff->setMergeMatchingRests(m_orgStaff->mergeMatchingRests());

    // get tick range for instrument
    auto i = part->instruments()->upper_bound(tick.ticks());
    if (i == part->instruments()->end()) {
        m_tickEnd = Fraction(-1, 1);
    } else {
        m_tickEnd = Fraction::fromTicks(i->first);
    }
    --i;
    if (i == part->instruments()->begin()) {
        m_tickStart = Fraction(-1, 1);
    } else {
        m_tickStart = Fraction::fromTicks(i->first);
    }

    // set dlg controls
    spinExtraDistance->setValue(s->userDist() / score->spatium());
    invisible->setChecked(m_staff->isLinesInvisible(Fraction(0, 1)));
    isSmallCheckbox->setChecked(stt->isSmall());
    color->setColor(stt->color().toQColor());
    partName->setText(part->partName());
    cutaway->setChecked(m_staff->cutaway());

    hideMode->setCurrentIndex(int(m_staff->hideWhenEmpty()));
    showIfEmpty->setChecked(m_staff->showIfEmpty());
    hideSystemBarLine->setChecked(m_staff->hideSystemBarLine());
    mergeMatchingRests->setChecked(m_staff->mergeMatchingRests());
    mag->setValue(stt->userMag() * 100.0);

    updateStaffType(*m_staff->staffType(Ms::Fraction(0, 1)));
    updateInstrument();
    updateNextPreviousButtons();
}

void EditStaff::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

void EditStaff::updateStaffType(const Ms::StaffType& staffType)
{
    lines->setValue(staffType.lines());
    lineDistance->setValue(staffType.lineDistance().val());
    showClef->setChecked(staffType.genClef());
    showTimesig->setChecked(staffType.genTimesig());
    showBarlines->setChecked(staffType.showBarlines());
    invisible->setChecked(staffType.invisible());
    isSmallCheckbox->setChecked(staffType.isSmall());
    staffGroupName->setText(qtrc("Staff type group name", staffType.groupName()));
}

void EditStaff::updateInstrument()
{
    updateInterval(m_instrument.transpose());

    QList<Ms::StaffName>& snl = m_instrument.shortNames();
    QString df = snl.isEmpty() ? "" : snl[0].name();
    shortName->setPlainText(df);

    QList<Ms::StaffName>& lnl = m_instrument.longNames();
    df = lnl.isEmpty() ? "" : lnl[0].name();

    longName->setPlainText(df);

    if (partName->text() == instrumentName->text()) {
        // Updates part name if no custom name has been set before
        partName->setText(m_instrument.name());
    }

    instrumentName->setText(m_instrument.name());

    m_minPitchA = m_instrument.minPitchA();
    m_maxPitchA = m_instrument.maxPitchA();
    m_minPitchP = m_instrument.minPitchP();
    m_maxPitchP = m_instrument.maxPitchP();
    minPitchA->setText(midiCodeToStr(m_minPitchA));
    maxPitchA->setText(midiCodeToStr(m_maxPitchA));
    minPitchP->setText(midiCodeToStr(m_minPitchP));
    maxPitchP->setText(midiCodeToStr(m_maxPitchP));
    singleNoteDynamics->setChecked(m_instrument.singleNoteDynamics());

    // only show string data controls if instrument has strings
    int numStr = m_instrument.stringData()->strings();
    stringDataFrame->setVisible(numStr > 0);
    numOfStrings->setText(QString::number(numStr));

    // show transp_PreferSharpFlat if instrument isn't non-transposing or octave-transposing
    bool showPreferSharpFlat = (iList->currentIndex() != 0) && (iList->currentIndex() != 25);
    transp_PreferSharpFlat->setVisible(showPreferSharpFlat);
    preferSharpFlat->setCurrentIndex(int(m_orgStaff->part()->preferSharpFlat()));
}

void EditStaff::updateInterval(const Ms::Interval& iv)
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

    int interval = Ms::searchInterval(diatonic, chromatic);
    if (interval == -1) {
        qDebug("EditStaff: unknown interval %d %d", diatonic, chromatic);
        interval = 0;
    }
    iList->setCurrentIndex(interval);
    up->setChecked(upFlag);
    down->setChecked(!upFlag);
    octave->setValue(oct);
}

void EditStaff::updateNextPreviousButtons()
{
    int staffIdx = m_orgStaff->idx();

    nextButton->setEnabled(staffIdx < (m_orgStaff->score()->nstaves() - 1));
    previousButton->setEnabled(staffIdx != 0);
}

void EditStaff::gotoNextStaff()
{
    int nextStaffIndex = m_orgStaff->idx() + 1;
    Staff* nextStaff = m_orgStaff->score()->staff(nextStaffIndex);

    if (nextStaff) {
        setStaff(nextStaff, m_tickStart);
    }
}

void EditStaff::gotoPreviousStaff()
{
    int previousStaffIndex = m_orgStaff->idx() - 1;
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
        qDebug("EditStaff: unknown button %d", int(br));
        break;
    }
}

void EditStaff::apply()
{
    int index = m_staff->score()->undoStack()->getCurIdx();
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
    m_staff->staffType(Fraction(0, 1))->setLineDistance(Ms::Spatium(lineDistance->value()));
}

void EditStaff::numOfLinesChanged()
{
    m_staff->staffType(Fraction(0, 1))->setLines(lines->value());
}

void EditStaff::showClefChanged()
{
    m_staff->staffType(Fraction(0, 1))->setGenClef(showClef->checkState() == Qt::Checked);
}

void EditStaff::showTimeSigChanged()
{
    m_staff->staffType(Fraction(0, 1))->setGenTimesig(showTimesig->checkState() == Qt::Checked);
}

void EditStaff::showBarlinesChanged()
{
    m_staff->staffType(Fraction(0, 1))->setShowBarlines(showBarlines->checkState() == Qt::Checked);
}

void EditStaff::invisibleChanged()
{
    m_staff->staffType(Fraction(0, 1))->setInvisible(invisible->checkState() == Qt::Checked);
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

INotationPartsPtr EditStaff::notationParts() const
{
    return notation() ? notation()->parts() : nullptr;
}

void EditStaff::initStaff()
{
    const INotationPtr notation = this->notation();
    const INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    auto context = interaction ? interaction->hitElementContext() : INotationInteraction::HitElementContext();
    const Element* element = Ms::toMeasure(context.element);

    if (!element || !context.staff) {
        return;
    }

    Fraction tick = { -1, 1 };
    if (element->isChordRest()) {
        tick = Ms::toChordRest(element)->tick();
    } else if (element->isNote()) {
        tick = Ms::toNote(element)->chord()->tick();
    } else if (element->isMeasure()) {
        tick = Ms::toMeasure(element)->tick();
    } else if (element->isInstrumentName()) {
        const Ms::System* system = Ms::toSystem(Ms::toInstrumentName(element)->parent());
        const Measure* measure = system ? system->firstMeasure() : nullptr;

        if (measure) {
            tick = measure->tick();
        }
    }

    setStaff(context.staff, tick);
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
    config.linesColor = color->color();
    config.visibleLines = invisible->isChecked();
    config.userDistance = spinExtraDistance->value() * m_orgStaff->score()->spatium();
    config.scale = mag->value() / 100.0;
    config.isSmall = isSmallCheckbox->isChecked();
    config.cutaway = cutaway->isChecked();
    config.showIfEmpty = showIfEmpty->isChecked();
    config.linesCount = lines->value();
    config.lineDistance = lineDistance->value();
    config.showClef = showClef->isChecked();
    config.showTimeSignature = showTimesig->isChecked();
    config.showKeySignature = editStaffTypeDialog->getStaffType().genKeysig();
    config.showBarlines = showBarlines->isChecked();
    config.showStemless = editStaffTypeDialog->getStaffType().stemless();
    config.showLedgerLinesPitched = editStaffTypeDialog->getStaffType().showLedgerLines();
    config.noteheadScheme = editStaffTypeDialog->getStaffType().noteHeadScheme();
    config.hideSystemBarline = hideSystemBarLine->isChecked();
    config.mergeMatchingRests = mergeMatchingRests->isChecked();
    config.hideMode = Staff::HideMode(hideMode->currentIndex());
    config.clefTypeList = m_instrument.clefType(m_orgStaff->rstaff());

    notationParts()->setStaffConfig(m_orgStaff->id(), config);
}

void EditStaff::applyPartProperties()
{
    Part* part    = m_orgStaff->part();

    QString sn = shortName->toPlainText();
    QString ln = longName->toPlainText();
    if (!Ms::Text::validateText(sn) || !Ms::Text::validateText(ln)) {
        interactive()->warning(trc("notation", "Invalid instrument name"),
                               trc("notation", "The instrument name is invalid."));
        return;
    }
    shortName->setPlainText(sn);    // show the fixed text
    longName->setPlainText(ln);

    int intervalIdx = iList->currentIndex();
    bool upFlag     = up->isChecked();

    Ms::Interval interval  = Ms::intervalList[intervalIdx];
    interval.diatonic  += octave->value() * 7;
    interval.chromatic += octave->value() * 12;

    if (!upFlag) {
        interval.flip();
    }

    m_instrument.setTranspose(interval);
    m_instrument.setMinPitchA(m_minPitchA);
    m_instrument.setMaxPitchA(m_maxPitchA);
    m_instrument.setMinPitchP(m_minPitchP);
    m_instrument.setMaxPitchP(m_maxPitchP);

    m_instrument.shortNames().clear();
    if (sn.length() > 0) {
        m_instrument.shortNames().append(Ms::StaffName(sn, 0));
    }

    m_instrument.longNames().clear();
    if (ln.length() > 0) {
        m_instrument.longNames().append(Ms::StaffName(ln, 0));
    }

    m_instrument.setSingleNoteDynamics(singleNoteDynamics->isChecked());

    QString newPartName = partName->text().simplified();

    Ms::Interval v1 = m_instrument.transpose();
    Ms::Interval v2 = m_orgInstrument.transpose();

    if (m_instrument != m_orgInstrument) {
        notationParts()->replaceInstrument(m_instrumentKey, m_instrument);
    }

    if (part->partName() != newPartName) {
        notationParts()->setPartName(m_instrumentKey.partId, newPartName);
    }

    bool preferSharpFlatChanged = (part->preferSharpFlat() != SharpFlat(preferSharpFlat->currentIndex()));
    // instrument becomes non/octave-transposing, preferSharpFlat isn't useful anymore
    if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25)) {
        notationParts()->setPartSharpFlat(m_instrumentKey.partId, SharpFlat::DEFAULT);
    } else {
        notationParts()->setPartSharpFlat(m_instrumentKey.partId, SharpFlat(preferSharpFlat->currentIndex()));
    }

    if (v1 != v2 || preferSharpFlatChanged) {
        notationParts()->setPartTransposition(m_instrumentKey.partId, v2);
    }
}

void EditStaff::showReplaceInstrumentDialog()
{
    RetVal<Instrument> selectedInstrument = selectInstrumentsScenario()->selectInstrument(m_instrumentKey);
    if (!selectedInstrument.ret) {
        LOGE() << selectedInstrument.ret.toString();
        return;
    }

    m_instrument = selectedInstrument.val;
    updateInstrument();
}

void EditStaff::editStringDataClicked()
{
    int frets = m_instrument.stringData()->frets();
    QList<Ms::instrString> stringList = m_instrument.stringData()->stringList();

    EditStringData* esd = new EditStringData(this, &stringList, &frets);
    esd->setWindowModality(Qt::WindowModal);
    if (esd->exec()) {
        Ms::StringData stringData(frets, stringList);

        // update instrument pitch ranges as necessary
        if (stringList.size() > 0) {
            // get new string range bottom and top
            // as we have to choose an int size, INT16 are surely beyond midi pitch limits
            int oldHighestStringPitch     = INT16_MIN;
            int highestStringPitch        = INT16_MIN;
            int lowestStringPitch         = INT16_MAX;
            for (const Ms::instrString& str : stringList) {
                if (str.pitch > highestStringPitch) {
                    highestStringPitch = str.pitch;
                }
                if (str.pitch < lowestStringPitch) {
                    lowestStringPitch  = str.pitch;
                }
            }
            // get old string range bottom
            for (const Ms::instrString& str : m_instrument.stringData()->stringList()) {
                if (str.pitch > oldHighestStringPitch) {
                    oldHighestStringPitch = str.pitch;
                }
            }
            // if there were no string, arbitrarely set old top to maxPitchA
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

static const char* s_es_noteNames[] = {
    QT_TRANSLATE_NOOP("editstaff", "C"),
    QT_TRANSLATE_NOOP("editstaff", "C♯"),
    QT_TRANSLATE_NOOP("editstaff", "D"),
    QT_TRANSLATE_NOOP("editstaff", "E♭"),
    QT_TRANSLATE_NOOP("editstaff", "E"),
    QT_TRANSLATE_NOOP("editstaff", "F"),
    QT_TRANSLATE_NOOP("editstaff", "F♯"),
    QT_TRANSLATE_NOOP("editstaff", "G"),
    QT_TRANSLATE_NOOP("editstaff", "A♭"),
    QT_TRANSLATE_NOOP("editstaff", "A"),
    QT_TRANSLATE_NOOP("editstaff", "B♭"),
    QT_TRANSLATE_NOOP("editstaff", "B")
};

QString EditStaff::midiCodeToStr(int midiCode)
{
    return QString("%1 %2").arg(qtrc("editstaff", s_es_noteNames[midiCode % 12])).arg(midiCode / 12 - 1);
}

void EditStaff::showStaffTypeDialog()
{
    editStaffTypeDialog->setStaffType(m_staff->staffType(Ms::Fraction(0, 1)));
    editStaffTypeDialog->setInstrument(m_instrument);

    if (editStaffTypeDialog->exec()) {
        m_staff->setStaffType(Fraction(0, 1), editStaffTypeDialog->getStaffType());
        updateStaffType(editStaffTypeDialog->getStaffType());
    }
}
