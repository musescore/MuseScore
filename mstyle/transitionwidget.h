#ifndef oxygentransitionwidget_h
#define oxygentransitionwidget_h
//////////////////////////////////////////////////////////////////////////////
// oxygentransitionwidget.h
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

#include "animation.h"

//! temporary widget used to perform smooth transition between one widget state and another
class TransitionWidget: public QWidget {

            Q_OBJECT

            //! declare opacity property
            Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

      public:

            //! shortcut to painter
            typedef QPointer<TransitionWidget> Pointer;

            //! constructor
            TransitionWidget( QWidget* parent, int duration );

            //! destructor
            virtual ~TransitionWidget( void )
                  {}

            //!@name flags
            //@{
            enum Flag {
                  None = 0,
                  GrabFromWindow = 1 << 0,
                  Transparent = 1 << 1,
                  PaintOnWidget = 1 << 2
                  };

            Q_DECLARE_FLAGS(Flags, Flag)

            void setFlags( Flags value ) {
                  flags_ = value;
                  }

            void setFlag( Flag flag, bool value = true ) {
                  if ( value ) flags_ |= flag;
                  else flags_ &= (~flag);
                  }

            bool testFlag( Flag flag ) const {
                  return flags_.testFlag( flag );
                  }

            //@}

            //! duration
            void setDuration( int duration ) {
                  if ( animation_ ) {
                        animation_.data()->setDuration( duration );
                        }
                  }

            //! duration
            int duration( void ) const {
                  return ( animation_ ) ? animation_.data()->duration() : 0;
                  }

            //!@name opacity
            //@{

            virtual qreal opacity( void ) const {
                  return opacity_;
                  }

            virtual void setOpacity( qreal value ) {
                  if ( opacity_ == value ) return;
                  opacity_ = value;
                  update();
                  }

            //@}

            //@name pixmaps handling
            //@{

            //! start
            void resetStartPixmap( void ) {
                  setStartPixmap( QPixmap() );
                  }

            //! start
            void setStartPixmap( QPixmap pixmap ) {
                  startPixmap_ = pixmap;
                  }

            //! start
            const QPixmap& startPixmap( void ) const {
                  return startPixmap_;
                  }

            //! end
            void resetEndPixmap( void ) {
                  setEndPixmap( QPixmap() );
                  }

            //! end
            void setEndPixmap( QPixmap pixmap ) {
                  endPixmap_ = pixmap;
                  currentPixmap_ = pixmap;
                  }

            //! start
            const QPixmap& endPixmap( void ) const {
                  return endPixmap_;
                  }

            //! current
            const QPixmap& currentPixmap( void ) const {
                  return currentPixmap_;
                  }

            //@}

            //! grap pixmap
            QPixmap grab( QWidget* = 0, QRect = QRect() );

            //! true if animated
            virtual bool isAnimated( void ) const {
                  return animation_.data()->isRunning();
                  }

            //! end animation
            virtual void endAnimation( void ) {
                  if ( animation_.data()->isRunning() ) animation_.data()->stop();
                  }

            //! animate transition
            virtual void animate( void ) {
                  endAnimation();
                  animation_.data()->start();
                  }

            //! true if paint is enabled
            static bool paintEnabled( void );

      signals:

            //! emmitted when animation is finished/aborder
            void finished( void );

      protected:

            //! paint event
            virtual void paintEvent( QPaintEvent* );

            //! grab widget background
            /*!
            Background is not rendered properly using QWidget::render.
            Use home-made grabber instead. This is directly inspired from bespin.
            Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
            */
            virtual void grabBackground( QPixmap&, QWidget*, QRect& ) const;

            //! grab widget
            virtual void grabWidget( QPixmap&, QWidget*, QRect& ) const;

            //! fade pixmap
            virtual void fade( const QPixmap& source, QPixmap& target, qreal opacity, const QRect& ) const;

      private:

            //! Flags
            Flags flags_;

            //! paint enabled
            static bool paintEnabled_;

            //! internal transition animation
            Animation::Pointer animation_;

            //! animation starting pixmap
            QPixmap startPixmap_;

            //! animation starting pixmap
            QPixmap endPixmap_;

            //! local start pixmap (used for fading)
            QPixmap localStartPixmap_;

            //! local end pixmap (used for fading)
            QPixmap localEndPixmap_;

            //! current pixmap
            QPixmap currentPixmap_;

            //! current state opacity
            qreal opacity_;

      };

#endif
