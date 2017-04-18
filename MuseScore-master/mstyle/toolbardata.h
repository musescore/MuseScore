#ifndef oxygentoolbardata_h
#define oxygentoolbardata_h

//////////////////////////////////////////////////////////////////////////////
// oxygentoolbardata.h
// stores event filters and maps widgets to timelines for animations
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

#include "animationdata.h"

//! toolbar data
class ToolBarData: public AnimationData {

            Q_OBJECT
            Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )
            Q_PROPERTY( qreal progress READ progress  WRITE setProgress )

      public:

            //! constructor
            ToolBarData( QObject* parent, QWidget* target, int duration );

            //! destructor
            virtual ~ToolBarData( void )
                  {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            //! return animation associated to action at given position, if any
            virtual const Animation::Pointer& animation( void ) const {
                  return animation_;
                  }

            //! return animation associated to action at given position, if any
            virtual const Animation::Pointer& progressAnimation( void ) const {
                  return progressAnimation_;
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  animation().data()->setDuration( duration );
                  }

            //! duration
            virtual void setFollowMouseDuration( int duration ) {
                  progressAnimation().data()->setDuration( duration );
                  }

            //! return 'hover' rect position when widget is animated
            virtual const QRect& animatedRect( void ) const {
                  return animatedRect_;
                  }

            //! current rect
            virtual const QRect& currentRect( void ) const {
                  return currentRect_;
                  }

            //! timer
            const QBasicTimer& timer( void ) const {
                  return timer_;
                  }

            //! animation opacity
            virtual qreal opacity( void ) const {
                  return opacity_;
                  }

            //! animation opacity
            virtual void setOpacity( qreal value ) {
                  if ( opacity_ == value ) return;
                  opacity_ = value;
                  setDirty();
                  }

            //! animation progress
            virtual qreal progress( void ) const {
                  return progress_;
                  }

            //! animation progress
            virtual void setProgress( qreal value ) {
                  if ( progress_ == value ) return;
                  progress_ = value;
                  updateAnimatedRect();
                  }

      protected slots:

            //! updated animated rect
            virtual void updateAnimatedRect( void );

      protected:

            //! timer event
            virtual void timerEvent( QTimerEvent*);

            //!@name current object handling
            //@{

            //! object pointer
            /*! there is no need to guard it because the object contents is never accessed */
            typedef const QObject* ObjectPointer;

            //! current object
            virtual const ObjectPointer& currentObject( void ) const {
                  return currentObject_;
                  }

            //! current object
            virtual void setCurrentObject( const QObject* object ) {
                  currentObject_ = ObjectPointer( object );
                  }

            //! current object
            virtual void clearCurrentObject( void ) {
                  currentObject_ = NULL;
                  }

            //@}

            //!@name rect handling
            //@{

            //! current rect
            virtual void setCurrentRect( const QRect& rect ) {
                  currentRect_ = rect;
                  }

            //! current rect
            virtual void clearCurrentRect( void ) {
                  currentRect_ = QRect();
                  }

            //! previous rect
            virtual const QRect& previousRect( void ) const {
                  return previousRect_;
                  }

            //! previous rect
            virtual void setPreviousRect( const QRect& rect ) {
                  previousRect_ = rect;
                  }

            //! previous rect
            virtual void clearPreviousRect( void ) {
                  previousRect_ = QRect();
                  }

            //! animated rect
            virtual void clearAnimatedRect( void ) {
                  animatedRect_ = QRect();
                  }

            //@}

            //! toolbar enterEvent
            virtual void enterEvent( const QObject* );

            //! toolbar enterEvent
            virtual void leaveEvent( const QObject* );

            //! toolbutton added
            virtual void childAddedEvent( QObject* );

            //! toolbutton enter event
            virtual void childEnterEvent( const QObject* );

      private:

            //! fade animation
            Animation::Pointer animation_;

            //! progress animation
            Animation::Pointer progressAnimation_;

            //! opacity
            qreal opacity_;

            //! opacity
            qreal progress_;

            //! timer
            /*! this allows to add some delay before starting leaveEvent animation */
            QBasicTimer timer_;

            //! current object
            ObjectPointer currentObject_;

            //! current rect
            QRect currentRect_;

            //! previous rect
            QRect previousRect_;

            //! animated rect
            QRect animatedRect_;

            //! true if toolbar was entered at least once (this prevents some initialization glitches)
            bool entered_;

      };

#endif
