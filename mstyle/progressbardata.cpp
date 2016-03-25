//////////////////////////////////////////////////////////////////////////////
// oxygenprogressbar.cpp
// data container for progressbar animations
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

#include <assert.h>
#include "progressbardata.h"

//______________________________________________
ProgressBarData::ProgressBarData( QObject* parent, QWidget* target, int duration ):
      GenericData( parent, target, duration ),
      startValue_(0),
      endValue_(0) {

      target->installEventFilter( this );

      // set animation curve shape
      animation().data()->setEasingCurve( QEasingCurve::InOutQuad );

      // make sure target is a progressbar and store relevant values
      QProgressBar* progress = qobject_cast<QProgressBar*>( target );
      Q_ASSERT( progress );
      setStartValue( progress->value() );
      setEndValue( progress->value() );

      // setup connections
      connect( target, SIGNAL( valueChanged( int ) ), SLOT( valueChanged( int ) ) );

      }

//______________________________________________
bool ProgressBarData::eventFilter( QObject* object, QEvent* event ) {

      if ( !( enabled() && object && object == target().data() ) ) return AnimationData::eventFilter( object, event );
      switch ( event->type() ) {
            case QEvent::Show: {

                  // reset start and end value
                  QProgressBar* progress = static_cast<QProgressBar*>( target().data() );
                  setStartValue( progress->value() );
                  setEndValue( progress->value() );
                  break;

                  }

            case QEvent::Hide: {
                  if ( animation().data()->isRunning() ) {
                        animation().data()->stop();
                        }
                  break;
                  }

            default:
                  break;

            }

      return AnimationData::eventFilter( object, event );

      }

//______________________________________________
void ProgressBarData::valueChanged( int value ) {

      // do nothing if not enabled
      if ( !enabled() ) return;

      // do nothing if progress is invalid
      QProgressBar* progress = static_cast<QProgressBar*>( target().data() );
      if ( !( progress && progress->maximum() != progress->minimum() ) ) return;

      // update start and end values
      bool isRunning( animation().data()->isRunning() );
      if ( isRunning ) {

            // in case next value arrives while animation is running,
            // end animation, set value immediately
            // and trigger target update. This increases responsiveness of progressbars
            setStartValue( value );
            setEndValue( value );
            animation().data()->stop();
            setOpacity(0);

            if ( target() ) target().data()->update();

            return;

            }

      setStartValue( endValue() );
      setEndValue( value );

      // start animation only if target is enabled, visible, not running,
      // and if end and start values are different enough
      // (with end value being larger than start value)
      if ( !(target() && target().data()->isEnabled() && target().data()->isVisible()) ) return;
      if ( isRunning || endValue() - startValue() < 2 ) return;

      animation().data()->start();

      }
