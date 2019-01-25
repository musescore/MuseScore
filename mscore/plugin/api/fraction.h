//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_FRACTION_H__
#define __PLUGIN_API_FRACTION_H__

#include "libmscore/fraction.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   FractionWrapper
//---------------------------------------------------------

class FractionWrapper : public QObject {
      Q_OBJECT
      Q_PROPERTY(int numerator READ numerator)
      Q_PROPERTY(int denominator READ denominator)
      Q_PROPERTY(int ticks READ ticks)
      Q_PROPERTY(QString str READ toString)

      Ms::Fraction f;

   public slots:
      void setFraction(Fraction _f) { f = _f; }

   public:
      FractionWrapper() = default;
      FractionWrapper(const Ms::Fraction& _f) : f(_f) {}

      Ms::Fraction fraction() const { return f; }
      int numerator() const   { return f.numerator(); }
      int denominator() const { return f.denominator(); }
      int ticks() const       { return f.ticks(); }
      QString toString() const { return f.toString(); }
      };

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

inline FractionWrapper* wrap(Ms::Fraction f)
      {
      FractionWrapper* w = new FractionWrapper(f);
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

}     // namespace PluginAPI
}     // namespace Ms

#endif
