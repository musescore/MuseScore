#include "selectionwindow.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/select.h"

namespace Ms {

static const int DEFAULT_POS_X  = 300;
static const int DEFAULT_POS_Y  = 100;

SelectionWindow::SelectionWindow(QWidget *parent, Score* score) :
      QDockWidget(tr("Selections"),parent)
      {
      setObjectName("selection-window");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      _score = score;

      _listWidget = new QListWidget;
      setWidget(_listWidget);
      QStringList labels;
      labels << "1st Voice"
             << "2nd Voice"
             << "3rd Voice"
             << "4th Voice"
             << "Dynamics"
             << "Fingering"
             << "Lyrics"
             << "Articulations";

      QStringListIterator it(labels);
      int row = 0;
      while (it.hasNext()) {
            QListWidgetItem *listItem = new QListWidgetItem(it.next(),_listWidget);
            listItem->setData(Qt::UserRole, QVariant(1 << row++));
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
            }
      }

void SelectionWindow::changeCheckbox(QListWidgetItem* item)
      {
      int type = item->data(Qt::UserRole).toInt();
      int filtered = _score->selectionFilter().filtered();
      if(item->checkState() == Qt::Checked)
            filtered = filtered | type;
      else
            filtered = filtered & ~type;

      _score->selectionFilter().setFiltered(filtered);
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
}
