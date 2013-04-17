//////////////////////////////////////////////////////////////////////////////
// oxygenmdiwindowdata.cpp
// mdi window data container for window titlebar buttons
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

#include "mdiwindowdata.h"

//________________________________________________
MdiWindowData::MdiWindowData( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ) {
      currentData_.animation_ = new Animation( duration, this );
      previousData_.animation_ = new Animation( duration, this );
      setupAnimation( currentAnimation(), "currentOpacity" );
      setupAnimation( previousAnimation(), "previousOpacity" );

      currentAnimation().data()->setDirection( Animation::Forward );
      previousAnimation().data()->setDirection( Animation::Backward );
      }

//______________________________________________
bool MdiWindowData::updateState( int primitive, bool state ) {

      if ( state ) {

            if ( primitive != currentData_.primitive_ ) {

                  previousData_.updateSubControl( currentData_.primitive_ );
                  currentData_.updateSubControl( primitive );
                  return true;

                  }
            else return false;

            }
      else {

            bool changed( false );
            if ( primitive == currentData_.primitive_ ) {
                  changed |= currentData_.updateSubControl( 0 );
                  changed |= previousData_.updateSubControl( primitive );
                  }

            return changed;

            }

      }

//______________________________________________
bool MdiWindowData::Data::updateSubControl( int value ) {
      if ( primitive_ == value ) return false;
      else {

            primitive_ = value;
            if ( animation_.data()->isRunning() ) animation_.data()->stop();
            if ( primitive_ != 0 ) animation_.data()->start();
            return true;

            }
      }
