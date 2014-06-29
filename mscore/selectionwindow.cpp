#include "selectionwindow.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/select.h"

namespace Ms {

static const int DEFAULT_POS_X  = 300;
static const int DEFAULT_POS_Y  = 100;

static const char* labels[] = {
      QT_TRANSLATE_NOOP("selectionfilter", "1st Voice"),
      QT_TRANSLATE_NOOP("selectionfilter", "2nd Voice"),
      QT_TRANSLATE_NOOP("selectionfilter", "3rd Voice"),
      QT_TRANSLATE_NOOP("selectionfilter", "4th Voice"),
      QT_TRANSLATE_NOOP("selectionfilter", "Dynamics"),
      QT_TRANSLATE_NOOP("selectionfilter", "Fingering"),
      QT_TRANSLATE_NOOP("selectionfilter", "Lyrics"),
      QT_TRANSLATE_NOOP("selectionfilter", "Chord Symbols"),
      QT_TRANSLATE_NOOP("selectionfilter", "Articulations"),
      QT_TRANSLATE_NOOP("selectionfilter", "Slurs"),
      QT_TRANSLATE_NOOP("selectionfilter", "Figured Bass"),
      QT_TRANSLATE_NOOP("selectionfilter", "Ottava"),
      QT_TRANSLATE_NOOP("selectionfilter", "Pedal Line"),
      QT_TRANSLATE_NOOP("selectionfilter", "Arpeggio"),
      QT_TRANSLATE_NOOP("selectionfilter", "Glissando"),
      QT_TRANSLATE_NOOP("selectionfilter", "Fret Diagram")
      };

const int numLabels = sizeof(labels)/sizeof(labels[0]);

SelectionWindow::SelectionWindow(QWidget *parent, Score* score) :
      QDockWidget(tr("Selections"),parent)
      {
      setObjectName("selection-window");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      _score = score;

      _listWidget = new QListWidget;
      setWidget(_listWidget);

      for (int row = 0; row < numLabels; row++) {
            QListWidgetItem *listItem = new QListWidgetItem(labels[row],_listWidget);
            listItem->setData(Qt::UserRole, QVariant(1 << row));
            listItem->setCheckState(Qt::Unchecked);
            _listWidget->addItem(listItem);
            }
      updateFilteredElements();

      connect(_listWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(changeCheckbox(QListWidgetItem*)));
      }

SelectionWindow::~SelectionWindow()
      {
      QSettings settings;
      if (isVisible()) {
            settings.setValue("selectionWindow/pos", pos());
            }
      }

void SelectionWindow::updateFilteredElements()
      {
      int filter = _score->selectionFilter().filtered();
      for(int row = 0; row < _listWidget->count(); row++) {
            QListWidgetItem *item = _listWidget->item(row);
            if (filter & 1 << row)
                  item->setCheckState(Qt::Checked);
            else
                  item->setCheckState(Qt::Unchecked);
            }
      }

void SelectionWindow::changeCheckbox(QListWidgetItem* item)
      {
      int type = item->data(Qt::UserRole).toInt();

      bool set = false;
      item->checkState() == Qt::Checked ? set = true : set = false;
      _score->selectionFilter().setFiltered(static_cast<SelectionFilterType>(type),set);

      if (_score->selection().isRange())
            _score->selection().updateSelectedElements();
      updateFilteredElements();
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showSelectionWindow(bool val)
      {
      QAction* a = getAction("toggle-selection-window");
      if (selectionWindow == 0) {
            selectionWindow = new SelectionWindow(this,this->currentScore());
            connect(selectionWindow, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::RightDockWidgetArea,selectionWindow);
            }
      selectionWindow->setVisible(val);
      }
void SelectionWindow::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

void SelectionWindow::hideEvent(QHideEvent* ev)
      {
      //QSettings settings;
      QSettings settings;
      settings.setValue("selectionWindow/pos", pos());
      QWidget::hideEvent(ev);
      }

void SelectionWindow::setScore(Score* score)
      {
      _score = score;
      updateFilteredElements();
      }

}
