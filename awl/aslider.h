//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __AWLASLIDER_H__
#define __AWLASLIDER_H__

// #include "synthesizer/sparm.h"

namespace Awl {

//---------------------------------------------------------
//    AbstractSlider
//
//!   The AwlAbstractSlider class provides an double value
//!   within a range
//
//!   The class is designed as a common super class for
//!   widgets like AwlKnob and AwlSlider
//!
//---------------------------------------------------------

class AbstractSlider : public QWidget {
      Q_OBJECT
      Q_PROPERTY(double value READ value WRITE setValue)
      Q_PROPERTY(bool center READ center WRITE setCenter)
      Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)

      Q_PROPERTY(int scaleWidth READ scaleWidth WRITE setScaleWidth)
      Q_PROPERTY(QColor scaleColor READ scaleColor WRITE setScaleColor)
      Q_PROPERTY(QColor scaleValueColor READ scaleValueColor WRITE setScaleValueColor)

      Q_PROPERTY(int id READ id WRITE setId)

      Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
      Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)
      Q_PROPERTY(double lineStep READ lineStep WRITE setLineStep)
      Q_PROPERTY(double pageStep READ pageStep WRITE setPageStep)
      Q_PROPERTY(bool   log      READ log      WRITE setLog)

   protected:
      int _id;
      double _value;
      double _minValue, _maxValue, _lineStep, _pageStep;
      bool _center;
      bool _invert;
      int _scaleWidth;        //! scale line width
      QColor _scaleColor;
      QColor _scaleValueColor;
      bool _log;

      virtual void wheelEvent(QWheelEvent*);
      virtual void keyPressEvent(QKeyEvent*);
      virtual void valueChange();

   signals:
      void valueChanged(double, int);

   public slots:
      virtual void setValue(double v);

   public:
      AbstractSlider(QWidget* parent = 0);

      virtual void setCenter(bool val);
      virtual void setScaleWidth(int);
      virtual void setScaleColor(const QColor&);
      virtual void setScaleValueColor(const QColor&);

      //! return the center flag
      bool center() const            { return _center; }

      //! return the scale line width
      int scaleWidth() const         { return _scaleWidth; }

      //! return current scale color
      QColor scaleColor() const      { return _scaleColor; }

      //! return color of active scale part
      QColor scaleValueColor() const { return _scaleValueColor; }

      virtual void setInvertedAppearance(bool val) { _invert = val; }
      bool invertedAppearance() const              { return _invert; }

      int id() const { return _id; }
      void setId(int i) { _id = i; }

      virtual double value() const;

      double minValue() const { return _minValue; }
      void setMinValue(double v) { _minValue = v; }
      void setMinLogValue(double v);
      double maxValue() const {return _maxValue; }
      void setMaxValue(double v) { _maxValue = v; }
      void setMaxLogValue(double v);
      void setRange(double a, double b) {
            setMinValue(a);
            setMaxValue(b);
            }
      void setLogRange(double a, double b) {
            setMinLogValue(a);
            setMaxLogValue(b);
            }
      bool log() const           { return _log;      }
      void setLog(bool v)        { _log = v;         }
      double lineStep() const    { return _lineStep; }
      void setLineStep(double v) { _lineStep = v;    }
      double pageStep() const    { return _pageStep; }
      void setPageStep(double f) { _pageStep = f;    }
      void setEnabled(bool val);
//      virtual void init(const SyntiParameter& p);
      };

}

#endif

