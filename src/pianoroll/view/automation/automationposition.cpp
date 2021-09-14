#include "automationposition.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;

AutomationPosition::AutomationPosition()
{
}

PianorollAutomationNote::AutomationType AutomationPosition::type()
{
    return PianorollAutomationNote::AutomationType::POSITION;
}

double AutomationPosition::maxValue()
{
    return 1000;
}

double AutomationPosition::minValue()
{
    return -1000;
}

double AutomationPosition::value(Ms::Staff* staff, NoteEventBlock& block)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);

    return evt->ontime();
}

void AutomationPosition::setValue(Ms::Staff* staff, NoteEventBlock& block, double value)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);
    Ms::Score* score = staff->score();

    Ms::NoteEvent ne = *evt;
    ne.setOntime(value);

    score->startCmd();
    score->undo(new Ms::ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
