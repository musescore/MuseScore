//////////////////////////////////////////////////////////////////////////////
// oxygenmenubardata_imp.h
// implements menubar data templatized methods
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

#ifndef __MENUBARDATA_IMP_H__
#define __MENUBARDATA_IMP_H__

//________________________________________________________________________
template< typename T > void MenuBarDataV1::enterEvent( const QObject* object ) {

      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      // if the current action is still active, one does nothing
      if ( local->activeAction() == currentAction().data() ) return;

      if ( currentAnimation().data()->isRunning() ) currentAnimation().data()->stop();
      clearCurrentAction();
      clearCurrentRect();

      }

//________________________________________________________________________
template< typename T > void MenuBarDataV1::leaveEvent( const QObject* object ) {

      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      // if the current action is still active, one does nothing
      if ( local->activeAction() == currentAction().data() ) return;

      if ( currentAnimation().data()->isRunning() ) currentAnimation().data()->stop();
      if ( previousAnimation().data()->isRunning() ) previousAnimation().data()->stop();
      if ( currentAction() ) {
            setPreviousRect( currentRect() );
            clearCurrentAction();
            clearCurrentRect();
            previousAnimation().data()->start();
            }

      }

//________________________________________________________________________
template< typename T > void MenuBarDataV1::mouseMoveEvent( const QObject* object ) {

      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      // check action
      if ( local->activeAction() == currentAction().data() ) return;

      bool hasCurrentAction( currentAction() );

      // check current action
      if ( currentAction() ) {
            if ( currentAnimation().data()->isRunning() ) currentAnimation().data()->stop();
            if ( previousAnimation().data()->isRunning() ) previousAnimation().data()->stop();

            // only start fadeout effect if there is no new selected action
            //if( !activeActionValid )
            if ( !local->activeAction() ) {
                  setPreviousRect( currentRect() );
                  previousAnimation().data()->start();
                  }

            clearCurrentAction();
            clearCurrentRect();

            }

      // check if local current actions is valid
      bool activeActionValid( local->activeAction() && local->activeAction()->isEnabled() && !local->activeAction()->isSeparator() );
      if ( activeActionValid ) {
            if ( currentAnimation().data()->isRunning() ) currentAnimation().data()->stop();

            setCurrentAction( local->activeAction() );
            setCurrentRect( local->actionGeometry( currentAction().data() ) );
            if ( !hasCurrentAction ) {
                  currentAnimation().data()->start();
                  }

            }

      }

//________________________________________________________________________
template< typename T > void MenuBarDataV1::mousePressEvent( const QObject* object ) {

      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      // check action
      if ( local->activeAction() == currentAction().data() ) return;

      // check current action
      bool activeActionValid( local->activeAction() && local->activeAction()->isEnabled() && !local->activeAction()->isSeparator() );
      if ( currentAction() && !activeActionValid ) {

            if ( currentAnimation().data()->isRunning() ) currentAnimation().data()->stop();
            if ( previousAnimation().data()->isRunning() ) previousAnimation().data()->stop();

            setPreviousRect( currentRect() );
            previousAnimation().data()->start();

            clearCurrentAction();
            clearCurrentRect();

            }

      }

//________________________________________________________________________
template< typename T > void MenuBarDataV2::enterEvent( const QObject* object ) {

      // cast widget
      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      if ( timer_.isActive() ) timer_.stop();

      // if the current action is still active, one does nothing
      if ( currentAction() && local->activeAction() == currentAction().data() ) return;

      if ( animation().data()->isRunning() ) animation().data()->stop();
      if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
      clearPreviousRect();
      clearAnimatedRect();

      if ( local->activeAction() &&  local->activeAction()->isEnabled() && !local->activeAction()->isSeparator() ) {
            setCurrentAction( local->activeAction() );
            setCurrentRect( local->actionGeometry( currentAction().data() ) );
            animation().data()->setDirection( Animation::Forward );
            animation().data()->start();

            }
      else {

            clearCurrentAction();
            clearCurrentRect();

            }

      return;
      }

//________________________________________________________________________
template< typename T > void MenuBarDataV2::leaveEvent( const QObject* object ) {

      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;

      // if the current action is still active, one does nothing
      if ( local->activeAction() == currentAction().data() ) return;

      if ( animation().data()->isRunning() ) animation().data()->stop();
      if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
      clearAnimatedRect();
      clearPreviousRect();
      if ( currentAction() ) {
            clearCurrentAction();
            animation().data()->setDirection( Animation::Backward );
            animation().data()->start();
            }

      return;

      }

//________________________________________________________________________
template< typename T > void MenuBarDataV2::mouseMoveEvent( const QObject* object ) {
      const T* local = qobject_cast<const T*>( object );
      if ( !local ) return;
      if ( local->activeAction() == currentAction().data() ) return;

      // check if current position match another action
      if ( local->activeAction() && local->activeAction()->isEnabled() && !local->activeAction()->isSeparator()) {

            if ( timer_.isActive() ) timer_.stop();

            QAction* activeAction( local->activeAction() );

            // update previous rect if the current action is valid
            QRect activeRect( local->actionGeometry( activeAction ) );
            if ( currentAction() ) {
                  if ( !progressAnimation().data()->isRunning() ) {

                        setPreviousRect( currentRect() );

                        }
                  else if ( progress() < 1 && currentRect().isValid() && previousRect().isValid() ) {

                        // re-calculate previous rect so that animatedRect
                        // is unchanged after currentRect is updated
                        // this prevents from having jumps in the animation
                        qreal ratio = progress() / (1.0 - progress());
                        previousRect_.adjust(
                              ratio * ( currentRect().left() - activeRect.left() ),
                              ratio * ( currentRect().top() - activeRect.top() ),
                              ratio * ( currentRect().right() - activeRect.right() ),
                              ratio * ( currentRect().bottom() - activeRect.bottom() ) );

                        }

                  // update current action
                  setCurrentAction( activeAction );
                  setCurrentRect( activeRect );
                  if ( animation().data()->isRunning() ) animation().data()->stop();
                  if ( !progressAnimation().data()->isRunning() ) progressAnimation().data()->start();

                  }
            else {

                  // update current action
                  setCurrentAction( activeAction );
                  setCurrentRect( activeRect );
                  setPreviousRect( activeRect );

                  clearAnimatedRect();
                  if ( progressAnimation().data()->isRunning() ) progressAnimation().data()->stop();
                  animation().data()->setDirection( Animation::Forward );

                  if ( !animation().data()->isRunning() ) animation().data()->start();

                  }

            }
      else if ( currentAction() ) {

            timer_.start( 150, this );

            }

      return;

      }

#endif
