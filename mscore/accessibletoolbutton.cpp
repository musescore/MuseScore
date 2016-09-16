
#include <QGraphicsColorizeEffect>
#include <QToolButton>
#include "accessibletoolbutton.h"

namespace Ms {

AccessibleToolButton::AccessibleToolButton(QWidget* parent, QAction* defaultQAction)
   : QToolButton(parent)
      {
      if (defaultQAction)
            setDefaultAction(defaultQAction);
      setFocusPolicy(Qt::TabFocus);

      setAccessibleName(defaultQAction->text());
      setAccessibleDescription(defaultQAction->toolTip());
      }


void AccessibleToolButton::focusInEvent(QFocusEvent* e)
      {
      //If the button gains focus by tabbing or backtabbing, it will change its color
      if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason) {
            setGraphicsEffect(new QGraphicsColorizeEffect());
            }

      QToolButton::focusInEvent(e);
      }

void AccessibleToolButton::focusOutEvent(QFocusEvent* e)
      {
      setGraphicsEffect(nullptr);
      QToolButton::focusInEvent(e);
      }

void AccessibleToolButton::keyPressEvent(QKeyEvent *e)
      {
      //Pressing Enter or Return button triggers the default action
      if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            animateClick();
            return;
            }

      QToolButton::keyPressEvent(e);
      }
}
