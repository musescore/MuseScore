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
///   Fraction object available to QML plugins.
///   Use PluginAPI::PluginAPI::fraction to create a
///   fraction for usage within your plugin:
///   \code
///   var ts = newElement(Element.TIMESIG);
///   ts.timesig = fraction(3, 4);
///   \endcode
//---------------------------------------------------------

class FractionWrapper : public QObject {
      Q_OBJECT
      /** Fraction numerator */
      Q_PROPERTY(int numerator READ numerator)
      /** Fraction denominator */
      Q_PROPERTY(int denominator READ denominator)
      /**
       * MIDI ticks number equal to the number of the whole
       * notes represented by this fraction.
       */
      Q_PROPERTY(int ticks READ ticks) // FIXME: fraction transition
      /** String representation of this fraction */
      Q_PROPERTY(QString str READ toString)

      Ms::Fraction f;

      /// \cond MS_INTERNAL
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
      /// \endcond
      };

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates FractionWrapper
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
