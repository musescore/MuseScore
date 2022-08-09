#include "automationvelocityabs.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;

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

double AutomationVelocityAbs::value(Ms::Staff* staff, NoteEventBlock& block)
{
    Ms::Note* note = block.note;

    //Change velocity to equivalent in new metric
    switch (Ms::VeloType(note->veloType())) {
    case Ms::VeloType::USER_VAL:
        return note->veloOffset();
    default:
    case Ms::VeloType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        return static_cast<int>(dynamicsVel * (1 + note->veloOffset() / 100.0));
    }
    }
}

void AutomationVelocityAbs::setValue(Ms::Staff* staff, NoteEventBlock& block, double value)
{
    Ms::Score* score = staff->score();
    Ms::Note* note = block.note;

    score->startCmd();

    switch (Ms::VeloType(note->veloType())) {
    case Ms::VeloType::USER_VAL:
        score->undo(new Ms::ChangeVelocity(note, Ms::VeloType::USER_VAL, value));
        break;
    default:
    case Ms::VeloType::OFFSET_VAL: {
        int dynamicsVel = staff->velocities().val(note->tick());
        int newVelocity = static_cast<int>((value / (qreal)dynamicsVel - 1) * 100);

        score->undo(new Ms::ChangeVelocity(note, Ms::VeloType::OFFSET_VAL, newVelocity));
        break;
    }
    }

    score->endCmd();
}
