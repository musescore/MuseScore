//////////////////////////////////////////////////////////////////////////////
// oxygenscrollbardata.cpp
// data container for QTabBar animations
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

#include "scrollbardata.h"

Q_GUI_EXPORT QStyleOptionSlider qt_qscrollbarStyleOption(QScrollBar*);

//______________________________________________
ScrollBarData::ScrollBarData( QObject* parent, QWidget* target, int duration ):
      SliderData( parent, target, duration ) {

      target->installEventFilter( this );

      addLineData_.animation_ = new Animation( duration, this );
      subLineData_.animation_ = new Animation( duration, this );

      connect( addLineAnimation().data(), SIGNAL( finished( void ) ), SLOT( clearAddLineRect( void ) ) );
      connect( subLineAnimation().data(), SIGNAL( finished( void ) ), SLOT( clearSubLineRect( void ) ) );

      // setup animation
      setupAnimation( addLineAnimation(), "addLineOpacity" );
      setupAnimation( subLineAnimation(), "subLineOpacity" );

      }

//______________________________________________
bool ScrollBarData::eventFilter( QObject* object, QEvent* event ) {

      if ( object != target().data() ) {
            return SliderData::eventFilter( object, event );
            }

      // check event type
      switch ( event->type() ) {

            case QEvent::HoverEnter:
            case QEvent::HoverMove:
                  hoverMoveEvent( object, event );
                  break;

            case QEvent::HoverLeave:
                  hoverLeaveEvent( object, event );
                  break;

            default:
                  break;

            }

      return SliderData::eventFilter( object, event );

      }

//______________________________________________
const Animation::Pointer& ScrollBarData::animation( QStyle::SubControl subcontrol ) const {
      switch ( subcontrol ) {
            default:
            case QStyle::SC_ScrollBarSlider:
                  return animation();

            case QStyle::SC_ScrollBarAddLine:
                  return addLineAnimation();

            case QStyle::SC_ScrollBarSubLine:
                  return subLineAnimation();
            }

      }

//______________________________________________
qreal ScrollBarData::opacity( QStyle::SubControl subcontrol ) const {
      switch ( subcontrol ) {
            default:
            case QStyle::SC_ScrollBarSlider:
                  return opacity();

            case QStyle::SC_ScrollBarAddLine:
                  return addLineOpacity();

            case QStyle::SC_ScrollBarSubLine:
                  return subLineOpacity();
            }

      }

//______________________________________________
void ScrollBarData::hoverMoveEvent(  QObject* object, QEvent* event ) {

      // try cast object to scrollbar
      QScrollBar* scrollBar( qobject_cast<QScrollBar*>( object ) );
      if ( !scrollBar || scrollBar->isSliderDown() ) return;

      // retrieve scrollbar option
      QStyleOptionSlider opt( qt_qscrollbarStyleOption( qobject_cast<QScrollBar*>( object ) ) );

      // cast event
      QHoverEvent* he = static_cast<QHoverEvent*>(event);
      QStyle::SubControl hoverControl = scrollBar->style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, he->pos(), scrollBar);
      updateAddLineArrow( hoverControl );
      updateSubLineArrow( hoverControl );

      }


//______________________________________________
void ScrollBarData::hoverLeaveEvent(  QObject* object, QEvent* event ) {
      Q_UNUSED( object );
      Q_UNUSED( event );
      updateSubLineArrow( QStyle::SC_None );
      updateAddLineArrow( QStyle::SC_None );
      }

//_____________________________________________________________________
void ScrollBarData::updateSubLineArrow( QStyle::SubControl hoverControl ) {
      if ( hoverControl == QStyle::SC_ScrollBarSubLine ) {

            if ( !subLineArrowHovered() ) {
                  setSubLineArrowHovered( true );
                  if ( enabled() ) {
                        subLineAnimation().data()->setDirection( Animation::Forward );
                        if ( !subLineAnimation().data()->isRunning() ) subLineAnimation().data()->start();
                        }
                  else setDirty();
                  }

            }
      else {

            if ( subLineArrowHovered() ) {
                  setSubLineArrowHovered( false );
                  if ( enabled() ) {
                        subLineAnimation().data()->setDirection( Animation::Backward );
                        if ( !subLineAnimation().data()->isRunning() ) subLineAnimation().data()->start();
                        }
                  else setDirty();
                  }

            }
      }

//_____________________________________________________________________
void ScrollBarData::updateAddLineArrow( QStyle::SubControl hoverControl ) {
      if ( hoverControl == QStyle::SC_ScrollBarAddLine ) {

            if ( !addLineArrowHovered() ) {
                  setAddLineArrowHovered( true );
                  if ( enabled() ) {
                        addLineAnimation().data()->setDirection( Animation::Forward );
                        if ( !addLineAnimation().data()->isRunning() ) addLineAnimation().data()->start();
                        }
                  else setDirty();
                  }

            }
      else {

            if ( addLineArrowHovered() ) {
                  setAddLineArrowHovered( false );
                  if ( enabled() ) {
                        addLineAnimation().data()->setDirection( Animation::Backward );
                        if ( !addLineAnimation().data()->isRunning() ) addLineAnimation().data()->start();
                        }
                  else setDirty();
                  }

            }
      }

