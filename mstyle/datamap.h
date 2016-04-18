//////////////////////////////////////////////////////////////////////////////
// oxygendatamap.h
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

#ifndef __DATAMAP_H__
#define __DATAMAP_H__

//! data map
/*! it maps templatized data object to associated object */
template< typename K, typename T > class BaseDataMap: public QMap< const K*, QPointer<T> > {

      public:

            typedef const K* Key;
            typedef QPointer<T> Value;

            //! constructor
            BaseDataMap( void ):
                  QMap<Key, Value>(),
                  enabled_( true ),
                  lastKey_( NULL )
                  {}

            //! destructor
            virtual ~BaseDataMap( void )
                  {}

            //! insertion
            virtual typename QMap< Key, Value >::iterator insert( const Key& key, const Value& value, bool enabled = true ) {
                  if ( value ) value.data()->setEnabled( enabled );
                  return QMap< Key, Value >::insert( key, value );
                  }

            //! find value
            Value find( Key key ) {
                  if ( !( enabled() && key ) ) return Value();
                  if ( key == lastKey_ ) return lastValue_;
                  else {
                        Value out;
                        typename QMap<Key, Value>::iterator iter( QMap<Key, Value>::find( key ) );
                        if ( iter != QMap<Key, Value>::end() ) out = iter.value();
                        lastKey_ = key;
                        lastValue_ = out;
                        return out;
                        }
                  }

            //! unregister widget
            bool unregisterWidget( Key key ) {

                  // check key
                  if ( !key ) return false;

                  // clear last value if needed
                  if ( key == lastKey_ ) {

                        if ( lastValue_ ) lastValue_.clear();
                        lastKey_ = NULL;

                        }

                  // find key in map
                  typename QMap<Key, Value>::iterator iter( QMap<Key, Value>::find( key ) );
                  if ( iter == QMap<Key, Value>::end() ) return false;

                  // delete value from map if found
                  if ( iter.value() ) iter.value().data()->deleteLater();
                  QMap<Key, Value>::erase( iter );

                  return true;

                  }

            //! maxFrame
            void setEnabled( bool enabled ) {
                  enabled_ = enabled;
                  foreach( const Value & value, *this ) {
                        if ( value ) value.data()->setEnabled( enabled );
                        }
                  }

            //! enability
            bool enabled( void ) const {
                  return enabled_;
                  }

            //! duration
            void setDuration( int duration ) const {
                  foreach( const Value & value, *this ) {
                        if ( value ) value.data()->setDuration( duration );
                        }
                  }

      private:

            //! enability
            bool enabled_;

            //! last key
            Key lastKey_;

            //! last value
            Value lastValue_;

      };

//! standard data map, using QObject as a key
template< typename T > class DataMap: public BaseDataMap< QObject, T > {

      public:

            //! constructor
            DataMap( void )
                  {}

            //! destructor
            virtual ~DataMap( void )
                  {}

      };

//! QPaintDevice based dataMap
template< typename T > class PaintDeviceDataMap: public BaseDataMap< QPaintDevice, T > {

      public:

            //! constructor
            PaintDeviceDataMap( void )
                  {}

            //! destructor
            virtual ~PaintDeviceDataMap( void )
                  {}

      };

#endif
