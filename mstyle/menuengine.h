#ifndef oxygenmenuengine_h
#define oxygenmenuengine_h

//////////////////////////////////////////////////////////////////////////////
// oxygenmenuengine.h
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
#include "menudata.h"

//! stores menu hovered action and timeLine
class MenuBaseEngine: public BaseEngine {
            Q_OBJECT

      public:

            //! constructor
            MenuBaseEngine( QObject* parent ):
                  BaseEngine( parent )
                  {}

            //! destructor
            virtual ~MenuBaseEngine( void )
                  {}

            //! register menubar
            virtual bool registerWidget( QWidget* ) = 0;

            //! true if widget is animated
            virtual bool isAnimated( const QObject*, WidgetIndex ) {
                  return false;
                  }

            //! opacity
            virtual qreal opacity( const QObject*, WidgetIndex ) {
                  return -1;
                  }

            //! return 'hover' rect position when widget is animated
            virtual QRect currentRect( const QObject*, WidgetIndex ) {
                  return QRect();
                  }

            //! return 'hover' rect position when widget is animated
            virtual QRect animatedRect( const QObject* ) {
                  return QRect();
                  }

            //! timer associated to the data
            virtual bool isTimerActive( const QObject* ) {
                  return false;
                  }

            //! enability
            virtual void setEnabled( bool value ) = 0;

            //! duration
            virtual void setDuration( int ) = 0;

            //! duration
            virtual void setFollowMouseDuration( int )
                  {}

      };

//! stores menu hovered action and timeLine
class MenuEngineV1: public MenuBaseEngine {
            Q_OBJECT

      public:

            //! constructor
            MenuEngineV1( QObject* parent ):
                  MenuBaseEngine( parent )
                  {}

            //! constructor
            MenuEngineV1( QObject* parent, MenuBaseEngine* other );

            //! destructor
            virtual ~MenuEngineV1( void )
                  {}

            //! register menubar
            virtual bool registerWidget( QWidget* );

            //! true if widget is animated
            virtual bool isAnimated( const QObject* object, WidgetIndex index );

            //! animation opacity
            virtual qreal opacity( const QObject* object, WidgetIndex index ) {
                  if ( !isAnimated( object, index ) ) return AnimationData::OpacityInvalid;
                  else return data_.find(object).data()->opacity( index );
                  }

            //! return 'hover' rect position when widget is animated
            virtual QRect currentRect( const QObject* object, WidgetIndex index ) {
                  if ( !isAnimated( object, index ) ) return QRect();
                  else return data_.find(object).data()->currentRect( index );
                  }

            //! enability
            virtual void setEnabled( bool value ) {
                  BaseEngine::setEnabled( value );
                  data_.setEnabled( value );
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  BaseEngine::setDuration( duration );
                  data_.setDuration( duration );
                  }

            //! return list of registered widgets
            virtual WidgetList registeredWidgets( void ) const;

      public slots:

            //! remove widget from map
            virtual bool unregisterWidget( QObject* object ) {
                  return data_.unregisterWidget( object );
                  }

      private:

            //! data map
            DataMap<MenuDataV1> data_;

      };

//! stores menu hovered action and timeLine
class MenuEngineV2: public MenuBaseEngine {

            Q_OBJECT

      public:

            //! constructor
            MenuEngineV2( QObject* parent ):
                  MenuBaseEngine( parent )
                  {}

            //! destructor
            virtual ~MenuEngineV2( void )
                  {}

            //! constructor
            MenuEngineV2( QObject* parent, MenuBaseEngine* other );

            //! register menu
            virtual bool registerWidget( QWidget* );

            //! return timeLine associated to action at given position, if any
            virtual bool isAnimated( const QObject*, WidgetIndex );

            //! animation opacity
            virtual qreal opacity( const QObject* object, WidgetIndex index ) {
                  if ( !isAnimated( object, index ) ) return AnimationData::OpacityInvalid;
                  else return data_.find(object).data()->opacity();
                  }

            //! return 'hover' rect position when widget is animated
            virtual QRect currentRect( const QObject* object, WidgetIndex index );

            //! return 'hover' rect position when widget is animated
            virtual QRect animatedRect( const QObject* );

            //! timer associated to the data
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
                  foreach( const DataMap<MenuDataV2>::Value & value, data_.values() ) {
                        if ( value ) value.data()->setFollowMouseDuration( duration );
                        }
                  }

            //! return list of registered widgets
            virtual WidgetList registeredWidgets( void ) const;

      protected slots:

            //! remove widget from map
            virtual bool unregisterWidget( QObject* object ) {
                  return data_.unregisterWidget( object );
                  }

      private:

            //! follow mouse animation duration
            int followMouseDuration_;

            //! data map
            DataMap<MenuDataV2> data_;

      };

#endif
