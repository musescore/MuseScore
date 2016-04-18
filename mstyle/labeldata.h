#ifndef oxygenlabeldata_h
#define oxygenlabeldata_h

//////////////////////////////////////////////////////////////////////////////
// oxygenlabeldata.h
// data container for QLabel transition
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

#include "transitiondata.h"

//! generic data
class LabelData: public TransitionData {

            Q_OBJECT

      public:

            //! constructor
            LabelData( QObject*, QLabel*, int );

            //! destructor
            virtual ~LabelData( void )
                  {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            //! returns true if animations are locked
            bool isLocked( void ) const {
                  return animationLockTimer_.isActive();
                  }

            //! start lock animation timer
            void lockAnimations( void ) {
                  animationLockTimer_.start( lockTime_, this );
                  }

            //! start lock animation timer
            void unlockAnimations( void ) {
                  animationLockTimer_.stop();
                  }

      protected slots:

            //! initialize animation
            virtual bool initializeAnimation( void );

            //! animate
            virtual bool animate( void );

            //! called when target is destroyed
            virtual void targetDestroyed( void );

      protected:

            //! timer event
            virtual void timerEvent( QTimerEvent* );

      private:

            //! lock time (milliseconds
            static const int lockTime_;

            //! timer used to disable animations when triggered too early
            QBasicTimer animationLockTimer_;

            //! needed to start animations out of parent paintEvent
            QBasicTimer timer_;

            //! target
            QPointer<QLabel> target_;

            //! old text
            QString text_;

            //! widget rect
            /*! needed to properly handle QLabel geometry changes */
            QRect widgetRect_;

      };

#endif
