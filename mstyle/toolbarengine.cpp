//////////////////////////////////////////////////////////////////////////////
// oxygentoolbarengine.cpp
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

#include "toolbarengine.h"

//____________________________________________________________
void ToolBarEngine::registerWidget( QWidget* widget ) {

      if ( !widget ) return;

      // create new data class
      if ( !data_.contains( widget ) ) {
            DataMap<ToolBarData>::Value value( new ToolBarData( this, widget, duration() ) );
            value.data()->setFollowMouseDuration( followMouseDuration() );
            data_.insert( widget, value, enabled() );
            }

      // connect destruction signal
      disconnect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ) );
      connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ) );

      }

//____________________________________________________________
BaseEngine::WidgetList ToolBarEngine::registeredWidgets( void ) const {
      WidgetList out;

      // the typedef is needed to make Krazy happy
      typedef DataMap<ToolBarData>::Value Value;
      foreach( const Value & value, data_ ) {
            if ( value ) out.insert( value.data()->target().data() );
            }
      return out;
      }

//____________________________________________________________
bool ToolBarEngine::isAnimated( const QObject* object ) {
      if ( !enabled() ) return false;

      DataMap<ToolBarData>::Value data( data_.find( object ) );
      if ( !data ) return false;
      if ( Animation::Pointer animation = data.data()->animation() ) return animation.data()->isRunning();
      else return false;
      }

//____________________________________________________________
bool ToolBarEngine::isFollowMouseAnimated( const QObject* object ) {
      if ( !enabled() ) return false;

      DataMap<ToolBarData>::Value data( data_.find( object ) );
      if ( !data ) return false;
      if ( Animation::Pointer animation = data.data()->progressAnimation() ) return animation.data()->isRunning();
      else return false;
      }

//____________________________________________________________
QRect ToolBarEngine::currentRect( const QObject* object ) {
      if ( !enabled() ) return QRect();
      DataMap<ToolBarData>::Value data( data_.find( object ) );
      return data ? data.data()->currentRect() : QRect();
      }

//____________________________________________________________
QRect ToolBarEngine::animatedRect( const QObject* object ) {
      if ( !enabled() ) return QRect();
      DataMap<ToolBarData>::Value data( data_.find( object ) );
      return data ? data.data()->animatedRect() : QRect();
      }

//____________________________________________________________
bool ToolBarEngine::isTimerActive( const QObject* object ) {
      if ( !enabled() ) return false;
      DataMap<ToolBarData>::Value data( data_.find( object ) );
      return data ? data.data()->timer().isActive() : false;
      }
