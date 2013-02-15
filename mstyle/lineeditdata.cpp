//////////////////////////////////////////////////////////////////////////////
// oxygenlineeditdata.cpp
// data container for QLineEdit transition
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
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

#include "lineeditdata.h"

// use 20 milliseconds for animation lock
const int LineEditData::lockTime_ = 20;

//______________________________________________________
LineEditData::LineEditData( QObject* parent, QLineEdit* target, int duration ):
      TransitionData( parent, target, duration ),
      target_( target ),
      hasClearButton_( false ),
      edited_( false ) {
      target_.data()->installEventFilter( this );

      checkClearButton();

      connect( target_.data(), SIGNAL( destroyed() ), SLOT( targetDestroyed() ) );
      connect( target_.data(), SIGNAL( textEdited( const QString& ) ), SLOT( textEdited( void ) ) );
      connect( target_.data(), SIGNAL( textChanged( const QString& ) ), SLOT( textChanged( void ) ) );

      /*
      Additional signal/slot connections depending on widget's parent.
      This is needed because parents sometime disable the textChanged signal of the embedded
      QLineEdit
      */
      if ( qobject_cast<QSpinBox*>( target_.data()->parentWidget() ) || qobject_cast<QDoubleSpinBox*>( target_.data()->parentWidget() ) ) {

            connect( target_.data()->parentWidget(), SIGNAL( valueChanged( const QString& ) ), SLOT( textChanged( void ) ) );

            }
      else if ( qobject_cast<QDateTimeEdit*>( target_.data()->parentWidget() ) ) {

            connect( target_.data()->parentWidget(), SIGNAL( dateTimeChanged ( const QDateTime& ) ), SLOT( textChanged( void ) ) );

            }

      // update cached pixmap on selection change
      connect( target_.data(), SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );

      }

//___________________________________________________________________
bool LineEditData::eventFilter( QObject* object, QEvent* event ) {

      if ( !( enabled() && object && object == target_.data() ) ) {
            return TransitionData::eventFilter( object, event );
            }

      switch ( event->type() ) {
            case QEvent::Show:
            case QEvent::Resize:
            case QEvent::Move:
                  if ( !recursiveCheck() ) {
                        timer_.start( 0, this );
                        break;
                        }

            default:
                  break;
            }

      return TransitionData::eventFilter( object, event );

      }

//___________________________________________________________________
void LineEditData::timerEvent( QTimerEvent* event ) {
      if ( event->timerId() == timer_.timerId() ) {

            timer_.stop();
            checkClearButton();
            if ( enabled() && transition() && target_ && target_.data()->isVisible() ) {
                  setRecursiveCheck( true );
                  transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) );
                  setRecursiveCheck( false );
                  }

            }
      else if ( event->timerId() == animationLockTimer_.timerId() ) {

            unlockAnimations();

            }
      else return TransitionData::timerEvent( event );

      }

//___________________________________________________________________
void LineEditData::checkClearButton( void ) {
      if ( !target_ ) return;
      QObjectList children( target_.data()->children() );
      hasClearButton_ = false;
      foreach( QObject * child, children ) {
            if ( child->inherits( "KLineEditButton" ) ) {
                  hasClearButton_ = true;
                  clearButtonRect_ = static_cast<QWidget*>(child)->geometry();
                  break;
                  }
            }

      return;
      }

//___________________________________________________________________
void LineEditData::textEdited( void ) {
      edited_ = true;
      if ( !recursiveCheck() ) {
            timer_.start( 0, this );
            }
      }


//___________________________________________________________________
void LineEditData::selectionChanged( void ) {
      if ( !recursiveCheck() ) {
            timer_.start( 0, this );
            }
      }

//___________________________________________________________________
void LineEditData::textChanged( void ) {

      // check whether text change was triggered manually
      // in which case do not start transition
      if ( edited_ ) {
            edited_ = false;
            return;
            }

      if ( transition().data()->isAnimated() ) {
            transition().data()->endAnimation();
            }

      if ( isLocked() ) {
            // if locked one do not start the new animation, to prevent flicker
            // instead, one hides the transition pixmap, trigger an update, and return.
            // animations are re-locked.
            transition().data()->hide();
            lockAnimations();
            timer_.start( 0, this );
            return;
            }

      if ( initializeAnimation() ) {

            lockAnimations();
            animate();

            }
      else {

            transition().data()->hide();

            }
      }

//___________________________________________________________________
bool LineEditData::initializeAnimation( void ) {
      if ( !( enabled() && target_ && target_.data()->isVisible() ) ) return false;

      if ( recursiveCheck() ) return false;

      QRect current( targetRect() );

      transition().data()->setOpacity(0);
      transition().data()->setGeometry( current );

      if ( widgetRect_.isValid() &&
                  !transition().data()->currentPixmap().isNull() &&
                  widgetRect_ != current ) {

            // if label geometry has changed since last animation
            // one must clone the pixmap to make it match the right
            // geometry before starting the animation.
            QPixmap pixmap( current.size() );
            pixmap.fill( Qt::transparent );
            QPainter p( &pixmap );
            p.drawPixmap( widgetRect_.topLeft() - current.topLeft(), transition().data()->currentPixmap() );
            p.end();
            transition().data()->setStartPixmap( pixmap );

            }
      else {

            transition().data()->setStartPixmap( transition().data()->currentPixmap() );

            }

      bool valid( !transition().data()->startPixmap().isNull() );
      if ( valid ) {
            transition().data()->show();
            transition().data()->raise();
            }

      setRecursiveCheck( true );
      transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) );
      setRecursiveCheck( false );

      return valid;

      }

//___________________________________________________________________
bool LineEditData::animate( void ) {
      transition().data()->animate();
      return true;
      }

//___________________________________________________________________
void LineEditData::targetDestroyed( void ) {
      setEnabled( false );
      target_.clear();
      }
