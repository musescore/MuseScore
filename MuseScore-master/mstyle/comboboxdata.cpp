// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// oxygencomboboxdata.cpp
// data container for QComboBox transition
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

#include "comboboxdata.h"

//______________________________________________________
ComboBoxData::ComboBoxData( QObject* parent, QComboBox* target, int duration ):
      TransitionData( parent, target, duration ),
      target_( target ) {
      target_.data()->installEventFilter( this );
      connect( target_.data(), SIGNAL( destroyed() ), SLOT( targetDestroyed() ) );
      connect( target_.data(), SIGNAL( currentIndexChanged( int ) ), SLOT( indexChanged() ) );
      }

//___________________________________________________________________
void ComboBoxData::indexChanged( void ) {

      if ( recursiveCheck() ) return;

      if ( transition().data()->isAnimated() ) {
            transition().data()->endAnimation();
            }

      if ( initializeAnimation() ) animate();
      else transition().data()->hide();

      }

//___________________________________________________________________
bool ComboBoxData::eventFilter( QObject* object, QEvent* event ) {

      // make sure engine is enabled
      if ( !( enabled() && object == target_.data() ) ) {
            return TransitionData::eventFilter( object, event );
            }

      // make sure that target is not editable
      if ( target_.data()->isEditable() ) {
            return TransitionData::eventFilter( object, event );
            }

      switch ( event->type() ) {

            case QEvent::Show:
            case QEvent::Resize:
            case QEvent::Move:
                  if ( !recursiveCheck() && target_.data()->isVisible() ) {
                        timer_.start( 0, this );
                        }
                  break;

            default:
                  break;
            }

      return TransitionData::eventFilter( object, event );

      }

//___________________________________________________________________
void ComboBoxData::timerEvent( QTimerEvent* event ) {
      if ( event->timerId() == timer_.timerId() ) {

            timer_.stop();
            if ( enabled() && transition() && target_ && !target_.data()->isVisible() ) {
                  setRecursiveCheck( true );
                  transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) );
                  setRecursiveCheck( false );
                  }

            }
      else return TransitionData::timerEvent( event );

      }

//___________________________________________________________________
bool ComboBoxData::initializeAnimation( void ) {
      if ( !( enabled() && target_ && target_.data()->isVisible() ) ) return false;
      if ( target_.data()->isEditable() ) {
            /*
            do nothing for editable comboboxes because
            lineEditor animations are handled directly
            */
            return false;
            }

      transition().data()->setOpacity(0);
      transition().data()->setGeometry( targetRect() );
      transition().data()->setStartPixmap( transition().data()->currentPixmap() );
      transition().data()->show();
      transition().data()->raise();
      return true;
      }

//___________________________________________________________________
bool ComboBoxData::animate( void ) {

      // check enability
      if ( !enabled() ) return false;

      // grab
      setRecursiveCheck( true );
      transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) );
      setRecursiveCheck( false );

      // start animation
      transition().data()->animate();

      return true;

      }

//___________________________________________________________________
void ComboBoxData::targetDestroyed( void ) {
      setEnabled( false );
      target_.clear();
      }

