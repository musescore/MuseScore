#include "automationvelocity.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

AutomationVelocity::AutomationVelocity()
{
}

PianorollAutomationNote::AutomationType AutomationVelocity::type()
{
    return PianorollAutomationNote::AutomationType::VELOCITY;
}

double AutomationVelocity::maxValue()
{
    return 200;
}

double AutomationVelocity::minValue()
{
    return -200;
}

double AutomationVelocity::value(Staff* staff, NoteEventBlock& block)
{
    //Note* note = block.note;

    ////Change velocity to equivalent in new metric
    //switch (note->veloType()) {
    //case VeloType::USER_VAL:
    //{
    //    int dynamicsVel = staff->velocities().val(note->tick());
    //    return static_cast<int>((note->userVelocity() / (qreal)dynamicsVel - 1) * 100);
    //}
    //default:
    //case VeloType::OFFSET_VAL:
    //    return note->userVelocity();
    //}
    return 0;
}

void AutomationVelocity::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    //Score* score = staff->score();
    //Note* note = block.note;

    //score->startCmd();

    //switch (note->veloType()) {
    //case VeloType::USER_VAL:
    //{
    //    int dynamicsVel = staff->velocities().val(note->tick());
    //    int newVelocity = static_cast<int>(dynamicsVel * (1 + value / 100.0));

    //    score->undo(new ChangeVelocity(note, newVelocity));

    //    break;
    //}
    //default:
    //case VeloType::OFFSET_VAL:
    //{
    //    score->undo(new ChangeVelocity(note, value));
    //    break;
    //}
    //}

    //score->endCmd();
}
