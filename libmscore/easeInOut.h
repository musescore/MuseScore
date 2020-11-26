//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EASEINOUT_H__
#define __EASEINOUT_H__

namespace Ms {

//---------------------------------------------------------
//   @@ EaseInOut
///   \brief Specialized transfer curve using an underlying Bezier curve.
/// 
///   The second and third control points of the Bezier curve are moveable
///   along the x axis according to two parameters 'EaseIn' and 'EaseOut',
///   which results in a transfer curve that goes from a straight line to an S
///   shaped curve.
//---------------------------------------------------------

class EaseInOut final {
      qreal       _easeIn;
      qreal       _easeOut;

public:
      EaseInOut() : _easeIn(0.0), _easeOut(1.0) {}
      EaseInOut(qreal easeIn, qreal easeOut) : _easeIn(easeIn), _easeOut(easeOut) {}

      void SetEases(qreal easeIn, qreal easeOut) { _easeIn = easeIn; _easeOut = easeOut; }
      qreal EvalX(const qreal t) const          { qreal tCompl = 1.0 - t;  return (3.0 * _easeIn * tCompl * tCompl + (3.0 - 3.0 * _easeOut * tCompl - 2.0 * t) * t) * t; }
      qreal EvalY(const qreal t) const          { return -(t * t) * (2.0 * t - 3.0); }
      QPointF Eval(const qreal t) const         { return {EvalX(t), EvalY(t)}; }
      qreal tFromX(const qreal x) const;
      qreal tFromY(const qreal y) const;
      qreal YfromX(const qreal x) const         { return EvalY(tFromX(x)); }
      qreal XfromY(const qreal y) const         { return EvalX(tFromY(y)); }
      void timeList(const int nbNotes, const int duration, QList<int>* times) const;
};

}     // namespace Ms
#endif