#ifndef SELECTIONWINDOW_H
#define SELECTIONWINDOW_H

namespace Ms {
class Score;


class SelectionWindow : public QDockWidget {
      Q_OBJECT

      Score* _score;
      virtual void closeEvent(QCloseEvent*);
      virtual void hideEvent (QHideEvent* event);
   signals:
      void closed(bool);
public:
      SelectionWindow(QWidget *parent = 0);
      ~SelectionWindow();
      virtual QSize sizeHint() const {return QSize(170,170);}

      };
} // namespace Ms
#endif // SELECTIONWINDOW_H
