//////////////////////////////////////////////////////////////////////////////
// oxygenanimation.h
// stores event filters and maps widgets to animations for animations
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

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

class Animation: public QPropertyAnimation {

            Q_OBJECT

      public:

            //! TimeLine shared pointer
            typedef QPointer<Animation> Pointer;

            //! constructor
            Animation( int duration, QObject* parent ):
                  QPropertyAnimation( parent ) {
                  setDuration( duration );
                  }

            //! destructor
            virtual ~Animation( void )
                  {}

            //! true if running
            bool isRunning( void ) const {
                  return state() == Animation::Running;
                  }

            //! restart
            void restart( void ) {
                  if ( isRunning() ) stop();
                  start();
                  }

      };


#endif
