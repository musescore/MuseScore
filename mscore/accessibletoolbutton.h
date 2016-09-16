
#ifndef __ACCESSIBLETOOLBUTTON_H__
#define __ACCESSIBLETOOLBUTTON_H__

namespace Ms {

/*
 * This class inherits QToolButton and allows tabbing through the tool bar's buttons
 */
class AccessibleToolButton : public QToolButton{
      Q_OBJECT
public:
      AccessibleToolButton(QWidget* parent, QAction* defaultQAction = 0);
      void focusInEvent(QFocusEvent* e);
      void focusOutEvent(QFocusEvent* e);
      void keyPressEvent(QKeyEvent *e);
      };

}

#endif // __ACCESSIBLETOOLBUTTON_H__
