#ifndef oxygentabbardata_h
#define oxygentabbardata_h

//////////////////////////////////////////////////////////////////////////////
// oxygentabbardata.h
// data container for QTabBar animations
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

#include "animationdata.h"

//! tabbars
class TabBarData: public AnimationData {
            Q_OBJECT

            //! declare opacity property
            Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
            Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

      public:

            //! constructor
            TabBarData( QObject* parent, QWidget* target, int duration );

            //! destructor
            virtual ~TabBarData( void )
                  {}

            //! duration
            void setDuration( int duration ) {
                  currentIndexAnimation().data()->setDuration( duration );
                  previousIndexAnimation().data()->setDuration( duration );
                  }

            //! update state
            bool updateState( const QPoint&, bool );

            //!@name current index handling
            //@{

            //! current opacity
            virtual qreal currentOpacity( void ) const {
                  return current_.opacity_;
                  }

            //! current opacity
            virtual void setCurrentOpacity( qreal value ) {
                  if ( current_.opacity_ == value ) return;
                  current_.opacity_ = value;
                  setDirty();
                  }

            //! current index
            virtual int currentIndex( void ) const {
                  return current_.index_;
                  }

            //! current index
            virtual void setCurrentIndex( int index ) {
                  current_.index_ = index;
                  }

            //! current index animation
            virtual const Animation::Pointer& currentIndexAnimation( void ) const {
                  return current_.animation_;
                  }

            //@}

            //!@name previous index handling
            //@{

            //! previous opacity
            virtual qreal previousOpacity( void ) const {
                  return previous_.opacity_;
                  }

            //! previous opacity
            virtual void setPreviousOpacity( qreal value ) {
                  if ( previous_.opacity_ == value ) return;
                  previous_.opacity_ = value;
                  setDirty();
                  }

            //! previous index
            virtual int previousIndex( void ) const {
                  return previous_.index_;
                  }

            //! previous index
            virtual void setPreviousIndex( int index ) {
                  previous_.index_ = index;
                  }

            //! previous index Animation
            virtual const Animation::Pointer& previousIndexAnimation( void ) const {
                  return previous_.animation_;
                  }

            //@}

            //! return Animation associated to action at given position, if any
            virtual Animation::Pointer animation( const QPoint& position ) const;

            //! return opacity associated to action at given position, if any
            virtual qreal opacity( const QPoint& position ) const;

      private:

            //! container for needed animation data
            class Data {
                  public:

                        //! default constructor
                        Data( void ):
                              opacity_(0),
                              index_(-1)
                              {}

                        Animation::Pointer animation_;
                        qreal opacity_;
                        int index_;
                  };

            //! current tab animation data (for hover enter animations)
            Data current_;

            //! previous tab animations data (for hover leave animations)
            Data previous_;

      };

#endif
