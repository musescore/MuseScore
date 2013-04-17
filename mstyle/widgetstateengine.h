//////////////////////////////////////////////////////////////////////////////
// oxygenwidgetstateengine.h
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

#ifndef __WIDGETSTATEENGINE_H__
#define __WIDGETSTATEENGINE_H__

#include "baseengine.h"
#include "datamap.h"
#include "widgetstatedata.h"
#include "animationmodes.h"


//! used for simple widgets
class WidgetStateEngine: public BaseEngine {

            Q_OBJECT

      public:

            //! constructor
            WidgetStateEngine( QObject* parent ):
                  BaseEngine( parent )
                  {}

            //! destructor
            virtual ~WidgetStateEngine( void )
                  {}

            //! register widget
            virtual bool registerWidget( QWidget*, AnimationModes );

            //! returns registered widgets
            virtual WidgetList registeredWidgets( AnimationModes ) const;

            using BaseEngine::registeredWidgets;

            //! true if widget hover state is changed
            virtual bool updateState( const QObject*, AnimationMode, bool );

            //! true if widget is animated
            virtual bool isAnimated( const QObject*, AnimationMode );

            //! animation opacity
            virtual qreal opacity( const QObject* object, AnimationMode mode ) {
                  return isAnimated( object, mode ) ? data( object, mode ).data()->opacity() : AnimationData::OpacityInvalid;
                  }

            //! duration
            virtual void setEnabled( bool value ) {
                  BaseEngine::setEnabled( value );
                  hoverData_.setEnabled( value );
                  focusData_.setEnabled( value );
                  enableData_.setEnabled( value );
                  }

            //! duration
            virtual void setDuration( int value ) {
                  BaseEngine::setDuration( value );
                  hoverData_.setDuration( value );
                  focusData_.setDuration( value );
                  enableData_.setDuration( value );
                  }

      public slots:

            //! remove widget from map
            virtual bool unregisterWidget( QObject* object ) {
                  if ( !object ) return false;
                  bool found = false;
                  if ( hoverData_.unregisterWidget( object ) ) found = true;
                  if ( focusData_.unregisterWidget( object ) ) found = true;
                  if ( enableData_.unregisterWidget( object ) ) found = true;
                  return found;
                  }

      protected:

            //! returns data associated to widget
            DataMap<WidgetStateData>::Value data( const QObject*, AnimationMode );

      private:

            //! maps
            DataMap<WidgetStateData> hoverData_;
            DataMap<WidgetStateData> focusData_;
            DataMap<WidgetStateData> enableData_;

      };

Q_DECLARE_OPERATORS_FOR_FLAGS(AnimationModes)

#endif

