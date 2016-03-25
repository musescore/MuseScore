//////////////////////////////////////////////////////////////////////////////
// oxygentoolbardata.cpp
// stores event filters and maps widgets to timelines for animations
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "toolbardata.h"

//________________________________________________________________________
ToolBarData::ToolBarData( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ),
      opacity_( 0 ),
      progress_( 0 ),
      currentObject_( 0 ),
      entered_( false ) {

      target->installEventFilter( this );

      animation_ = new Animation( duration, this );
      animation().data()->setDirection( Animation::Forward );
      animation().data()->setStartValue( 0.0 );
      animation().data()->setEndValue( 1.0 );
      animation().data()->setTargetObject( this );
      animation().data()->setPropertyName( "opacity" );

      // progress animation
      progressAnimation_ = new Animation( duration, this );
      progressAnimation().data()->setDirection( Animation::Forward );
      progressAnimation().data()->setStartValue( 0 );
      progressAnimation().data()->setEndValue( 1 );
      progressAnimation().data()->setTargetObject( this );
      progressAnimation().data()->setPropertyName( "progress" );
      progressAnimation().data()->setEasingCurve( QEasingCurve::Linear );

      // add all children widgets to event handler
      foreach( QObject * child, target->children() ) {
            if ( qobject_cast<QToolButton*>( child ) ) childAddedEvent( child );
            }

      }

//______________________________________________
bool ToolBarData::eventFilter( QObject* object, QEvent* event ) {

      // check object
      const QObject* targetData = target().data();
      if ( object ==  targetData ) {

            switch ( event->type() ) {

                  case QEvent::Enter: {
                        if ( enabled() ) {
                              object->event( event );
                              enterEvent( object );
                              return true;
                              }
                        else return false;
                        }

                  case QEvent::ChildAdded: {

                        // add children even in disabled case, to make sure they
                        // are properly registered when engine is enabled
                        QChildEvent* childEvent( static_cast<QChildEvent*>( event ) );
                        childAddedEvent( childEvent->child() );
                        break;
                        }

                  default:
                        break;

                  }

            }
      else if ( object->parent() == targetData ) {

            if ( !enabled() ) return false;

            switch ( event->type() ) {

                  case QEvent::HoverEnter:
                        childEnterEvent( object );
                        break;

                  case QEvent::HoverLeave:
                        if ( currentObject() && !timer_.isActive() ) timer_.start( 100, this );
                        break;

                  default:
                        break;

                  }

            }

      return false;

      }

//____________________________________________________________
void ToolBarData::updateAnimatedRect( void ) {

      // check rect validity
      if ( currentRect().isNull() || previousRect().isNull() ) {
            animatedRect_ = QRect();
            return;
            }

      // compute rect located 'between' previous and current
      animatedRect_.setLeft( previousRect().left() + progress() * (currentRect().left() - previousRect().left()) );
      animatedRect_.setRight( previousRect().right() + progress() * (currentRect().right() - previousRect().right()) );
      animatedRect_.setTop( previousRect().top() + progress() * (currentRect().top() - previousRect().top()) );
      animatedRect_.setBottom( previousRect().bottom() + progress() * (currentRect().bottom() - previousRect().bottom()) );

      // trigger update
      setDirty();
      return;

      }

//________________________________________________________________________
void ToolBarData::enterEvent( const QObject* ) {

      if ( timer_.isActive() ) timer_.stop();
      if ( animation().data()->isRunning() ) animation().data()->stop();
      if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
      clearPreviousRect();
      clearAnimatedRect();

      return;

      }


//________________________________________________________________________
void ToolBarData::leaveEvent( const QObject* ) {

      if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
      if ( animation().data()->isRunning() ) animation().data()->stop();
      clearAnimatedRect();
      clearPreviousRect();

      if ( currentObject() ) {

            clearCurrentObject();
            animation().data()->setDirection( Animation::Backward );
            animation().data()->start();

            }

      return;

      }

//________________________________________________________________________
void ToolBarData::childEnterEvent( const QObject* object ) {

      if ( object == currentObject() ) return;

      const QToolButton* local = qobject_cast<const QToolButton*>( object );

      // check if current position match another action
      if ( local && local->isEnabled() ) {

            if ( timer_.isActive() ) timer_.stop();

            // get rect
            QRect activeRect( local->rect().translated( local->mapToParent( QPoint(0, 0) ) ) );

            // update previous rect if the current action is valid
            if ( currentObject() ) {

                  if ( !progressAnimation().data()->isRunning() ) {

                        setPreviousRect( currentRect() );

                        }
                  else if ( progress() < 1 && currentRect().isValid() && previousRect().isValid() ) {

                        // re-calculate previous rect so that animatedRect
                        // is unchanged after currentRect is updated
                        // this prevents from having jumps in the animation
                        qreal ratio = progress() / (1.0 - progress());
                        previousRect_.adjust(
                              ratio * ( currentRect().left() - activeRect.left() ),
                              ratio * ( currentRect().top() - activeRect.top() ),
                              ratio * ( currentRect().right() - activeRect.right() ),
                              ratio * ( currentRect().bottom() - activeRect.bottom() ) );

                        }

                  // update current action
                  setCurrentObject( local );
                  setCurrentRect( activeRect );
                  if ( animation().data()->isRunning() ) animation().data()->stop();
                  if ( !progressAnimation().data()->isRunning() ) progressAnimation().data()->start();

                  }
            else {

                  setCurrentObject( local );
                  setCurrentRect( activeRect );
                  if ( !entered_ ) {

                        entered_ = true;
                        if ( animation().data()->isRunning() ) animation().data()->stop();
                        if ( !progressAnimation().data()->isRunning() ) progressAnimation().data()->start();

                        }
                  else {

                        setPreviousRect( activeRect );
                        clearAnimatedRect();
                        if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
                        animation().data()->setDirection( Animation::Forward );
                        if ( !animation().data()->isRunning() ) animation().data()->start();
                        }

                  }

            }
      else if ( currentObject() ) {

            if ( !timer_.isActive() ) timer_.start( 100, this );

            }

      return;

      }

//___________________________________________________________________
void ToolBarData::childAddedEvent( QObject* object ) {
      QWidget* widget( qobject_cast<QWidget*>( object ) );
      if ( !widget ) return;

      // add connections
      connect( animation().data(), SIGNAL( valueChanged( const QVariant& ) ), widget, SLOT( update() ), Qt::UniqueConnection  );
      connect( progressAnimation().data(), SIGNAL( valueChanged( const QVariant& ) ), widget, SLOT( update() ), Qt::UniqueConnection  );

      // add event filter
      widget->removeEventFilter( this );
      widget->installEventFilter( this );
      }

//___________________________________________________________
void ToolBarData::timerEvent( QTimerEvent* event ) {

      if ( event->timerId() != timer_.timerId() ) return AnimationData::timerEvent( event );
      timer_.stop();
      leaveEvent( target().data() );
      }

