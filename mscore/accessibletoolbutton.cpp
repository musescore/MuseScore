
#include <QGraphicsColorizeEffect>
#include <QToolButton>
#include "accessibletoolbutton.h"

namespace Ms {

AccessibleToolButton::AccessibleToolButton(QWidget* parent, QAction* defaultQAction ): QToolButton(parent)
      {
      this->setDefaultAction(defaultQAction);
#if defined(Q_OS_MAC)
      this->setFocusPolicy(Qt::StrongFocus);
#else
      this->setFocusPolicy(Qt::TabFocus);
#endif

      this->setAccessibleName(defaultQAction->text());
      this->setAccessibleDescription(defaultQAction->toolTip());
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
      this->setGraphicsEffect(NULL);
      QToolButton::focusInEvent(e);
      }

void AccessibleToolButton::keyPressEvent(QKeyEvent *e)
      {
      //Pressing Enter or Return button triggers the default action
      if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            this->animateClick();
            return;
            }

      QToolButton::keyPressEvent(e);
      }
}
