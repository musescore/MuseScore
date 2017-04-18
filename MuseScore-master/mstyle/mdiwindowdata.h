#ifndef oxygenmdiwindowdata_h
#define oxygenmdiwindowdata_h

//////////////////////////////////////////////////////////////////////////////
// oxygenmdiwindowdata.h
// mdi window data container for window titlebar buttons
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

//! handles mdiwindow arrows hover
class MdiWindowData: public AnimationData {

            Q_OBJECT

            //! declare opacity property
            Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
            Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

      public:

            //! constructor
            MdiWindowData( QObject*, QWidget*, int );

            //! destructor
            virtual ~MdiWindowData( void )
                  {}

            //! animation state
            virtual bool updateState( int primitive, bool value );

            //! animation state
            virtual bool isAnimated( int primitive ) const {
                  return(
                              ( primitive == currentData_.primitive_ && currentAnimation().data()->isRunning() ) ||
                              ( primitive == previousData_.primitive_ && previousAnimation().data()->isRunning() ) );
                  }

            //! opacity
            virtual qreal opacity( int primitive ) const {
                  if ( primitive == currentData_.primitive_ ) return currentOpacity();
                  else if ( primitive == previousData_.primitive_ ) return previousOpacity();
                  else return OpacityInvalid;
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  currentAnimation().data()->setDuration( duration );
                  previousAnimation().data()->setDuration( duration );
                  }

            //!@name current animation
            //@{

            //! opacity
            qreal currentOpacity( void ) const {
                  return currentData_.opacity_;
                  }

            //! opacity
            void setCurrentOpacity( qreal value ) {
                  if ( currentData_.opacity_ == value ) return;
                  currentData_.opacity_ = value;
                  setDirty();
                  }

            //! animation
            Animation::Pointer currentAnimation( void ) const {
                  return currentData_.animation_;
                  }

            //@}
            //!@name previous animation
            //@{

            //! opacity
            qreal previousOpacity( void ) const {
                  return previousData_.opacity_;
                  }

            //! opacity
            void setPreviousOpacity( qreal value ) {
                  if ( previousData_.opacity_ == value ) return;
                  previousData_.opacity_ = value;
                  setDirty();
                  }

            //! animation
            Animation::Pointer previousAnimation( void ) const {
                  return previousData_.animation_;
                  }

            //@}

      private:

            //! container for needed animation data
            class Data {

                  public:

                        //! default constructor
                        Data( void ):
                              primitive_( 0 ),
                              opacity_(0)
                              {}

                        //! subcontrol
                        bool updateSubControl( int );

                        //! subcontrol
                        int primitive_;

                        //! animation
                        Animation::Pointer animation_;

                        //! opacity
                        qreal opacity_;

                  };

            //! current data
            Data currentData_;

            //! previous data
            Data previousData_;

      };


#endif
