#include "automationvelocityabs.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

AutomationVelocityAbs::AutomationVelocityAbs()
{
}

PianorollAutomationNote::AutomationType AutomationVelocityAbs::type()
{
    return PianorollAutomationNote::AutomationType::VELOCITY_ABS;
}

double AutomationVelocityAbs::maxValue()
{
    return 128;
}

double AutomationVelocityAbs::minValue()
{
    return 0;
}

double AutomationVelocityAbs::value(Staff* staff, NoteEventBlock& block)
{
    Note* note = block.note;

    //Change velocity to equivalent in new metric
    switch (VeloType(note->veloType())) {
    case VeloType::USER_VAL:
        return note->veloOffset();
    default:
    case VeloType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        return static_cast<int>(dynamicsVel * (1 + note->veloOffset() / 100.0));
    }
    }
}

void AutomationVelocityAbs::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    Score* score = staff->score();
    Note* note = block.note;

    score->startCmd();

    switch (VeloType(note->veloType())) {
    case VeloType::USER_VAL:
        score->undo(new ChangeVelocity(note, VeloType::USER_VAL, value));
        break;
    default:
    case VeloType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        int newVelocity = static_cast<int>((value / (qreal)dynamicsVel - 1) * 100);

        score->undo(new ChangeVelocity(note, VeloType::OFFSET_VAL, newVelocity));
        break;
    }
    }

    score->endCmd();
}
