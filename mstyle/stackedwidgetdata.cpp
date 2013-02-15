//////////////////////////////////////////////////////////////////////////////
// oxygenstackedwidgetdata.cpp
// data container for QStackedWidget transition
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

#include "stackedwidgetdata.h"

//______________________________________________________
StackedWidgetData::StackedWidgetData( QObject* parent, QStackedWidget* target, int duration ):
      TransitionData( parent, target, duration ),
      target_( target ),
      index_( target->currentIndex() ) {

      // configure transition
      connect( target_.data(), SIGNAL( destroyed() ), SLOT( targetDestroyed() ) );
      connect( target_.data(), SIGNAL( currentChanged( int ) ), SLOT( animate() ) );

      // disable focus
      transition().data()->setAttribute(Qt::WA_NoMousePropagation, true);
      transition().data()->setFlag(TransitionWidget::PaintOnWidget, true);

      setMaxRenderTime( 50 );

      }

//___________________________________________________________________
bool StackedWidgetData::initializeAnimation( void ) {

      // check enability
      if ( !( target_ && target_.data()->isVisible() ) ) {
            return false;
            }

      // check index
      if ( target_.data()->currentIndex() == index_ ) {
            return false;
            }

      // do not animate if either index or currentIndex is not valid
      // but update index_ none the less
      if ( target_.data()->currentIndex() < 0 || index_ < 0 ) {
            index_ = target_.data()->currentIndex();
            return false;
            }

      // get old widget (matching index_) and initialize transition
      if ( QWidget* widget = target_.data()->widget( index_ ) ) {

            transition().data()->setOpacity( 0 );
            startClock();
            transition().data()->setGeometry( widget->geometry() );
            transition().data()->setStartPixmap( transition().data()->grab( widget ) );

            index_ = target_.data()->currentIndex();
            return !slow();

            }
      else {

            index_ = target_.data()->currentIndex();
            return false;

            }

      }

//___________________________________________________________________
bool StackedWidgetData::animate( void ) {

      // check enability
      if ( !enabled() ) return false;

      // initialize animation
      if ( !initializeAnimation() ) return false;

      // show transition widget
      transition().data()->show();
      transition().data()->raise();
      transition().data()->animate();
      return true;

      }

//___________________________________________________________________
void StackedWidgetData::finishAnimation( void ) {
      // disable updates on currentWidget
      if ( target_ && target_.data()->currentWidget() ) {
            target_.data()->currentWidget()->setUpdatesEnabled( false );
            }

      // hide transition
      transition().data()->hide();

      // reenable updates and repaint
      if ( target_ && target_.data()->currentWidget() ) {
            target_.data()->currentWidget()->setUpdatesEnabled( true );
            target_.data()->currentWidget()->repaint();
            }

      // invalidate start widget
      transition().data()->resetStartPixmap();

      }

//___________________________________________________________________
void StackedWidgetData::targetDestroyed( void ) {
      setEnabled( false );
      target_.clear();
      }

