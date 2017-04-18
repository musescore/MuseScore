//////////////////////////////////////////////////////////////////////////////
// oxygenspinboxdata.cpp
// spinbox data container for up/down arrow hover (mouse-over) animations
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

#include "spinboxdata.h"

//________________________________________________
SpinBoxData::SpinBoxData( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ) {
      upArrowData_.animation_ = new Animation( duration, this );
      downArrowData_.animation_ = new Animation( duration, this );
      setupAnimation( upArrowAnimation(), "upArrowOpacity" );
      setupAnimation( downArrowAnimation(), "downArrowOpacity" );
      }

//______________________________________________
bool SpinBoxData::Data::updateState( bool value ) {
      if ( state_ == value ) return false;
      else {

            state_ = value;
            animation_.data()->setDirection( state_ ? Animation::Forward : Animation::Backward );
            if ( !animation_.data()->isRunning() ) animation_.data()->start();
            return true;

            }
      }
