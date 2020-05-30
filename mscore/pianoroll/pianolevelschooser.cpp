#include "pianolevelschooser.h"
#include "pianolevelsfilter.h"
#include "pianoview.h"

#include "libmscore/score.h"

namespace Ms {
//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

PianoLevelsChooser::PianoLevelsChooser(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    _levelsIndex = 0;

    for (int i = 0; PianoLevelsFilter::FILTER_LIST[i]; ++i) {
        QString name = PianoLevelsFilter::FILTER_LIST[i]->name();
        levelsCombo->addItem(name, i);
        levelsCombo->setItemData(i, PianoLevelsFilter::FILTER_LIST[i]->tooltip(), Qt::ToolTipRole);
    }

    connect(levelsCombo, SIGNAL(activated(int)), SLOT(setLevelsIndex(int)));
    connect(setEventsBn, SIGNAL(clicked(bool)), SLOT(setEventDataPressed()));
}

//---------------------------------------------------------
//   setPianoView
//---------------------------------------------------------

void PianoLevelsChooser::setPianoView(PianoView* pianoView)
{
    _pianoView = pianoView;
}

//---------------------------------------------------------
//   updateSetboxValue
//---------------------------------------------------------

void PianoLevelsChooser::updateSetboxValue()
{
    QList<PianoItem*> items = _pianoView->getSelectedItems();

    if (items.size() == 1) {
        PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];

        PianoItem* item = items[0];
        Note* note = item->note();

        NoteEvent* event = item->getTweakNoteEvent();
        int value = filter->value(_staff, note, event);
        eventValSpinBox->setValue(value);
    }
}

//---------------------------------------------------------
//   setLevelsIndex
//---------------------------------------------------------

void PianoLevelsChooser::setLevelsIndex(int index)
{
    if (_levelsIndex != index) {
        _levelsIndex = index;
        updateSetboxValue();
        emit levelsIndexChanged(index);
    }
}

//---------------------------------------------------------
//   setEventDataPressed
//---------------------------------------------------------

void PianoLevelsChooser::setEventDataPressed()
{
    PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];

    int val = eventValSpinBox->value();
    QList<Note*> noteList = _staff->getNotes();

    Score* score = _staff->score();

    score->startCmd();

    for (Note* note: noteList) {
        if (!note->selected()) {
            continue;
        }

        if (filter->isPerEvent()) {
            for (NoteEvent& e : note->playEvents()) {
                filter->setValue(_staff, note, &e, val);
            }
        } else {
            filter->setValue(_staff, note, nullptr, val);
        }
    }

    score->endCmd();

    emit notesChanged();
}
}
