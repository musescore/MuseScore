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
    return note->userVelocity();
}

void AutomationVelocityAbs::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    Score* score = staff->score();
    Note* note = block.note;

    score->startCmd();

    score->undo(new ChangeVelocity(note, value));

    score->endCmd();
}
