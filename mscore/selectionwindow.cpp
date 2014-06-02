#include "selectionwindow.h"
#include "musescore.h"

namespace Ms {

static const int DEFAULT_POS_X  = 300;
static const int DEFAULT_POS_Y  = 100;

SelectionWindow::SelectionWindow(QWidget *parent) :
      QDockWidget(tr("Selections"),parent)
      {
      setObjectName("selection-window");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      _score = 0;

      QListWidget* listWidget = new QListWidget;
      setWidget(listWidget);
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
      while (it.hasNext()) {
            QListWidgetItem *listItem = new QListWidgetItem(it.next(),listWidget);
            listItem->setCheckState(Qt::Unchecked);
            listWidget->addItem(listItem);
            }
      }

SelectionWindow::~SelectionWindow()
      {
      QSettings settings;
      if (isVisible()) {
            settings.setValue("selectionWindow/pos", pos());
            }
      }
//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showSelectionWindow(bool val)
      {
      QAction* a = getAction("toggle-selection-window");
      if (selectionWindow == 0) {
            selectionWindow = new SelectionWindow(this);
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
