// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// oxygenheaderviewdata.cpp
// data container for QHeaderView animations
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

#include "headerviewdata.h"

//______________________________________________
HeaderViewData::HeaderViewData( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ) {

      target->installEventFilter( this );

      current_.animation_ = new Animation( duration, this );
      setupAnimation( currentIndexAnimation(), "currentOpacity" );
      currentIndexAnimation().data()->setDirection( Animation::Forward );

      previous_.animation_ = new Animation( duration, this );
      setupAnimation( previousIndexAnimation(), "previousOpacity" );
      previousIndexAnimation().data()->setDirection( Animation::Backward );

      }

//______________________________________________
bool HeaderViewData::updateState( const QPoint& position , bool hovered ) {

      if ( !enabled() ) return false;

      const QHeaderView* local( qobject_cast<const QHeaderView*>( target().data() ) );
      if ( !local ) return false;

      int index( local->logicalIndexAt( position ) );
      if ( index < 0 ) return false;

      if ( hovered ) {


            if ( index != currentIndex() ) {

                  if ( currentIndex() >= 0 ) {
                        setPreviousIndex( currentIndex() );
                        setCurrentIndex( -1 );
                        previousIndexAnimation().data()->restart();
                        }

                  setCurrentIndex( index );

                  /*
                  for now animation is only triggered when hovering the sorting section,
                  since no hover effect is implemented for all other sections
                  */
                  if ( index == local->sortIndicatorSection() ) {
                        currentIndexAnimation().data()->restart();
                        }

                  return true;

                  }
            else return false;

            }
      else if ( index == currentIndex() ) {

            setPreviousIndex( currentIndex() );
            setCurrentIndex( -1 );
            previousIndexAnimation().data()->restart();
            return true;

            }
      else return false;

      }

//______________________________________________
Animation::Pointer HeaderViewData::animation( const QPoint& position ) const {

      if ( !enabled() )  return Animation::Pointer();

      const QHeaderView* local( qobject_cast<const QHeaderView*>( target().data() ) );
      if ( !local ) return Animation::Pointer();

      int index( local->logicalIndexAt( position ) );
      if ( index < 0 ) return Animation::Pointer();
      else if ( index == currentIndex() ) return currentIndexAnimation();
      else if ( index == previousIndex() ) return previousIndexAnimation();
      else return Animation::Pointer();

      }

//______________________________________________
qreal HeaderViewData::opacity( const QPoint& position ) const {

      if ( !enabled() ) return OpacityInvalid;

      const QHeaderView* local( qobject_cast<const QHeaderView*>( target().data() ) );
      if ( !local ) return OpacityInvalid;

      int index( local->logicalIndexAt( position ) );
      if ( index < 0 ) return OpacityInvalid;
      else if ( index == currentIndex() ) return currentOpacity();
      else if ( index == previousIndex() ) return previousOpacity();
      else return OpacityInvalid;

      }
