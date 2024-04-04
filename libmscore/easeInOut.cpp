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

/**
 \file
 Implementation of classe easeInOut for implementing transfer curve with
 parametrerable ease-In and ease-Out.
*/

#include "easeInOut.h"

namespace Ms {

//-------------------------------------------------------------------------------------------------
// The following function is inspired by "A Primer on Bézier Curve" sections 17 and 23 by Pomax:
// https://pomax.github.io/bezierinfo/
// However, the fuction is greatly specialized, simplified and optimized for use as an ease-in and
// ease-out transfer curve for bends, glissandi and portamenti in MuseScore. The function computes
// Y from X by performing a cubic root finding to compute t from X on the X component of the
// transfer curve and then a cubic Bezier evaluation to compute Y from t on the Y component of the
// transfer curve. The code could be simplified because the Bezier curve is constrained as a well
// defined transfer function. It is not suitable for general or arbitrary Bezier curve work. -YP
//-------------------------------------------------------------------------------------------------
qreal EaseInOut::tFromX(const qreal x) const {
      const qreal pi = 3.14159265358979323846;
      qreal c = -x;
      qreal w1 = _easeIn - x;
      qreal w2 = 1.0 - _easeOut - x;
      qreal d = x + 3.0 * w1 - 3.0 * w2 + (1.0 - x);
      qreal a = (-3.0 * x - 6.0 * w1 + 3 * w2) / d;
      qreal b = (3.0 * x + 3.0 * w1) / d;
      c = -x / d;
      qreal a2 = a * a;
      qreal p = (3.0 * b - a2) / 3.0;
      qreal q = (2.0 * a2 * a - 9.0 * a * b + 27.0 * c) / 27.0;
      qreal discr = (q * q) / 4.0 + (p * p * p) / 27.0;
      qreal t = 0.0;
      // Crazy idea to first test the least probable case with such an expensive test but...
      if (qFuzzyCompare(discr, 0.0)) {
            // Case that happens extremely rarely --> 2 roots.
            qreal q2 = q / 2.0;
            qreal u = q2 < 0.0 ? std::pow(-q2, 1.0 / 3.0) : -std::pow(q2, 1.0 / 3.0);
            // Find the only root that is within the 0 to 1 interval.
            t = 2.0 * u - a / 3.0;
            if (0.0 > t || t > 1.0) {
                  t = -u - a / 3.0;
                  }
            }
      else if (discr < 0.0) {
            // Case that happens about 75% of the time --> 3 roots.
            qreal mp3 = -p / 3.0;
            qreal r = std::sqrt(mp3 * mp3 * mp3);
            qreal phi = std::acos(std::min(std::max(-1.0, -q / (2.0 * r)), 1.0));
            qreal t1 = 2.0 * std::pow(r, 1.0 / 3.0);
            // Find the only root that is within the 0 to 1 interval.
            t = t1 * std::cos(phi / 3.0) - a / 3.0;
            if (0.0 > t || t > 1.0) {
                  t = t1 * std::cos((phi + 2.0 * pi) / 3.0) - a / 3.0;
                  if (0.0 > t || t > 1.0) {
                        t = t1 * std::cos((phi + 4.0 * pi) / 3.0) - a / 3.0;
                  }
            }
      }
      else if (discr > 0.0) {
            // Case that happens about 25% of the time --> 1 root.
            qreal q2 = q / 2.0;
            qreal sd = std::sqrt(discr);
            qreal u = std::pow(-q2 + sd, 1.0 / 3.0);
            qreal v = std::pow(q2 + sd, 1.0 / 3.0);
            t = u - v - a / 3.0;
            }
      return t;
      }

//-------------------------------------------------------------------------------------------------
// This function is even more simplified and optimized because all the Bezier control points are
// constant. Thus, when computing the Y root there is only one case and the math simplifies to the
// following simple expression.
//-------------------------------------------------------------------------------------------------
qreal EaseInOut::tFromY(const qreal y) const {
      return 0.5 + std::cos((4.0 * M_PI + std::acos(1.0 - 2.0 * y)) / 3.0);
      }

//-------------------------------------------------------------------------------------------------
// Given a number of note to place within the given duration, return the list of on-times for each
// note given the current ease-in and ease-out parameters. The first note is at time 0 while the 
//-------------------------------------------------------------------------------------------------
void EaseInOut::timeList(const int nbNotes, const int duration, QList<int>* times) const {
      qreal nNotes = qreal(nbNotes);
      qreal space = qreal(duration);
      if (qFuzzyIsNull(_easeIn) && qFuzzyIsNull(_easeOut)) {
            for (int n = 0; n <= nbNotes; n++) {
                  times->push_back(static_cast<int>(std::lround((static_cast<qreal>(n) / nNotes) * space)));
                  }
      } else {
            for (int n = 0; n <= nbNotes; n++) {
                  times->push_back(static_cast<int>(std::lround(XfromY(static_cast<qreal>(n) / nNotes) * space)));
                 }
            }
      }

}


