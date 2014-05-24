
#include <QGraphicsColorizeEffect>
#include <QToolButton>
#include "accessibletoolbutton.h"

namespace Ms {

AccessibleToolButton::AccessibleToolButton(QWidget* parent, QAction* defaultQAction ): QToolButton(parent){
    this->setDefaultAction(defaultQAction);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setAutoRaise(true);
    this->setAccessibleName(defaultQAction->text());
    this->setAccessibleDescription(defaultQAction->toolTip());
    }


void AccessibleToolButton::focusInEvent(QFocusEvent* e){
    //If the button gains focus by tabbing, it will change its color
    if(e->reason() == Qt::TabFocusReason){
        setGraphicsEffect(new QGraphicsColorizeEffect());
        }

    QToolButton::focusInEvent(e);
    }

void AccessibleToolButton::focusOutEvent(QFocusEvent* e){
    this->setGraphicsEffect(NULL);
    QToolButton::focusInEvent(e);
    }

bool AccessibleToolButton::event(QEvent *e){
    if(e->type() == QEvent::KeyPress){
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

        //Pressing Enter or Return button triggers the default action
        if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return){
            this->animateClick();
            return true;
            }
        }

    return QToolButton::event(e);
    }
}
