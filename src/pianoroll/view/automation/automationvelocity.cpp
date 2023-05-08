#include "automationvelocity.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;

AutomationVelocity::AutomationVelocity()
{
}

PianorollAutomationEditor::AutomationType AutomationVelocity::type()
{
    return PianorollAutomationEditor::AutomationType::VELOCITY;
}

double AutomationVelocity::maxValue()
{
    return 200;
}

double AutomationVelocity::minValue()
{
    return -200;
}

double AutomationVelocity::value(Ms::Staff* staff, NoteEventBlock& block)
{
    Ms::Note* note = block.note;

    //Change velocity to equivalent in new metric
    switch (note->veloType()) {
    case Ms::Note::ValueType::USER_VAL:
    {
        int dynamicsVel = staff->velocities().val(note->tick());
        return static_cast<int>((note->veloOffset() / (qreal)dynamicsVel - 1) * 100);
    }
    default:
    case Ms::Note::ValueType::OFFSET_VAL:
        return note->veloOffset();
    }
}

void AutomationVelocity::setValue(Ms::Staff* staff, NoteEventBlock& block, double value)
{
    Ms::Score* score = staff->score();
    Ms::Note* note = block.note;

    score->startCmd();

    switch (note->veloType()) {
    case Ms::Note::ValueType::USER_VAL:
    {
        int dynamicsVel = staff->velocities().val(note->tick());
        int newVelocity = static_cast<int>(dynamicsVel * (1 + value / 100.0));

        score->undo(new Ms::ChangeVelocity(note, Ms::Note::ValueType::USER_VAL, newVelocity));

        break;
    }
    default:
    case Ms::Note::ValueType::OFFSET_VAL:
    {
        score->undo(new Ms::ChangeVelocity(note, Ms::Note::ValueType::OFFSET_VAL, value));
        break;
    }
    }

    score->endCmd();
}
