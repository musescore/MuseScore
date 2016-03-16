//////////////////////////////////////////////////////////////////////////////
// oxygenprogressbarengine.cpp
// handle progress bar animations
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

#include "progressbarengine.h"

//_______________________________________________
bool ProgressBarEngine::registerWidget( QWidget* widget ) {

      // check enability
      if ( !widget ) return false;

      // create new data class
      if ( !data_.contains( widget ) ) data_.insert( widget, new ProgressBarData( this, widget, duration() ), enabled() );
      if ( busyIndicatorEnabled() && !dataSet_.contains( widget ) ) dataSet_.insert( widget );

      // connect destruction signal
      connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ), Qt::UniqueConnection );

      return true;

      }

//____________________________________________________________
bool ProgressBarEngine::isAnimated( const QObject* object ) {

      DataMap<ProgressBarData>::Value data( ProgressBarEngine::data( object ) );
      return ( data && data.data()->animation() && data.data()->animation().data()->isRunning() );

      }

//____________________________________________________________
void ProgressBarEngine::setBusyStepDuration( int value ) {
      if ( busyStepDuration_ == value ) return;
      busyStepDuration_ = value;

      // restart timer with specified time
      if ( timer_.isActive() ) {
            timer_.stop();
            timer_.start( busyStepDuration(), this );
            }

      }

//_______________________________________________
void ProgressBarEngine::timerEvent( QTimerEvent* event ) {

      // check enability and timer
      if ( !(busyIndicatorEnabled() && event->timerId() == timer_.timerId() ) ) {
            return BaseEngine::timerEvent( event );
            }

      bool animated( false );

      // loop over objects in map
      for ( ProgressBarSet::iterator iter = dataSet_.begin(); iter != dataSet_.end(); ++iter ) {

            // cast to progressbar
            QProgressBar* progressBar( qobject_cast<QProgressBar*>( *iter ) );

            // check cast, visibility and range
            if ( progressBar && progressBar->isVisible() && progressBar->minimum() == 0 && progressBar->maximum() == 0  ) {

                  // update animation flag
                  animated = true;

                  // update value
                  progressBar->setValue(progressBar->value() + 1);
                  progressBar->update();

                  }
            }

      if ( !animated ) timer_.stop();

      }

//____________________________________________________________
DataMap<ProgressBarData>::Value ProgressBarEngine::data( const QObject* object ) {
      return data_.find( object ).data();
      }

