//////////////////////////////////////////////////////////////////////////////
// oxygenmenubardata.h
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

#ifndef __MENUBARDATA_H__
#define __MENUBARDATA_H__

#include "animationdata.h"

//! widget index
enum WidgetIndex {
      Current,
      Previous
      };

//! menubar data
class MenuBarDataV1: public AnimationData {

            Q_OBJECT

            //! declare opacity property
            Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
            Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

      public:

            //! constructor
            MenuBarDataV1( QObject* parent, QWidget* target, int duration );

            //! destructor
            virtual ~MenuBarDataV1( void )
                  {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            //! animations
            virtual const Animation::Pointer& currentAnimation( void ) const {
                  return current_.animation_;
                  }

            //! animations
            virtual const Animation::Pointer& previousAnimation( void ) const {
                  return previous_.animation_;
                  }

            //! return animation matching given point
            virtual Animation::Pointer animation( const QPoint& point ) const {
                  if ( currentRect().contains( point ) ) return currentAnimation();
                  else if ( previousRect().contains( point ) ) return previousAnimation();
                  else return Animation::Pointer();
                  }

            //! return animation matching given point
            virtual qreal opacity( const QPoint& point ) const {
                  if ( currentRect().contains( point ) ) return currentOpacity();
                  else if ( previousRect().contains( point ) ) return previousOpacity();
                  else return OpacityInvalid;
                  }

            // return rect matching QPoint
            virtual QRect currentRect( const QPoint& point ) const {
                  if ( currentRect().contains( point ) ) return currentRect();
                  else if ( previousRect().contains( point ) ) return previousRect();
                  else return QRect();
                  }

            //! animation associated to given Widget index
            virtual const Animation::Pointer& animation( WidgetIndex index ) const {
                  return index == Current ? currentAnimation() : previousAnimation();
                  }

            //! opacity associated to given Widget index
            virtual qreal opacity( WidgetIndex index ) const {
                  return index == Current ? currentOpacity() : previousOpacity();
                  }

            //! opacity associated to given Widget index
            virtual const QRect& currentRect( WidgetIndex index ) const {
                  return index == Current ? currentRect() : previousRect();
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  currentAnimation().data()->setDuration( duration );
                  previousAnimation().data()->setDuration( duration );
                  }

            //! current opacity
            virtual qreal currentOpacity( void ) const {
                  return current_.opacity_;
                  }

            //! current opacity
            virtual void setCurrentOpacity( qreal value ) {
                  if ( current_.opacity_ == value ) return;
                  current_.opacity_ = value;
                  setDirty();
                  }

            //! current rect
            virtual const QRect& currentRect( void ) const {
                  return current_.rect_;
                  }

            //! previous opacity
            virtual qreal previousOpacity( void ) const {
                  return previous_.opacity_;
                  }

            //! previous opacity
            virtual void setPreviousOpacity( qreal value ) {
                  if ( previous_.opacity_ == value ) return;
                  previous_.opacity_ = value;
                  setDirty();
                  }

            //! previous rect
            virtual const QRect& previousRect( void ) const {
                  return previous_.rect_;
                  }

      protected:

            //!@name current action handling
            //@{

            //! guarded action pointer
            typedef QPointer<QAction> ActionPointer;

            //! current action
            virtual const ActionPointer& currentAction( void ) const {
                  return currentAction_;
                  }

            //! current action
            virtual void setCurrentAction( QAction* action ) {
                  currentAction_ = ActionPointer( action );
                  }

            //! current action
            virtual void clearCurrentAction( void ) {
                  currentAction_ = ActionPointer();
                  }

            //@}

            //!@name rect handling
            //@{

            //! current rect
            virtual void setCurrentRect( const QRect& rect ) {
                  current_.rect_ = rect;
                  }

            //! current rect
            virtual void clearCurrentRect( void ) {
                  current_.rect_ = QRect();
                  }

            //! previous rect
            virtual void setPreviousRect( const QRect& rect ) {
                  previous_.rect_ = rect;
                  }

            //! previous rect
            virtual void clearPreviousRect( void ) {
                  previous_.rect_ = QRect();
                  }

            //@}

            // leave event
            template< typename T > inline void enterEvent( const QObject* object );

            // leave event
            template< typename T > inline void leaveEvent( const QObject* object );

            //! mouse move event
            template< typename T > inline void mouseMoveEvent( const QObject* object );

            //! mouse move event
            template< typename T > inline void mousePressEvent( const QObject* object );

            //! menubar enterEvent
            virtual void enterEvent( const QObject* object ) {
                  enterEvent<QMenuBar>( object );
                  }

            //! menubar enterEvent
            virtual void leaveEvent( const QObject* object ) {
                  leaveEvent<QMenuBar>( object );
                  }

            //! menubar mouseMoveEvent
            virtual void mouseMoveEvent( const QObject* object ) {
                  mouseMoveEvent<QMenuBar>( object );
                  }

            //! menubar mousePressEvent
            virtual void mousePressEvent( const QObject* object ) {
                  mousePressEvent<QMenuBar>( object );
                  }

      private:

            //! container for needed animation data
            class Data {
                  public:

                        //! default constructor
                        Data( void ):
                              opacity_(0)
                              {}

                        Animation::Pointer animation_;
                        qreal opacity_;
                        QRect rect_;
                  };

            //! current tab animation data (for hover enter animations)
            Data current_;

            //! previous tab animations data (for hover leave animations)
            Data previous_;

            //! current action
            ActionPointer currentAction_;

      };


//! menubar data
class MenuBarDataV2: public AnimationData {

            Q_OBJECT
            Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )
            Q_PROPERTY( qreal progress READ progress  WRITE setProgress )

      public:

            //! constructor
            MenuBarDataV2( QObject* parent, QWidget* target, int duration );

            //! destructor
            virtual ~MenuBarDataV2( void )
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

      protected:

            //! animated rect
            virtual void clearAnimatedRect( void ) {
                  animatedRect_ = QRect();
                  }

            //! updated animated rect
            virtual void updateAnimatedRect( void );

            //! timer event
            virtual void timerEvent( QTimerEvent* );

            //!@name current action handling
            //@{

            //! guarded action pointer
            typedef QPointer<QAction> ActionPointer;

            //! current action
            virtual const ActionPointer& currentAction( void ) const {
                  return currentAction_;
                  }

            //! current action
            virtual void setCurrentAction( QAction* action ) {
                  currentAction_ = ActionPointer( action );
                  }

            //! current action
            virtual void clearCurrentAction( void ) {
                  currentAction_ = ActionPointer();
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

            //@}

            // leave event
            template< typename T > inline void enterEvent( const QObject* object );

            // leave event
            template< typename T > inline void leaveEvent( const QObject* object );

            //! mouse move event
            template< typename T > inline void mouseMoveEvent( const QObject* object );

            //! menubar enterEvent
            virtual void enterEvent( const QObject* object ) {
                  enterEvent<QMenuBar>( object );
                  }

            //! menubar enterEvent
            virtual void leaveEvent( const QObject* object ) {
                  leaveEvent<QMenuBar>( object );
                  }

            //! menubar mouseMoveEvent
            virtual void mouseMoveEvent( const QObject* object ) {
                  mouseMoveEvent<QMenuBar>( object );
                  }

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

            //! current action
            ActionPointer currentAction_;

            // current rect
            QRect currentRect_;

            // previous rect
            QRect previousRect_;

            // animated rect
            QRect animatedRect_;

      };

#include "menubardata_imp.h"
#endif

