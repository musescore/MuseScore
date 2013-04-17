//////////////////////////////////////////////////////////////////////////////
// oxygentransitionwidget.cpp
// stores event filters and maps widgets to transitions for transitions
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

#include "transitionwidget.h"

#include <cassert>

//________________________________________________
bool TransitionWidget::paintEnabled_ = true;
bool TransitionWidget::paintEnabled( void ) {
      return paintEnabled_;
      }

//________________________________________________
TransitionWidget::TransitionWidget( QWidget* parent, int duration ):
      QWidget( parent ),
      flags_( None ),
      animation_( new Animation( duration, this ) ),
      opacity_(0) {

      // background flags
      setAttribute(Qt::WA_NoSystemBackground );
      setAutoFillBackground( false );

      // setup animation
      animation_.data()->setStartValue( 0 );
      animation_.data()->setEndValue( 1.0 );
      animation_.data()->setTargetObject( this );
      animation_.data()->setPropertyName( "opacity" );

      // setup connections
      connect( animation_.data(), SIGNAL( finished( void ) ), SIGNAL( finished( void ) ) );

      }

//________________________________________________
QPixmap TransitionWidget::grab( QWidget* widget, QRect rect ) {

      // change rect
      if ( !rect.isValid() ) rect = widget->rect();
      if ( !rect.isValid() ) return QPixmap();

      // initialize pixmap
      QPixmap out( rect.size() );
      out.fill( Qt::transparent );
      paintEnabled_ = false;

      if ( testFlag( GrabFromWindow ) ) {

            rect = rect.translated( widget->mapTo( widget->window(), widget->rect().topLeft() ) );
            widget = widget->window();
            out = widget->grab(rect);
            // out = QPixmap::grabWidget( widget, rect );

            }
      else {

            if ( !testFlag( Transparent ) ) {
                  grabBackground( out, widget, rect );
                  }
            grabWidget( out, widget, rect );

            }

      paintEnabled_ = true;

      return out;

      }

//________________________________________________
void TransitionWidget::paintEvent( QPaintEvent* event ) {

      // fully transparent case
      if ( opacity() >= 1.0 && endPixmap().isNull() ) return;
      if ( !paintEnabled_ ) return;

      // get rect
      QRect rect = event->rect();
      if ( !rect.isValid() ) rect = this->rect();

      // local pixmap
      const bool paintOnWidget( testFlag( PaintOnWidget ) && !testFlag( Transparent ) );
      if ( !paintOnWidget ) {

            if ( currentPixmap_.isNull() || currentPixmap_.size() != size() ) {
                  currentPixmap_ = QPixmap( size() );
                  }

            }

      // fill
      currentPixmap_.fill( Qt::transparent );

      // copy local pixmap to current
            {

            QPainter p;

            // draw end pixmap first, provided that opacity is small enough
            if ( opacity() >= 0.004 && !endPixmap_.isNull() ) {

                  // faded endPixmap if parent target is transparent and opacity is
                  if ( opacity() <= 0.996 && testFlag( Transparent ) ) {

                        fade( endPixmap_, currentPixmap_, opacity(), rect );
                        p.begin( &currentPixmap_ );
                        p.setClipRect( event->rect() );

                        }
                  else {

                        if ( paintOnWidget ) p.begin( this );
                        else p.begin( &currentPixmap_ );
                        p.setClipRect( event->rect() );
                        p.drawPixmap( QPoint(), endPixmap_ );

                        }

                  }
            else {

                  if ( paintOnWidget ) p.begin( this );
                  else p.begin( &currentPixmap_ );
                  p.setClipRect( event->rect() );

                  }

            // draw fading start pixmap
            if ( opacity() <= 0.996 && !startPixmap_.isNull() ) {
                  if ( opacity() >= 0.004 ) {

                        fade( startPixmap_, localStartPixmap_, 1.0 - opacity(), rect );
                        p.drawPixmap( QPoint(), localStartPixmap_ );

                        }
                  else p.drawPixmap( QPoint(), startPixmap_ );

                  }

            p.end();
            }

      // copy current pixmap on widget
      if ( !paintOnWidget ) {
            QPainter p( this );
            p.setClipRect( event->rect() );
            p.drawPixmap( QPoint(0, 0), currentPixmap_ );
            p.end();
            }
      }

//---------------------------------------------------------
//   grabBackground
//---------------------------------------------------------

void TransitionWidget::grabBackground(QPixmap& pixmap, QWidget* widget, QRect& rect) const
      {
      if (!widget)
            return;

      QWidgetList widgets;
      if( widget->autoFillBackground() )
        { widgets.push_back( widget ); }

      QWidget* parent(0);

      // get highest level parent
      for ( parent = widget->parentWidget(); parent; parent = parent->parentWidget() ) {

            if ( !( parent->isVisible() && parent->rect().isValid() ) ) continue;

            // store in list
            widgets.push_back( parent );

            // stop at topLevel
            if ( parent->isTopLevel() || parent->autoFillBackground() ) break;

            }

      if ( !parent ) parent = widget;

      // painting
      QPainter p(&pixmap);
      p.setClipRect( rect );
      const QBrush backgroundBrush = parent->palette().brush( parent->backgroundRole());
      if ( backgroundBrush.style() == Qt::TexturePattern) {

            p.drawTiledPixmap( rect, backgroundBrush.texture(), widget->mapTo( parent, rect.topLeft() ) );

            }
      else {

            p.fillRect( pixmap.rect(), backgroundBrush );

            }

      if ( parent->isTopLevel() && parent->testAttribute(Qt::WA_StyledBackground)) {
            QStyleOption option;
            option.initFrom(parent);
            option.rect = rect;
            option.rect.translate( widget->mapTo( parent, rect.topLeft() ) );
            p.translate(-option.rect.topLeft());
            parent->style()->drawPrimitive ( QStyle::PE_Widget, &option, &p, parent );
            }

      p.end();

      // draw all widgets in parent list
      // backward
      QPaintEvent event(rect);
      for (int i = widgets.size() - 1; i>=0; i--) {
            QWidget* w = widgets.at(i);
            w->render(&pixmap, rect.topLeft());
//            QPainter::setRedirected(w, &pixmap, widget->mapTo(w, rect.topLeft()));
//            event = QPaintEvent(QRect(QPoint(), rect.size()));
//            QCoreApplication::sendEvent(w, &event);
//            QPainter::restoreRedirected(w);
            }
      }

//---------------------------------------------------------
//   grabWidget
//---------------------------------------------------------

void TransitionWidget::grabWidget( QPixmap& pixmap, QWidget* widget, QRect& rect ) const
      {
      widget->render( &pixmap, pixmap.rect().topLeft(), rect, QWidget::DrawChildren );
      }

//________________________________________________
void TransitionWidget::fade( const QPixmap& source, QPixmap& target, qreal opacity, const QRect& rect ) const
    {
      if ( target.isNull() || target.size() != size() ) {
            target = QPixmap( size() );
            }

      // erase target
      target.fill( Qt::transparent );

      // check opacity
      if ( opacity * 255 < 1 ) return;

      QPainter p( &target );
      p.setClipRect( rect );

      // draw pixmap
      p.drawPixmap( QPoint(0, 0), source );

      // opacity mask (0.996 corresponds to 254/255)
      if ( opacity <= 0.996 ) {
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QColor color( Qt::black );
            color.setAlphaF( opacity );
            p.fillRect(rect, color );
            }

      p.end();
      return;
      }

