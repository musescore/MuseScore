#include "importmidi_trview.h"


TracksView::TracksView(QWidget *parent)
      : QTableView(parent)
      {
      }

// show tooltip if the text is wider than the table cell

bool TracksView::viewportEvent(QEvent *event)
      {
      if (event->type() == QEvent::ToolTip) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            QModelIndex index = indexAt(helpEvent->pos());
            if (index.isValid()) {
                  QSize sizeHint = itemDelegate(index)->sizeHint(viewOptions(), index);
                  QRect rItem(0, 0, sizeHint.width(), sizeHint.height());
                  QRect rVisual = visualRect(index);
                  if (rItem.width() <= rVisual.width()) {
                        QToolTip::hideText();
                        return false;
                        }
                  }
            }

      return QTableView::viewportEvent(event);
      }
