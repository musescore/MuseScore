//////////////////////////////////////////////////////////////////////////////
// oxygenmenubardata.cpp
// data container for QMenuBar animations
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

#include "menubardata.h"

//______________________________________________
MenuBarDataV1::MenuBarDataV1( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ) {

      target->installEventFilter( this );

      // setup timeLine
      current_.animation_ = new Animation( duration, this );
      setupAnimation( currentAnimation(), "currentOpacity" );
      currentAnimation().data()->setDirection( Animation::Forward );

      previous_.animation_ = new Animation( duration, this );
      setupAnimation( previousAnimation(), "previousOpacity" );
      previousAnimation().data()->setDirection( Animation::Backward );

      }

//______________________________________________
bool MenuBarDataV1::eventFilter( QObject* object, QEvent* event ) {

      if ( !( enabled() && object == target().data() ) ) {
            return AnimationData::eventFilter( object, event );
            }

      // check event type
      switch ( event->type() ) {

            case QEvent::Enter: {
                  // first need to call proper event processing
                  // then implement transition
                  object->event( event );
                  enterEvent( object );
                  return true;
                  }

            case QEvent::Leave: {
                  // first need to call proper event processing
                  // then implement transition
                  object->event( event );
                  leaveEvent( object );
                  return true;
                  }

            case QEvent::MouseMove: {
                  // first need to call proper event processing
                  // then implement transition
                  object->event( event );
                  mouseMoveEvent( object );
                  return true;
                  }

            case QEvent::MouseButtonPress: {
                  // first need to call proper event processing
                  // then implement transition
                  mousePressEvent( object );
                  break;
                  }

            default:
                  break;

            }

      // always forward event
      return AnimationData::eventFilter( object, event );

      }

//______________________________________________
MenuBarDataV2::MenuBarDataV2( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ),
      opacity_(0),
      progress_(0) {

      target->installEventFilter( this );

      animation_ = new Animation( duration, this );
      animation().data()->setDirection( Animation::Forward );
      animation().data()->setStartValue( 0.0 );
      animation().data()->setEndValue( 1.0 );
      animation().data()->setTargetObject( this );
      animation().data()->setPropertyName( "opacity" );

      progressAnimation_ = new Animation( duration, this );
      progressAnimation().data()->setDirection( Animation::Forward );
      progressAnimation().data()->setStartValue( 0 );
      progressAnimation().data()->setEndValue( 1 );
      progressAnimation().data()->setTargetObject( this );
      progressAnimation().data()->setPropertyName( "progress" );
      progressAnimation().data()->setEasingCurve( QEasingCurve::Linear );

      }

//______________________________________________
bool MenuBarDataV2::eventFilter( QObject* object, QEvent* event ) {

      if ( !enabled() ) return false;

      // check event type
      switch ( event->type() ) {

            case QEvent::Enter: {
                  object->event( event );
                  enterEvent( object );
                  return true;
                  }

            case QEvent::Hide:
            case QEvent::Leave: {
                  object->event( event );
                  if ( timer_.isActive() ) timer_.stop();
                  timer_.start( 100, this );
                  return true;
                  }

            case QEvent::MouseMove: {
                  object->event( event );
                  mouseMoveEvent( object );
                  return true;
                  }

            default:
                  break;

            }

      // always forward event
      return false;

      }

//____________________________________________________________
void MenuBarDataV2::updateAnimatedRect( void ) {

      // check rect validity
      if ( !( currentRect().isValid() && previousRect().isValid() ) ) {
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

//___________________________________________________________
void MenuBarDataV2::timerEvent( QTimerEvent* event ) {

      if ( event->timerId() != timer_.timerId() ) return AnimationData::timerEvent( event );
      timer_.stop();
      leaveEvent( target().data() );
      }
