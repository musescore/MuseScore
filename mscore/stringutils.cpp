//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "stringutils.h"

namespace Ms {

//---------------------------------------------------------
//   removeLigatures
//---------------------------------------------------------

QString stringutils::removeLigatures(const QString& pre)
      {
      QString result = pre;

      // Latin Big Letter AA (&#42802) and Latin Small Letter aa (&#42803)
      result = result.replace(QRegularExpression("[\\x{A732}]"), QString("AA"));
      result = result.replace(QRegularExpression("[\\x{A733}]"), QString("aa"));

      // Latin Big Letter AE (&#198) and Latin Small Letter ae (&#230), A with above dots, a with above dots
      result = result.replace(QRegularExpression("[\\x{00C6}\\x{00C4}]"), QString("AE"));
      result = result.replace(QRegularExpression("[\\x{00E6}\\x{00E4}]"), QString("ae"));

      // Latin Big Letter AO (&#42804) and Latin Small Letter ao (&#42805)
      result = result.replace(QRegularExpression("[\\x{A734}]"), QString("AO"));
      result = result.replace(QRegularExpression("[\\x{A735}]"), QString("ao"));

      // Latin Big Letter AU (&#42806) and Latin Small Letter au (&#42807)
      result = result.replace(QRegularExpression("[\\x{A736}]"), QString("AU"));
      result = result.replace(QRegularExpression("[\\x{A737}]"), QString("au"));

      // IJ (&#306) and ij (&#307)
      result = result.replace(QRegularExpression("[\\x{0132}]"), QString("IJ"));
      result = result.replace(QRegularExpression("[\\x{0133}]"), QString("ij"));

      // Eszett (&#223)
      result = result.replace(QRegularExpression("[\\x{00DF}]"), QString("ss"));

      // O with stroke (&#216) and o with stroke (&#248)
      result = result.replace(QRegularExpression("[\\x{00D8}]"), QChar('O'));
      result = result.replace(QRegularExpression("[\\x{00F8}]"), QChar('o'));

      // Big Letter OE (&#338) and small letter oe (&#339), O with above dots, o with above dots
      result = result.replace(QRegularExpression("[\\x{0152}\\x{00D6}]"), QString("OE"));
      result = result.replace(QRegularExpression("[\\x{0153}\\x{00F6}]"), QString("oe"));

      // Big Letter OO (&#42830) and small letter oo (&#42831)
      result = result.replace(QRegularExpression("[\\x{A74E}]"), QString("OO"));
      result = result.replace(QRegularExpression("[\\x{A74F}]"), QString("oo"));

      // ue (&#7531), U with above dots, u with above dots
      result = result.replace(QRegularExpression("[\\x{00DC}]"), QString("UE"));
      result = result.replace(QRegularExpression("[\\x{00FC}\\x{1D6B}]"), QString("ue"));

      return result;
      }

//---------------------------------------------------------
//   removeDiacritics
//---------------------------------------------------------

QString stringutils::removeDiacritics(const QString& pre)
      {
      QString text = pre.normalized(QString::NormalizationForm_KD);
      QString result;
      for (int i = 0; i < text.length(); ++i)  {
            if (text.at(i).category() != QChar::Mark_NonSpacing) {
                  result.append(text.at(i));
                  }
            }
      result = result.normalized(QString::NormalizationForm_C);
      return result;
      }

} // namespace Ms
