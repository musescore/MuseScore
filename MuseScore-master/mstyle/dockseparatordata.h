#ifndef oxygendockseparatordata_h
#define oxygendockseparatordata_h

//////////////////////////////////////////////////////////////////////////////
// oxygendockseparatordata.h
// generic data container for widgetstate hover (mouse-over) animations
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

#include "genericdata.h"
#include "animation.h"

//! dock widget splitters hover effect
class DockSeparatorData: public AnimationData {

            Q_OBJECT

            //! declare opacity property
            Q_PROPERTY( qreal verticalOpacity READ verticalOpacity WRITE setVerticalOpacity )
            Q_PROPERTY( qreal horizontalOpacity READ horizontalOpacity WRITE setHorizontalOpacity )

      public:

            //! constructor
            DockSeparatorData( QObject* parent, QWidget* target, int duration );

            //! destructor
            virtual ~DockSeparatorData( void )
                  {}

            //@}

            /*!
            returns true if hover has Changed
            and starts timer accordingly
            */
            virtual void updateRect( const QRect&, const Qt::Orientation&, bool hovered );

            //! returns true if current splitter is animated
            virtual bool isAnimated( QRect r, const Qt::Orientation& orientation ) const {
                  return orientation == Qt::Vertical ? verticalData_.isAnimated( r ) : horizontalData_.isAnimated( r );
                  }

            //! opacity for given orientation
            qreal opacity( const Qt::Orientation& orientation ) const {
                  return orientation == Qt::Vertical ? verticalOpacity() : horizontalOpacity();
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  horizontalAnimation().data()->setDuration( duration );
                  verticalAnimation().data()->setDuration( duration );
                  }

            //!@name horizontal splitter data
            //@{

            Animation::Pointer horizontalAnimation( void ) const {
                  return horizontalData_.animation_;
                  }

            const QRect& horizontalRect( void ) const {
                  return horizontalData_.rect_;
                  }

            void setHorizontalRect( const QRect& r ) {
                  horizontalData_.rect_ = r;
                  }

            qreal horizontalOpacity( void ) const {
                  return horizontalData_.opacity_;
                  }

            void setHorizontalOpacity( qreal value ) {

                  if ( horizontalData_.opacity_ == value ) return;

                  horizontalData_.opacity_ = value;
                  if ( target() && !horizontalRect().isEmpty() ) target().data()->update( horizontalRect() );

                  }

            //@}


            //!@name vertical splitter data
            //@{

            Animation::Pointer verticalAnimation( void ) const {
                  return verticalData_.animation_;
                  }

            const QRect& verticalRect( void ) const {
                  return verticalData_.rect_;
                  }

            void setVerticalRect( const QRect& r ) {
                  verticalData_.rect_ = r;
                  }

            qreal verticalOpacity( void ) const {
                  return verticalData_.opacity_;
                  }

            void setVerticalOpacity( qreal value ) {
                  if ( verticalData_.opacity_ == value ) return;
                  verticalData_.opacity_ = value;
                  if ( target() && !verticalRect().isEmpty() ) target().data()->update( verticalRect() );
                  }

            //@}


      private:

            //! stores data needed for animation
            class Data {

                  public:

                        //! constructor
                        Data( void ):
                              opacity_( AnimationData::OpacityInvalid )
                              {}

                        //! true if is animated
                        bool isAnimated( QRect r ) const {
                              return r == rect_ && animation_.data()->isRunning();
                              }

                        //! animation pointer
                        Animation::Pointer animation_;

                        //! opacity variable
                        qreal opacity_;

                        //! stores active separator rect
                        QRect rect_;

                  };

            //! horizontal
            Data horizontalData_;

            //! vertical
            Data verticalData_;

      };

#endif
