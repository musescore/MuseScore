#include "pianolevelsfilter.h"

#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

namespace Ms {
static const char* STRN_NOTE_ON_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Position");
static const char* STRN_NOTE_ON_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter",
                                                       "Move the selected note(s) forward or backward by thousandths of the full note duration");

static const char* STRN_LEN_MUL_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Duration (multiplier)");
static const char* STRN_LEN_MUL_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter",
                                                       "Multiply the duration by thousandths of the full note duration");

static const char* STRN_LEN_OFF_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Duration");
static const char* STRN_LEN_OFF_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter",
                                                       "Shorten or lengthen by thousandths of a whole note");

static const char* STRN_VEL_DYN_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Velocity (relative)");
static const char* STRN_VEL_DYN_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter",
                                                       "Increase or decrease the velocity by the specified value");

static const char* STRN_VEL_ABS_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Velocity (absolute)");
static const char* STRN_VEL_ABS_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter",
                                                       "Ignore dynamic markings and set the velocity directly");

PianoLevelsFilter* PianoLevelsFilter::FILTER_LIST[] = {
    new PianoLevelFilterLenWholenote,
    new PianoLevelFilterLenMultiplier,
    new PianoLevelFilterVeloOffset,
    new PianoLevelFilterVeloUser,
    new PianoLevelFilterOnTime,
    nullptr    //end of list indicator
};

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PianoLevelFilterOnTime::name()
{
    return qApp->translate("PianoLevelsFilter", STRN_NOTE_ON_NAME);
}

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterOnTime::tooltip()
{
    return qApp->translate("PianoLevelsFilter", STRN_NOTE_ON_TT);
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterOnTime::value(Staff* /*staff*/, Note* /*note*/, NoteEvent* evt)
{
    return evt->ontime();
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterOnTime::setValue(Staff* staff, Note* note, NoteEvent* evt, int value)
{
    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setOntime(value);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PianoLevelFilterLenMultiplier::name()
{
    return qApp->translate("PianoLevelsFilter", STRN_LEN_MUL_NAME);
}

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterLenMultiplier::tooltip()
{
    return qApp->translate("PianoLevelsFilter", STRN_LEN_MUL_TT);
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterLenMultiplier::value(Staff* /*staff*/, Note* /*note*/, NoteEvent* evt)
{
    return evt->len();
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterLenMultiplier::setValue(Staff* staff, Note* note, NoteEvent* evt, int value)
{
    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setLen(value);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PianoLevelFilterLenWholenote::name()
{
    return qApp->translate("PianoLevelsFilter", STRN_LEN_OFF_NAME);
}

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterLenWholenote::tooltip()
{
    return qApp->translate("PianoLevelsFilter", STRN_LEN_OFF_TT);
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterLenWholenote::value(Staff* /*staff*/, Note* note, NoteEvent* evt)
{
    Chord* chord = note->chord();
    Fraction noteLen = chord->ticks();
    int evtLen = evt->len();
    Fraction offsetLen = noteLen - (noteLen * evtLen / 1000);

    return -offsetLen.numerator() * 1000 / offsetLen.denominator();
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterLenWholenote::setValue(Staff* staff, Note* note, NoteEvent* evt, int value)
{
    Chord* chord = note->chord();
    Fraction noteLen = chord->ticks();
    Fraction cutLen(-value, 1000);
    Fraction playLen = noteLen - cutLen;
    Fraction evtLenFrac = playLen / noteLen;
    int evtLen = qMax(evtLenFrac.numerator() * 1000 / evtLenFrac.denominator(), 1);

    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setLen(evtLen);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PianoLevelFilterVeloOffset::name()
{
    return qApp->translate("PianoLevelsFilter", STRN_VEL_DYN_NAME);
}

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterVeloOffset::tooltip()
{
    return qApp->translate("PianoLevelsFilter", STRN_VEL_DYN_TT);
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterVeloOffset::value(Staff* staff, Note* note, NoteEvent* /*evt*/)
{
    //Change velocity to equivalent in new metric
    switch (Note::ValueType(note->veloType())) {
    case Note::ValueType::USER_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        return static_cast<int>((note->veloOffset() / (qreal)dynamicsVel - 1) * 100);
    }
    default:
    case Note::ValueType::OFFSET_VAL:
        return note->veloOffset();
    }
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterVeloOffset::setValue(Staff* staff, Note* note, NoteEvent* /*evt*/, int value)
{
    Score* score = staff->score();

    score->startCmd();

    switch (Note::ValueType(note->veloType())) {
    case Note::ValueType::USER_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        int newVelocity = static_cast<int>(dynamicsVel * (1 + value / 100.0));

        score->undo(new ChangeVelocity(note, Note::ValueType::USER_VAL, newVelocity));

        break;
    }
    default:
    case Note::ValueType::OFFSET_VAL: {
        score->undo(new ChangeVelocity(note, Note::ValueType::OFFSET_VAL, value));
        break;
    }
    }

    score->endCmd();
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PianoLevelFilterVeloUser::name()
{
    return qApp->translate("PianoLevelsFilter", STRN_VEL_ABS_NAME);
}

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterVeloUser::tooltip()
{
    return qApp->translate("PianoLevelsFilter", STRN_VEL_ABS_TT);
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterVeloUser::value(Staff* staff, Note* note, NoteEvent* /*evt*/)
{
    //Change velocity to equivalent in new metric
    switch (Note::ValueType(note->veloType())) {
    case Note::ValueType::USER_VAL:
        return note->veloOffset();
    default:
    case Note::ValueType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        return static_cast<int>(dynamicsVel * (1 + note->veloOffset() / 100.0));
    }
    }
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterVeloUser::setValue(Staff* staff, Note* note, NoteEvent* /*evt*/, int value)
{
    Score* score = staff->score();

    score->startCmd();

    switch (Note::ValueType(note->veloType())) {
    case Note::ValueType::USER_VAL:
        score->undo(new ChangeVelocity(note, Note::ValueType::USER_VAL, value));
        break;
    default:
    case Note::ValueType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        int newVelocity = static_cast<int>((value / (qreal)dynamicsVel - 1) * 100);

        score->undo(new ChangeVelocity(note, Note::ValueType::OFFSET_VAL, newVelocity));
        break;
    }
    }

    score->endCmd();
}
}
