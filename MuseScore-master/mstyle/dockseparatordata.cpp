//////////////////////////////////////////////////////////////////////////////
// oxygendockseparatordata.cpp
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

#include "dockseparatordata.h"


//______________________________________________
DockSeparatorData::DockSeparatorData( QObject* parent, QWidget* target, int duration ):
      AnimationData( parent, target ) {

      // setup animation
      horizontalData_.animation_ = new Animation( duration, this );
      horizontalData_.animation_.data()->setStartValue( 0.0 );
      horizontalData_.animation_.data()->setEndValue( 1.0 );
      horizontalData_.animation_.data()->setTargetObject( this );
      horizontalData_.animation_.data()->setPropertyName( "horizontalOpacity" );

      // setup animation
      verticalData_.animation_ = new Animation( duration, this );
      verticalData_.animation_.data()->setStartValue( 0.0 );
      verticalData_.animation_.data()->setEndValue( 1.0 );
      verticalData_.animation_.data()->setTargetObject( this );
      verticalData_.animation_.data()->setPropertyName( "verticalOpacity" );

      }

//______________________________________________
void DockSeparatorData::updateRect( const QRect& r, const Qt::Orientation& orientation, bool hovered ) {

      Data& data( orientation == Qt::Vertical ? verticalData_ : horizontalData_ );

      if ( hovered ) {
            data.rect_ = r;
            if ( data.animation_.data()->direction() == Animation::Backward ) {
                  if ( data.animation_.data()->isRunning() ) data.animation_.data()->stop();
                  data.animation_.data()->setDirection( Animation::Forward );
                  data.animation_.data()->start();
                  }

            }
      else if ( data.animation_.data()->direction() == Animation::Forward && r == data.rect_  ) {

            if ( data.animation_.data()->isRunning() ) data.animation_.data()->stop();
            data.animation_.data()->setDirection( Animation::Backward );
            data.animation_.data()->start();

            }

      }
