#ifndef oxygentoolbarengine_h
#define oxygentoolbarengine_h

//////////////////////////////////////////////////////////////////////////////
// oxygentoolbarengine.h
// stores event filters and maps widgets to timelines for animations
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
#include "toolbardata.h"

//! follow-mouse toolbar animation
class ToolBarEngine: public BaseEngine {

            Q_OBJECT

      public:

            //! constructor
            ToolBarEngine( QObject* parent ):
                  BaseEngine( parent ),
                  followMouseDuration_( 150 )
                  {}

            //! destructor
            virtual ~ToolBarEngine( void )
                  {}

            //! register toolbar
            virtual void registerWidget( QWidget* );

            //! returns registered widgets
            virtual WidgetList registeredWidgets( void ) const;

            //! return true if object is animated
            virtual bool isAnimated( const QObject* );

            //! return true if object is animated
            virtual bool isFollowMouseAnimated( const QObject* );

            //! animation opacity
            virtual qreal opacity( const QObject* object ) {
                  return isAnimated( object ) ? data_.find( object ).data()->opacity() : AnimationData::OpacityInvalid;
                  }

            //! return 'hover' rect position when widget is animated
            virtual QRect currentRect( const QObject* );

            //! return 'hover' rect position when widget is animated
            virtual QRect animatedRect( const QObject* );

            //! timer
            virtual bool isTimerActive( const QObject* );

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

            //! duration
            virtual int followMouseDuration( void ) const {
                  return followMouseDuration_;
                  }

            //! duration
            virtual void setFollowMouseDuration( int duration ) {
                  followMouseDuration_ = duration;
                  foreach( const DataMap<ToolBarData>::Value & value, data_.values() ) {
                        if ( value ) value.data()->setFollowMouseDuration( duration );
                        }
                  }

      protected slots:

            //! remove widget from map
            virtual bool unregisterWidget( QObject* object ) {
                  return data_.unregisterWidget( object );
                  }

      private:

            //! follow mouse animation duration
            int followMouseDuration_;

            //! data map
            DataMap<ToolBarData> data_;

      };

#endif
