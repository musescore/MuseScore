//////////////////////////////////////////////////////////////////////////////
// oxygenmenuengine.cpp
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

#include "menuengine.h"

//____________________________________________________________
MenuEngineV1::MenuEngineV1( QObject* parent, MenuBaseEngine* other ):
      MenuBaseEngine( parent ) {
      if ( other ) {
            foreach( QWidget * widget,  other->registeredWidgets() ) {
                  registerWidget( widget );
                  }
            }
      }

//____________________________________________________________
bool MenuEngineV1::registerWidget( QWidget* widget ) {

      if ( !widget ) return false;

      // create new data class
      if ( !data_.contains( widget ) ) data_.insert( widget, new MenuDataV1( this, widget, duration() ), enabled() );

      // connect destruction signal
      connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ), Qt::UniqueConnection );
      return true;
      }

//____________________________________________________________
bool MenuEngineV1::isAnimated( const QObject* object, WidgetIndex index ) {
      DataMap<MenuDataV1>::Value data( data_.find( object ) );
      if ( !data ) {
            return false;
            }

      if ( Animation::Pointer animation = data.data()->animation( index ) ) {

            return animation.data()->isRunning();

            }
      else return false;
      }

//____________________________________________________________
BaseEngine::WidgetList MenuEngineV1::registeredWidgets( void ) const {

      WidgetList out;

      // the typedef is needed to make Krazy happy
      typedef DataMap<MenuDataV1>::Value Value;
      foreach( const Value & value, data_ ) {
            if ( value ) out.insert( value.data()->target().data() );
            }

      return out;

      }

//____________________________________________________________
MenuEngineV2::MenuEngineV2( QObject* parent, MenuBaseEngine* other ):
      MenuBaseEngine( parent ),
      followMouseDuration_( 150 ) {
      if ( other ) {
            foreach( QWidget * widget, other->registeredWidgets() ) {
                  registerWidget( widget );
                  }
            }
      }

//____________________________________________________________
bool MenuEngineV2::registerWidget( QWidget* widget ) {

      if ( !widget ) return false;

      // create new data class
      if ( !data_.contains( widget ) ) {
            DataMap<MenuDataV2>::Value value( new MenuDataV2( this, widget, duration() ) );
            value.data()->setFollowMouseDuration( followMouseDuration() );
            data_.insert( widget, value, enabled() );
            }

      // connect destruction signal
      connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ), Qt::UniqueConnection );

      return true;
      }

//____________________________________________________________
QRect MenuEngineV2::currentRect( const QObject* object, WidgetIndex ) {
      if ( !enabled() ) return QRect();
      DataMap<MenuDataV2>::Value data( data_.find( object ) );
      return data ? data.data()->currentRect() : QRect();

      }

//____________________________________________________________
bool MenuEngineV2::isAnimated( const QObject* object, WidgetIndex index ) {
      DataMap<MenuDataV2>::Value data( data_.find( object ) );
      if ( !data ) {
            return false;
            }

      switch ( index ) {
            case Previous: {
                  if ( Animation::Pointer animation = data.data()->animation() ) {
                        return animation.data()->direction() == Animation::Backward && animation.data()->isRunning();
                        }
                  else return false;
                  }

            case Current: {
                  if ( data.data()->animation() && data.data()->animation().data()->isRunning() ) return true;
                  else return false;
                  }

            default:
                  return false;

            }

      }

//____________________________________________________________
QRect MenuEngineV2::animatedRect( const QObject* object ) {
      if ( !enabled() ) return QRect();
      DataMap<MenuDataV2>::Value data( data_.find( object ) );
      return data ? data.data()->animatedRect() : QRect();

      }

//____________________________________________________________
bool MenuEngineV2::isTimerActive( const QObject* object ) {
      if ( !enabled() ) return false;
      DataMap<MenuDataV2>::Value data( data_.find( object ) );
      return data ? data.data()->timer().isActive() : false;

      }

//____________________________________________________________
BaseEngine::WidgetList MenuEngineV2::registeredWidgets( void ) const {

      WidgetList out;

      // the typedef is needed to make Krazy happy
      typedef DataMap<MenuDataV2>::Value Value;
      foreach( const Value & value, data_ ) {
            if ( value ) out.insert( value.data()->target().data() );
            }

      return out;

      }

