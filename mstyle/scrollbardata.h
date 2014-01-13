#ifndef oxygenscrollbardata_h
#define oxygenscrollbardata_h

//////////////////////////////////////////////////////////////////////////////
// oxygenscrollbardata.h
// data container for QScrollBar animations
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

//! scrollbar data
class ScrollBarData: public SliderData {

            Q_OBJECT
            Q_PROPERTY( qreal addLineOpacity READ addLineOpacity WRITE setAddLineOpacity )
            Q_PROPERTY( qreal subLineOpacity READ subLineOpacity WRITE setSubLineOpacity )

      public:

            //! constructor
            ScrollBarData( QObject* parent, QWidget* target, int );

            //! destructor
            virtual ~ScrollBarData( void )
                  {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            //! needed to avoid warning about virtual function being hidden
            using SliderData::animation;
            using SliderData::opacity;

            //! return animation for a given subcontrol
            virtual const Animation::Pointer& animation( QStyle::SubControl ) const;

            //! return default opacity for a given subcontrol
            virtual qreal opacity( QStyle::SubControl ) const;

            //! return default opacity for a given subcontrol
            virtual bool isHovered( QStyle::SubControl control ) const {
                  switch ( control ) {
                        case QStyle::SC_ScrollBarAddLine:
                              return addLineArrowHovered();
                        case QStyle::SC_ScrollBarSubLine:
                              return subLineArrowHovered();
                        default:
                              return false;
                        }


                  }

            //! subControlRect
            virtual QRect subControlRect( QStyle::SubControl control ) const {
                  switch ( control ) {
                        case QStyle::SC_ScrollBarAddLine:
                              return addLineData_.rect_;
                        case QStyle::SC_ScrollBarSubLine:
                              return subLineData_.rect_;
                        default:
                              return QRect();
                        }
                  }


            //! subcontrol rect
            virtual void setSubControlRect( QStyle::SubControl control, const QRect& rect ) {
                  switch ( control ) {
                        case QStyle::SC_ScrollBarAddLine:
                              addLineData_.rect_ = rect;
                              break;

                        case QStyle::SC_ScrollBarSubLine:
                              subLineData_.rect_ = rect;
                              break;

                        default:
                              break;
                        }
                  }

            //! duration
            virtual void setDuration( int duration ) {
                  SliderData::setDuration( duration );
                  addLineAnimation().data()->setDuration( duration );
                  subLineAnimation().data()->setDuration( duration );
                  }

            //! addLine opacity
            virtual void setAddLineOpacity( qreal value ) {
                  if ( addLineData_.opacity_ == value ) return;
                  addLineData_.opacity_ = value;
                  setDirty();
                  }

            //! addLine opacity
            virtual qreal addLineOpacity( void ) const {
                  return addLineData_.opacity_;
                  }

            //! subLine opacity
            virtual void setSubLineOpacity( qreal value ) {
                  if ( subLineData_.opacity_ == value ) return;
                  subLineData_.opacity_ = value;
                  setDirty();
                  }

            //! subLine opacity
            virtual qreal subLineOpacity( void ) const {
                  return subLineData_.opacity_;
                  }

      protected slots:

            //! clear addLineRect
            void clearAddLineRect( void ) {
                  if ( addLineAnimation().data()->direction() == Animation::Backward ) {
                        addLineData_.rect_ = QRect();
                        }
                  }

            //! clear subLineRect
            void clearSubLineRect( void ) {
                  if ( subLineAnimation().data()->direction() == Animation::Backward ) {
                        subLineData_.rect_ = QRect();
                        }
                  }

      protected:

            //! hoverMoveEvent
            virtual void hoverMoveEvent( QObject*, QEvent* );

            //! hoverMoveEvent
            virtual void hoverLeaveEvent( QObject*, QEvent* );

            //!@name hover flags
            //@{

            virtual bool addLineArrowHovered( void ) const {
                  return addLineData_.hovered_;
                  }

            virtual void setAddLineArrowHovered( bool value ) {
                  addLineData_.hovered_ = value;
                  }

            virtual bool subLineArrowHovered( void ) const {
                  return subLineData_.hovered_;
                  }

            virtual void setSubLineArrowHovered( bool value ) {
                  subLineData_.hovered_ = value;
                  }

            //@}

            //! update add line arrow
            virtual void updateAddLineArrow( QStyle::SubControl );

            //! update sub line arrow
            virtual void updateSubLineArrow( QStyle::SubControl );

            //!@name timelines
            //@{

            virtual const Animation::Pointer& addLineAnimation( void ) const {
                  return addLineData_.animation_;
                  }

            virtual const Animation::Pointer& subLineAnimation( void ) const {
                  return subLineData_.animation_;
                  }

      private:

            //! stores arrow data
            class Data {

                  public:

                        //! constructor
                        Data( void ):
                              hovered_( false ),
                              opacity_( AnimationData::OpacityInvalid )
                              {}

                        //! true if hovered
                        bool hovered_;

                        //! animation
                        Animation::Pointer animation_;

                        //! opacity
                        qreal opacity_;

                        //! rect
                        QRect rect_;

                  };


            //! add line data (down arrow)
            Data addLineData_;

            //! subtract line data (up arrow)
            Data subLineData_;

      };

#endif
