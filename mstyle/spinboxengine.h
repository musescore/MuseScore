#ifndef oxygenspinboxengine_h
#define oxygenspinboxengine_h

//////////////////////////////////////////////////////////////////////////////
// oxygenspinboxengine.h
// stores event filters and maps widgets to animations
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

#include "baseengine.h"
#include "datamap.h"
#include "spinboxdata.h"

//! handle spinbox arrows hover effect
class SpinBoxEngine: public BaseEngine {

            Q_OBJECT

      public:

            //! constructor
            SpinBoxEngine( QObject* parent ):
                  BaseEngine( parent )
                  {}

            //! destructor
            virtual ~SpinBoxEngine( void )
                  {}

            //! register widget
            virtual bool registerWidget( QWidget* );

            //! state
            virtual bool updateState( const QObject* object, QStyle::SubControl subControl, bool value ) {
                  if ( DataMap<SpinBoxData>::Value data = data_.find( object ) ) {
                        return data.data()->updateState( subControl, value );
                        }
                  else return false;
                  }

            //! true if widget is animated
            virtual bool isAnimated( const QObject* object, QStyle::SubControl subControl ) {
                  if ( DataMap<SpinBoxData>::Value data = data_.find( object ) ) {
                        return data.data()->isAnimated( subControl );
                        }
                  else return false;

                  }

            //! animation opacity
            virtual qreal opacity( const QObject* object, QStyle::SubControl subControl ) {
                  if ( DataMap<SpinBoxData>::Value data = data_.find( object ) ) {
                        return data.data()->opacity( subControl );
                        }
                  else return AnimationData::OpacityInvalid;
                  }

            //! enability
            virtual void setEnabled( bool value ) {
                  BaseEngine::setEnabled( value );
                  data_.setEnabled( value );
                  }

            //! duration
            virtual void setDuration( int value ) {
                  BaseEngine::setDuration( value );
                  data_.setDuration( value );
                  }


      public slots:

            //! remove widget from map
            virtual bool unregisterWidget( QObject* object ) {
                  return data_.unregisterWidget( object );
                  }

      private:

            //! data map
            DataMap<SpinBoxData> data_;

      };

#endif
