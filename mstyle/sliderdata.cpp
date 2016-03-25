//////////////////////////////////////////////////////////////////////////////
// oxygensliderdata.cpp
// data container for QSlider animations
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

#include "sliderdata.h"

Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider*);


//______________________________________________
bool SliderData::updateState( bool state ) {
      if ( state == sliderHovered_ ) return false;
      updateSlider( state ? QStyle::SC_SliderHandle : QStyle::SC_None );
      return true;
      }

//_____________________________________________________________________
void SliderData::updateSlider( QStyle::SubControl hoverControl ) {

      if ( hoverControl == QStyle::SC_SliderHandle ) {

            if ( !sliderHovered() ) {
                  setSliderHovered( true );
                  if ( enabled() ) {
                        animation().data()->setDirection( Animation::Forward );
                        if ( !animation().data()->isRunning() ) animation().data()->start();
                        }
                  else setDirty();
                  }

            }
      else {

            if ( sliderHovered() ) {
                  setSliderHovered( false );
                  if ( enabled() ) {
                        animation().data()->setDirection( Animation::Backward );
                        if ( !animation().data()->isRunning() ) animation().data()->start();
                        }
                  else setDirty();
                  }

            }
      }
