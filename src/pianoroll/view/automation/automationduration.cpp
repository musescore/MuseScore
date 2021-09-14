#include "automationduration.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;

AutomationDuration::AutomationDuration()
{
}

PianorollAutomationNote::AutomationType AutomationDuration::type()
{
    return PianorollAutomationNote::AutomationType::DURATION;
}

double AutomationDuration::maxValue()
{
    return 1000;
}

double AutomationDuration::minValue()
{
    return -1000;
}

double AutomationDuration::value(Ms::Staff* staff, NoteEventBlock& block)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);

    Ms::Chord* chord = note->chord();
    Ms::Fraction noteLen = chord->ticks();
    int evtLen = evt->len();
    Ms::Fraction offsetLen = noteLen - (noteLen * evtLen / 1000);

    return -offsetLen.numerator() * 1000 / offsetLen.denominator();
}

void AutomationDuration::setValue(Ms::Staff* staff, NoteEventBlock& block, double value)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);

    Ms::Chord* chord = note->chord();
    Ms::Fraction noteLen = chord->ticks();
    Ms::Fraction cutLen(-value, 1000);
    Ms::Fraction playLen = noteLen - cutLen;
    Ms::Fraction evtLenFrac = playLen / noteLen;
    int evtLen = qMax(evtLenFrac.numerator() * 1000 / evtLenFrac.denominator(), 1);

    Ms::Score* score = staff->score();

    Ms::NoteEvent ne = *evt;
    ne.setLen(evtLen);

    score->startCmd();
    score->undo(new Ms::ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
