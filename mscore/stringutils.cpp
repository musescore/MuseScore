//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  Copyright (C) 2002-2018 Werner Schweer and others
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

      // Characters with above dots (Ae, ae, Oe, oe, Ue, ue
      result = result.replace(QRegularExpression("[\\x{00C4}]"), QString("Ae"));
      result = result.replace(QRegularExpression("[\\x{00E4}]"), QString("ae"));
      result = result.replace(QRegularExpression("[\\x{00D6}]"), QString("Oe"));
      result = result.replace(QRegularExpression("[\\x{00F6}]"), QString("oe"));
      result = result.replace(QRegularExpression("[\\x{00DC}]"), QString("Ue"));
      result = result.replace(QRegularExpression("[\\x{00FC}]"), QString("ue"));

      // Latin Big Letter AA (&#42802) and Latin Small Letter aa (&#42803)
      result = result.replace(QRegularExpression("[\\x{A732}]"), QString("Aa"));
      result = result.replace(QRegularExpression("[\\x{A733}]"), QString("aa"));

      // Latin Big Letter AE (&#198) and Latin Small Letter ae (&#230)
      result = result.replace(QRegularExpression("[\\x{00C6}]"), QString("Ae"));
      result = result.replace(QRegularExpression("[\\x{00E6}]"), QString("ae"));

      // Latin Big Letter AO (&#42804) and Latin Small Letter ao (&#42805)
      result = result.replace(QRegularExpression("[\\x{A734}]"), QString("Ao"));
      result = result.replace(QRegularExpression("[\\x{A735}]"), QString("ao"));

      // Latin Big Letter AU (&#42806) and Latin Small Letter au (&#42807)
      result = result.replace(QRegularExpression("[\\x{A736}]"), QString("Au"));
      result = result.replace(QRegularExpression("[\\x{A737}]"), QString("au"));

      // IJ (&#306) and ij (&#307)
      result = result.replace(QRegularExpression("[\\x{0132}]"), QString("IJ"));
      result = result.replace(QRegularExpression("[\\x{0133}]"), QString("ij"));

      // Eszett SS (&#7838) and ss (&#223)
      result = result.replace(QRegularExpression("[\\x{1E9E}]"), QString("SS"));
      result = result.replace(QRegularExpression("[\\x{00DF}]"), QString("ss"));

      // O with stroke (&#216) and o with stroke (&#248)
      result = result.replace(QRegularExpression("[\\x{00D8}]"), QChar('O'));
      result = result.replace(QRegularExpression("[\\x{00F8}]"), QChar('o'));

      // Big Letter OE (&#338) and small letter oe (&#339)
      result = result.replace(QRegularExpression("[\\x{0152}]"), QString("Oe"));
      result = result.replace(QRegularExpression("[\\x{0153}]"), QString("oe"));

      // Big Letter OO (&#42830) and small letter oo (&#42831)
      result = result.replace(QRegularExpression("[\\x{A74E}]"), QString("Oo"));
      result = result.replace(QRegularExpression("[\\x{A74F}]"), QString("oo"));

      // ue (&#7531)
      result = result.replace(QRegularExpression("[\\x{1D6B}]"), QString("ue"));

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

//---------------------------------------------------------
//   convertFileSizeToHumanReadable
//---------------------------------------------------------

QString stringutils::convertFileSizeToHumanReadable(const qlonglong& bytes)
      {
      // If less than 1 KB, report in B
      if (bytes < 0x400) {
            return tr("%1 B").arg(bytes);
            }
      // If less than 1 MB, report in KB, unless rounded result is 1024 KB, then report in MB
      else if (bytes >= 0x400 && bytes < 0x100000) {
            qlonglong result = (bytes + (0x400 / 2)) / 0x400;
            if (result < 0x400) {
                  return tr("%1 KB").arg(result);
                  }
            else {
                  result = (bytes + (0x100000 / 2)) / 0x100000;
                  return tr("%1 MB").arg(result);
                  }
            }
      // If less than 1 GB, report in MB, unless rounded result is 1024 MB, then report in GB
      else if (bytes >= 0x100000 && bytes < 0x40000000) {
            qlonglong result = (bytes + (0x100000 / 2)) / 0x100000;
            if (result < 0x100000) {
                  return tr("%1 MB").arg(result);
                  }
            else {
                  result = (bytes + (0x40000000 / 2)) / 0x40000000;
                  return tr("%1 GB").arg(result);
                  }
            }
      // If less than 1 TB, report in GB, unless rounded result is 1024 GB, then report in TB
      else if (bytes >= 0x40000000 && bytes < 0x10000000000) {
            qlonglong result = (bytes + (0x40000000 / 2)) / 0x40000000;
            if (result < 0x40000000) {
                  return tr("%1 GB").arg(result);
                  }
            else {
                  result = (bytes + (0x10000000000 / 2)) / 0x10000000000;
                  return tr("%1 TB").arg(result);
                  }
            }
      // If more than 1 TB, report in TB
      else {
            qlonglong result = (bytes + (0x10000000000 / 2)) / 0x10000000000;
            return tr("%1 TB").arg(result);
            }
      }

} // namespace Ms
