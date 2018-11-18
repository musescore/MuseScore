#include "pianolevelschooser.h"
#include "pianolevelsfilter.h"

namespace Ms {


//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

PianoLevelsChooser::PianoLevelsChooser(QWidget *parent)
      : QWidget(parent)
{
      _levelsIndex = 0;

      QGridLayout* layout = new QGridLayout;

      levelsCombo = new QComboBox;
      for (int i = 0; PianoLevelsFilter::FILTER_LIST[i]; ++i) {
            QString name = PianoLevelsFilter::FILTER_LIST[i]->name();
            levelsCombo->addItem(name, i);
            }

      layout->addWidget(levelsCombo, 0, 0, 1, 1);

      setLayout(layout);

      connect(levelsCombo, SIGNAL(activated(int)), SLOT(setLevelsIndex(int)));
}


//---------------------------------------------------------
//   PianoLevelsChooser
//---------------------------------------------------------

void PianoLevelsChooser::setLevelsIndex(int index)
{
      if (_levelsIndex != index) {
            _levelsIndex = index;
            emit levelsIndexChanged(index);
            }
}

}
