#ifndef IMPORTMIDI_TRVIEW_H
#define IMPORTMIDI_TRVIEW_H

#include <QTableView>


class TracksView : public QTableView
      {
   public:
      explicit TracksView(QWidget *parent);

   protected:
      bool viewportEvent(QEvent *event);
      };


#endif // IMPORTMIDI_TRVIEW_H
