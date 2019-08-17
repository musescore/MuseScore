#include "pianolevelsfilter.h"

#include "libmscore/note.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"


namespace Ms {

PianoLevelsFilter* PianoLevelsFilter::FILTER_LIST[] = {
      new PianoLevelFilterLen,
      new PianoLevelFilterVeloOffset,
      new PianoLevelFilterVeloUser,
      new PianoLevelFilterOnTime,
      0  //end of list indicator
};

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
//   value
//---------------------------------------------------------

int PianoLevelFilterVeloOffset::value(Staff* staff, Note* note, NoteEvent* /*evt*/)
      {
      //Change velocity to equivilent in new metric
      switch (Note::ValueType(note->veloType())) {
            case Note::ValueType::USER_VAL: {
                  int dynamicsVel = staff->velocities().velo(note->tick().ticks());
                  return (int)((note->veloOffset() / (qreal)dynamicsVel - 1) * 100);
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
                  int dynamicsVel = staff->velocities().velo(note->tick().ticks());
                  int newVelocity = (int)(dynamicsVel * (1 + value / 100.0));

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
//   value
//---------------------------------------------------------

int PianoLevelFilterVeloUser::value(Staff* staff, Note* note, NoteEvent* /*evt*/)
      {
      //Change velocity to equivilent in new metric
      switch (Note::ValueType(note->veloType())) {
            case Note::ValueType::USER_VAL:
                  return note->veloOffset();
            default:
            case Note::ValueType::OFFSET_VAL: {
                  int dynamicsVel = staff->velocities().velo(note->tick().ticks());
                  return (int)(dynamicsVel * (1 + note->veloOffset() / 100.0));
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
                  int dynamicsVel = staff->velocities().velo(note->tick().ticks());
                  int newVelocity = (int)((value / (qreal)dynamicsVel - 1) * 100);

                  score->undo(new ChangeVelocity(note, Note::ValueType::OFFSET_VAL, newVelocity));
                  break;
                  }
            }

      score->endCmd();
      }

}
