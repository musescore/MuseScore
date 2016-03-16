#ifndef oxygentransitiondata_h
#define oxygentransitiondata_h

//////////////////////////////////////////////////////////////////////////////
// oxygentransitiondata.h
// data container for generic transitions
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

#include "transitionwidget.h"

//! generic data
class TransitionData: public QObject {

            Q_OBJECT

      public:

            //! constructor
            TransitionData( QObject* parent, QWidget* target, int );

            //! destructor
            virtual ~TransitionData( void );

            //! enability
            virtual void setEnabled( bool value ) {
                  enabled_ = value;
                  }

            //! enability
            virtual bool enabled( void ) const {
                  return enabled_;
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  transition().data()->setDuration( duration );
                  }

            //! max render time
            void setMaxRenderTime( int value ) {
                  maxRenderTime_ = value;
                  }

            //! max renderTime
            const int& maxRenderTime( void ) const {
                  return maxRenderTime_;
                  }

            //! start clock
            void startClock( void ) {
                  if ( clock_.isNull() ) clock_.start();
                  else clock_.restart();
                  }

            //! check if rendering is two slow
            bool slow( void ) const {
                  return !( clock_.isNull() || clock_.elapsed() <= maxRenderTime() );
                  }

      protected slots:

            //! initialize animation
            virtual bool initializeAnimation( void ) = 0;

            //! animate
            virtual bool animate( void ) = 0;

            //! finish animation
            virtual void finishAnimation( void ) {
                  if ( transition() ) {
                        transition().data()->hide();
                        }
                  }

      protected:

            //! transition widget
            virtual const TransitionWidget::Pointer& transition( void ) const {
                  return transition_;
                  }

            //! used to avoid recursion when grabbing widgets
            void setRecursiveCheck( bool value ) {
                  recursiveCheck_ = value;
                  }

            //! used to avoid recursion when grabbing widgets
            bool recursiveCheck( void ) const {
                  return recursiveCheck_;
                  }

      private:

            //! enability
            bool enabled_;

            //! used to avoid recursion when grabbing widgets
            bool recursiveCheck_;

            //! timer used to detect slow rendering
            QTime clock_;

            //! max render time
            /*! used to detect slow rendering */
            int maxRenderTime_;

            //! animation handling
            TransitionWidget::Pointer transition_;

      };

#endif

