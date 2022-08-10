#include "automationduration.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

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

double AutomationDuration::value(Staff* staff, NoteEventBlock& block)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);

//    Ms::Chord* chord = note->chord();
//    Ms::Fraction noteLen = chord->ticks();
    Fraction noteLen = note->playTicksFraction();
    int evtLen = evt->len();
    Fraction offsetLen = noteLen - (noteLen * evtLen / 1000);

    return -offsetLen.numerator() * 1000 / offsetLen.denominator();
}

void AutomationDuration::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);

    Fraction noteLen = note->playTicksFraction();
//    Ms::Chord* chord = note->chord();
//    Ms::Fraction noteLen = chord->ticks();
    Fraction cutLen(-value, 1000);
    Fraction playLen = noteLen - cutLen;
    Fraction evtLenFrac = playLen / noteLen;
    int evtLen = qMax(evtLenFrac.numerator() * 1000 / evtLenFrac.denominator(), 1);

    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setLen(evtLen);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
