//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include "editstaff.h"

#include <QMessageBox>

#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/text.h"
#include "libmscore/utils.h"

#include "log.h"

#include "ui/view/iconcodes.h"
#include "widgetstatestore.h"

using namespace mu::notation;
using namespace mu::framework;

static const QChar GO_UP_ICON = iconCodeToChar(IconCode::Code::ARROW_UP);
static const QChar GO_DOWN_ICON = iconCodeToChar(IconCode::Code::ARROW_DOWN);
static const QChar EDIT_ICON = iconCodeToChar(IconCode::Code::EDIT);

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("EditStaff");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    editStaffTypeDialog = new EditStaffType(this);
    editStaffTypeDialog->setWindowModality(Qt::WindowModal);

    WidgetStateStore::restoreGeometry(this);

    connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
    connect(changeInstrument,     SIGNAL(clicked()),            SLOT(showInstrumentDialog()));
    connect(changeStaffType,      SIGNAL(clicked()),            SLOT(showStaffTypeDialog()));
    connect(minPitchASelect,      SIGNAL(clicked()),            SLOT(minPitchAClicked()));
    connect(maxPitchASelect,      SIGNAL(clicked()),            SLOT(maxPitchAClicked()));
    connect(minPitchPSelect,      SIGNAL(clicked()),            SLOT(minPitchPClicked()));
    connect(maxPitchPSelect,      SIGNAL(clicked()),            SLOT(maxPitchPClicked()));
    connect(editStringData,       SIGNAL(clicked()),            SLOT(editStringDataClicked()));
    connect(lines,                SIGNAL(valueChanged(int)),    SLOT(numOfLinesChanged()));
    connect(lineDistance,         SIGNAL(valueChanged(double)), SLOT(lineDistanceChanged()));
    connect(showClef,             SIGNAL(clicked()),            SLOT(showClefChanged()));
    connect(showTimesig,          SIGNAL(clicked()),            SLOT(showTimeSigChanged()));
    connect(showBarlines,         SIGNAL(clicked()),            SLOT(showBarlinesChanged()));

    connect(nextButton,           SIGNAL(clicked()),            SLOT(gotoNextStaff()));
    connect(previousButton,       SIGNAL(clicked()),            SLOT(gotoPreviousStaff()));

    connect(iList,                SIGNAL(currentIndexChanged(int)),  SLOT(transpositionChanged()));

    nextButton->setText(GO_DOWN_ICON);
    previousButton->setText(GO_UP_ICON);
    minPitchASelect->setText(EDIT_ICON);
    maxPitchASelect->setText(EDIT_ICON);
    minPitchPSelect->setText(EDIT_ICON);
    maxPitchPSelect->setText(EDIT_ICON);
}

EditStaff::EditStaff(const EditStaff& other)
    : QDialog(other.parentWidget())
{
}

int EditStaff::metaTypeId()
{
    return QMetaType::type("EditStaff");
}

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void EditStaff::setStaff(Staff* s, const Fraction& tick)
{
    if (m_staff != nullptr) {
        delete m_staff;
    }

    m_orgStaff = s;
    Part* part        = m_orgStaff->part();
    m_instrument      = instrument();
    m_orgInstrument = m_instrument;
    Ms::Score* score      = part->score();
    m_staff             = new Ms::Staff(score);
    Ms::StaffType* stt = m_staff->setStaffType(Fraction(0,1), *m_orgStaff->staffType(Fraction(0,1)));
    m_staff->setInvisible(m_orgStaff->invisible());
    m_staff->setUserDist(m_orgStaff->userDist());
    m_staff->setColor(m_orgStaff->color());
    m_staff->setPart(part);
    m_staff->setHideWhenEmpty(m_orgStaff->hideWhenEmpty());
    m_staff->setShowIfEmpty(m_orgStaff->showIfEmpty());
    stt->setUserMag(m_orgStaff->staffType(Fraction(0,1))->userMag());
    m_staff->setHideSystemBarLine(m_orgStaff->hideSystemBarLine());
    m_staff->setMergeMatchingRests(m_orgStaff->mergeMatchingRests());

    // get tick range for instrument
    auto i = part->instruments()->upper_bound(tick.ticks());
    if (i == part->instruments()->end()) {
        m_tickEnd = Fraction(-1,1);
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
    invisible->setChecked(m_staff->invisible());
    color->setColor(s->color());
    partName->setText(part->partName());
    hideMode->setCurrentIndex(int(m_staff->hideWhenEmpty()));
    showIfEmpty->setChecked(m_staff->showIfEmpty());
    hideSystemBarLine->setChecked(m_staff->hideSystemBarLine());
    mergeMatchingRests->setChecked(m_staff->mergeMatchingRests());
    mag->setValue(stt->userMag() * 100.0);
    updateStaffType(*m_staff->staffType(Ms::Fraction(0, 1)));
    updateInstrument();
    updateNextPreviousButtons();
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStaff::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

//---------------------------------------------------------
//   updateStaffType
//---------------------------------------------------------

void EditStaff::updateStaffType(const Ms::StaffType& staffType)
{
    lines->setValue(staffType.lines());
    lineDistance->setValue(staffType.lineDistance().val());
    showClef->setChecked(staffType.genClef());
    showTimesig->setChecked(staffType.genTimesig());
    showBarlines->setChecked(staffType.showBarlines());
    staffGroupName->setText(qApp->translate("Staff type group name", staffType.groupName()));
}

//---------------------------------------------------------
//   updateInstrument
//---------------------------------------------------------

void EditStaff::updateInstrument()
{
    updateInterval(m_instrument.transpose);

    QList<Ms::StaffName>& snl = m_instrument.shortNames;
    QString df = snl.isEmpty() ? "" : snl[0].name();
    shortName->setPlainText(df);

    QList<Ms::StaffName>& lnl = m_instrument.longNames;
    df = lnl.isEmpty() ? "" : lnl[0].name();

    longName->setPlainText(df);

    if (partName->text() == instrumentName->text()) {    // Updates part name if no custom name has been set before
        partName->setText(m_instrument.name);
    }

    instrumentName->setText(m_instrument.name);

    m_minPitchA = m_instrument.amateurPitchRange.min;
    m_maxPitchA = m_instrument.amateurPitchRange.max;
    m_minPitchP = m_instrument.professionalPitchRange.min;
    m_maxPitchP = m_instrument.professionalPitchRange.max;
    minPitchA->setText(midiCodeToStr(m_minPitchA));
    maxPitchA->setText(midiCodeToStr(m_maxPitchA));
    minPitchP->setText(midiCodeToStr(m_minPitchP));
    maxPitchP->setText(midiCodeToStr(m_maxPitchP));
    singleNoteDynamics->setChecked(m_instrument.singleNoteDynamics);

    // only show string data controls if instrument has strings
    int numStr = m_instrument.stringData.strings();
    stringDataFrame->setVisible(numStr > 0);
    numOfStrings->setText(QString::number(numStr));

    // show transp_PreferSharpFlat if instrument isn't non-transposing or octave-transposing
    bool showPreferSharpFlat = (iList->currentIndex() != 0) && (iList->currentIndex() != 25);
    transp_PreferSharpFlat->setVisible(showPreferSharpFlat);
    preferSharpFlat->setCurrentIndex(int(m_orgStaff->part()->preferSharpFlat()));
}

//---------------------------------------------------------
//   updateInterval
//---------------------------------------------------------

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

//---------------------------------------------------------
//   updateNextPreviousButtons
//---------------------------------------------------------

void EditStaff::updateNextPreviousButtons()
{
    int staffIdx = m_orgStaff->idx();

    nextButton->setEnabled(staffIdx < (m_orgStaff->score()->nstaves() - 1));
    previousButton->setEnabled(staffIdx != 0);
}

//---------------------------------------------------------
//   gotoNextStaff
//---------------------------------------------------------

void EditStaff::gotoNextStaff()
{
    int nextStaffIndex = m_orgStaff->idx() + 1;
    Staff* nextStaff = m_orgStaff->score()->staff(nextStaffIndex);
    if (nextStaff) {
        m_staffIdx = nextStaffIndex;
        updateCurrentStaff();
    }
}

//---------------------------------------------------------
//   gotoPreviousStaff
//---------------------------------------------------------

void EditStaff::gotoPreviousStaff()
{
    int previousStaffIndex = m_orgStaff->idx() - 1;
    Staff* prevStaff = m_orgStaff->score()->staff(previousStaffIndex);
    if (prevStaff) {
        m_staffIdx = previousStaffIndex;
        updateCurrentStaff();
    }
}

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

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

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStaff::apply()
{
    applyStaffProperties();
    applyPartProperties();
}

//---------------------------------------------------------
//   Slots
//---------------------------------------------------------

void EditStaff::minPitchAClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.amateurPitchRange.min);
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        minPitchA->setText(midiCodeToStr(newCode));
        m_minPitchA = newCode;
    }
}

void EditStaff::maxPitchAClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.amateurPitchRange.max);
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        maxPitchA->setText(midiCodeToStr(newCode));
        m_maxPitchA = newCode;
    }
}

void EditStaff::minPitchPClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.professionalPitchRange.min);
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        minPitchP->setText(midiCodeToStr(newCode));
        m_minPitchP = newCode;
    }
}

void EditStaff::maxPitchPClicked()
{
    int newCode;
    EditPitch ep(this, m_instrument.professionalPitchRange.max);
    ep.setWindowModality(Qt::WindowModal);
    if ((newCode = ep.exec()) != -1) {
        maxPitchP->setText(midiCodeToStr(newCode));
        m_maxPitchP = newCode;
    }
}

void EditStaff::lineDistanceChanged()
{
    m_staff->staffType(Fraction(0,1))->setLineDistance(Ms::Spatium(lineDistance->value()));
}

void EditStaff::numOfLinesChanged()
{
    m_staff->staffType(Fraction(0,1))->setLines(lines->value());
}

void EditStaff::showClefChanged()
{
    m_staff->staffType(Fraction(0,1))->setGenClef(showClef->checkState() == Qt::Checked);
}

void EditStaff::showTimeSigChanged()
{
    m_staff->staffType(Fraction(0,1))->setGenTimesig(showTimesig->checkState() == Qt::Checked);
}

void EditStaff::showBarlinesChanged()
{
    m_staff->staffType(Fraction(0,1))->setShowBarlines(showBarlines->checkState() == Qt::Checked);
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

void EditStaff::setStaffIdx(int staffIdx)
{
    if (m_staffIdx == staffIdx) {
        return;
    }

    m_staffIdx = staffIdx;

    updateCurrentStaff();

    emit staffIdxChanged(m_staffIdx);
}

INotationPartsPtr EditStaff::notationParts() const
{
    return globalContext()->currentNotation()->parts();
}

int EditStaff::staffIdx() const
{
    return m_staffIdx;
}

void EditStaff::updateCurrentStaff()
{
    Staff* staff = this->staff(m_staffIdx);

    if (staff) {
        m_partId = staff->part()->id();
        m_instrumentId = staff->part()->instrumentId();
        setStaff(staff, staff->tick());
    }
}

Staff* EditStaff::staff(int staffIndex) const
{
    INotationPartsPtr notationParts = this->notationParts();
    if (!notationParts) {
        return nullptr;
    }

    async::NotifyList<const Part*> parts = notationParts->partList();
    for (const Part* part: parts) {
        async::NotifyList<mu::instruments::Instrument> instruments = notationParts->instrumentList(part->id());
        for (mu::instruments::Instrument instrument: instruments) {
            async::NotifyList<const Staff*> staves = notationParts->staffList(part->id(), instrument.id);
            for (const Staff* staff: staves) {
                if (staff->idx() == staffIndex) {
                    return const_cast<Staff*>(staff);
                }
            }
        }
    }

    return nullptr;
}

mu::instruments::Instrument EditStaff::instrument() const
{
    INotationPartsPtr notationParts = this->notationParts();
    if (!notationParts) {
        return mu::instruments::Instrument();
    }

    async::NotifyList<const Part*> parts = notationParts->partList();
    for (const Part* part: parts) {
        if (part->id() == m_partId) {
            async::NotifyList<mu::instruments::Instrument> instruments = notationParts->instrumentList(part->id());
            for (mu::instruments::Instrument instrument: instruments) {
                if (instrument.id == m_instrumentId) {
                    return instrument;
                }
            }
        }
    }

    return mu::instruments::Instrument();
}

void EditStaff::applyStaffProperties()
{
    StaffConfig config;
    config.linesColor = color->color();
    config.visibleLines = invisible->isChecked();
    config.userDistance = spinExtraDistance->value() * m_orgStaff->score()->spatium();
    config.scale = mag->value() / 100.0;
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
    config.clefType = m_instrument.clefs[m_orgStaff->rstaff()];

    notationParts()->setStaffConfig(m_orgStaff->id(), config);

    // TODO
    //    notationParts()->setStaffType(m_staff->id(), *(m_staff->staffType(Fraction(0, 1))));
}

void EditStaff::applyPartProperties()
{
    Part* part    = m_orgStaff->part();

    QString sn = shortName->toPlainText();
    QString ln = longName->toPlainText();
    if (!Ms::Text::validateText(sn) || !Ms::Text::validateText(ln)) {
        QMessageBox msgBox;
        msgBox.setText(tr("The instrument name is invalid."));
        msgBox.exec();
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

    m_instrument.transpose = interval;
    m_instrument.amateurPitchRange.min = m_minPitchA;
    m_instrument.amateurPitchRange.max = m_maxPitchA;
    m_instrument.professionalPitchRange.min = m_minPitchP;
    m_instrument.professionalPitchRange.max = m_maxPitchP;

    m_instrument.shortNames.clear();
    if (sn.length() > 0) {
        m_instrument.shortNames.append(Ms::StaffName(sn, 0));
    }

    m_instrument.longNames.clear();
    if (ln.length() > 0) {
        m_instrument.longNames.append(Ms::StaffName(ln, 0));
    }

    m_instrument.singleNoteDynamics = singleNoteDynamics->isChecked();

    QString newPartName = partName->text().simplified();

    Ms::Interval v1 = m_instrument.transpose;
    Ms::Interval v2 = m_orgInstrument.transpose;

    if (isInstrumentChanged()) {
        notationParts()->replaceInstrument(m_instrumentId, m_partId, m_instrument);
    }

    if (part->partName() != newPartName) {
        notationParts()->setPartName(m_partId, newPartName);
    }

    bool preferSharpFlatChanged = (part->preferSharpFlat() != SharpFlat(preferSharpFlat->currentIndex()));
    // instrument becomes non/octave-transposing, preferSharpFlat isn't useful anymore
    if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25)) {
        notationParts()->setPartSharpFlat(m_partId, SharpFlat::DEFAULT);
    } else {
        notationParts()->setPartSharpFlat(m_partId, SharpFlat(preferSharpFlat->currentIndex()));
    }

    if (v1 != v2 || preferSharpFlatChanged) {
        notationParts()->setPartTransposition(m_partId, v2);
    }
}

bool EditStaff::isInstrumentChanged()
{
    return m_instrument.name != m_orgInstrument.name
           || m_instrument.transpose != m_orgInstrument.transpose
           || m_instrument.amateurPitchRange != m_orgInstrument.amateurPitchRange
           || m_instrument.professionalPitchRange != m_orgInstrument.professionalPitchRange
           || m_instrument.shortNames != m_orgInstrument.shortNames
           || m_instrument.longNames != m_orgInstrument.longNames
           || m_instrument.singleNoteDynamics != m_orgInstrument.singleNoteDynamics;
}

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
{
    //!TODO: implement after merge  the select instrument dialogue to master branch
    NOT_IMPLEMENTED;
}

//---------------------------------------------------------
//   editStringDataClicked
//---------------------------------------------------------

void EditStaff::editStringDataClicked()
{
    int frets = m_instrument.stringData.frets();
    QList<Ms::instrString> stringList = m_instrument.stringData.stringList();

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
            for (const Ms::instrString& str : m_instrument.stringData.stringList()) {
                if (str.pitch > oldHighestStringPitch) {
                    oldHighestStringPitch = str.pitch;
                }
            }
            // if there were no string, arbitrarely set old top to maxPitchA
            if (oldHighestStringPitch == INT16_MIN) {
                oldHighestStringPitch = m_instrument.amateurPitchRange.max;
            }

            // range bottom is surely the pitch of the lowest string
            m_instrument.amateurPitchRange.min = lowestStringPitch;
            m_instrument.professionalPitchRange.min = lowestStringPitch;
            // range top should keep the same interval with the highest string it has now
            m_instrument.amateurPitchRange.max = m_instrument.amateurPitchRange.max + highestStringPitch - oldHighestStringPitch;
            m_instrument.professionalPitchRange.max = m_instrument.professionalPitchRange.max + highestStringPitch - oldHighestStringPitch;
            // update dlg controls
            minPitchA->setText(midiCodeToStr(m_instrument.amateurPitchRange.min));
            maxPitchA->setText(midiCodeToStr(m_instrument.amateurPitchRange.max));
            minPitchP->setText(midiCodeToStr(m_instrument.professionalPitchRange.min));
            maxPitchP->setText(midiCodeToStr(m_instrument.professionalPitchRange.max));
            // if no longer there is any string, leave everything as it is now
        }

        // update instrument data and dlg controls
        m_instrument.stringData = stringData;
        numOfStrings->setText(QString::number(stringData.strings()));
    }
}

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------

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
    return QString("%1 %2").arg(qApp->translate("editstaff", s_es_noteNames[midiCode % 12])).arg(midiCode / 12 - 1);
}

//---------------------------------------------------------
//   showStaffTypeDialog
//---------------------------------------------------------

void EditStaff::showStaffTypeDialog()
{
    editStaffTypeDialog->setStaffType(m_staff->staffType(Ms::Fraction(0, 1)));
    editStaffTypeDialog->setInstrument(m_instrument);

    if (editStaffTypeDialog->exec()) {
        m_staff->setStaffType(Fraction(0,1), editStaffTypeDialog->getStaffType());
        updateStaffType(editStaffTypeDialog->getStaffType());
    }
}
