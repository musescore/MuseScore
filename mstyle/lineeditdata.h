#ifndef oxygenlineeditdata_h
#define oxygenlineeditdata_h

//////////////////////////////////////////////////////////////////////////////
// oxygenlineeditdata.h
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

#include "transitiondata.h"

//! generic data
class LineEditData: public TransitionData {

            Q_OBJECT

      public:

            //! constructor
            LineEditData( QObject*, QLineEdit*, int );

            //! destructor
            virtual ~LineEditData( void )
                  {}

            //! event filter
            bool eventFilter( QObject*, QEvent* );

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

            //! text edited
            virtual void textEdited( void );

            //! selection changed
            virtual void selectionChanged( void );

            //! text changed
            virtual void textChanged( void );

            //! initialize animation
            virtual bool initializeAnimation( void );

            //! animate
            virtual bool animate( void );

            //! called when target is destroyed
            virtual void targetDestroyed( void );

      protected:

            //! timer event
            virtual void timerEvent( QTimerEvent* );

            //! target rect
            /*! return rect corresponding to the area to be updated when animating */
            QRect targetRect( void ) const {
                  if ( !target_ ) return QRect();
                  QRect out( target_.data()->rect() );
                  if ( hasClearButton_ && clearButtonRect_.isValid() ) {
                        out.setRight( clearButtonRect_.left() );
                        }

                  return out;
                  }

            //! check if target has clear button
            void checkClearButton( void );

      private:

            //! lock time (milliseconds
            static const int lockTime_;

            //! timer used to disable animations when triggered too early
            QBasicTimer animationLockTimer_;

            //! needed to start animations out of parent paintEvent
            QBasicTimer timer_;

            //! target
            QPointer<QLineEdit> target_;

            //! true if target has clean button
            bool hasClearButton_;

            //! clear button rect
            QRect clearButtonRect_;

            //! true if text was manually edited
            /*! needed to trigger animation only on programatically enabled text */
            bool edited_;

            //! old text
            QString text_;

            //! widget rect
            /*! needed to properly handle QLabel geometry changes */
            QRect widgetRect_;

      };

#endif
