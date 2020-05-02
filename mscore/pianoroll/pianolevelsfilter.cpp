#include "pianolevelsfilter.h"

#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"


namespace Ms {


static const char* STRN_NOTE_ON_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Note start time");
static const char* STRN_NOTE_ON_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Add (or subtract) a bit to the start time of a note");

static const char* STRN_LEN_MUL_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Length (multiplier)");
static const char* STRN_LEN_MUL_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Multiply the note length by a bit");

static const char* STRN_LEN_OFF_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Length (offset)");
static const char* STRN_LEN_OFF_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Add (or subtract) a bit from the end of the note");

static const char* STRN_VEL_DYN_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Velocity (relative)");
static const char* STRN_VEL_DYN_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Raise or lower loudness relative to current dynamics value");

static const char* STRN_VEL_ABS_NAME = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Velocity (absolute)");
static const char* STRN_VEL_ABS_TT = QT_TRANSLATE_NOOP("PianoLevelsFilter", "Ignore dynamic markings and use this as the MIDI output value");


PianoLevelsFilter* PianoLevelsFilter::FILTER_LIST[] = {
      new PianoLevelFilterLen,
      new PianoLevelFilterLenOfftime,
      new PianoLevelFilterVeloOffset,
      new PianoLevelFilterVeloUser,
      new PianoLevelFilterOnTime,
      0  //end of list indicator
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

QString PianoLevelFilterLen::name()
      {
      return qApp->translate("PianoLevelsFilter", STRN_LEN_MUL_NAME);
      }

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterLen::tooltip()
      {
      return qApp->translate("PianoLevelsFilter", STRN_LEN_MUL_TT);
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterLen::value(Staff* /*staff*/, Note* /*note*/, NoteEvent* evt)
      {
      return evt->len();
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterLen::setValue(Staff* staff, Note* note, NoteEvent* evt, int value)
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

QString PianoLevelFilterLenOfftime::name()
      {
      return qApp->translate("PianoLevelsFilter", STRN_LEN_OFF_NAME);
      }

//---------------------------------------------------------
//   tooltip
//---------------------------------------------------------

QString PianoLevelFilterLenOfftime::tooltip()
      {
      return qApp->translate("PianoLevelsFilter", STRN_LEN_OFF_TT);
      }

//---------------------------------------------------------
//   maxRange
//---------------------------------------------------------

int PianoLevelFilterLenOfftime::maxRange()
      {
      return MScore::division;
      }

//---------------------------------------------------------
//   divisionGap
//---------------------------------------------------------

int PianoLevelFilterLenOfftime::divisionGap()
      {
      return MScore::division / 4;
      }


//---------------------------------------------------------
//   value
//---------------------------------------------------------

int PianoLevelFilterLenOfftime::value(Staff* /*staff*/, Note* note, NoteEvent* evt)
      {
      Chord* chord = note->chord();
      int ticks = chord->ticks().ticks();
      int gate = evt->len();
      int offTicks = ticks - (ticks * gate / 1000);

      return offTicks;
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PianoLevelFilterLenOfftime::setValue(Staff* staff, Note* note, NoteEvent* evt, int value)
      {
      Chord* chord = note->chord();
      int ticks = chord->ticks().ticks();
      int onTicks = qMax(ticks - value, 1);
      int gate = 1000 * onTicks / ticks;

      Score* score = staff->score();

      NoteEvent ne = *evt;
      ne.setLen(gate);

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
