#include "automationvelocity.h"

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
    return 100;
}

double AutomationVelocity::minValue()
{
    return 0;
}

double AutomationVelocity::value(NoteEventBlock& block)
{
    return 50;
}
