//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2014 Werner Schweer and others
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

/**
 MusicXML font handling support.
 */

#include "libmscore/sym.h"
#include "libmscore/xml.h"
#include "musicxmlfonthandler.h"

namespace Ms {

//---------------------------------------------------------
//   charFormat2QString
//    convert charFormat to QString for debug print
//---------------------------------------------------------

#if 0
static QString charFormat2QString(const CharFormat& f)
      {
      return QString("b %1 i %2 u %3 s %4 va %5 fs %6 fam %7")
            .arg(f.bold())
            .arg(f.italic())
            .arg(f.underline())
            .arg(f.strike())
            .arg(static_cast<int>(f.valign()))
            .arg(f.fontSize())
            .arg(f.fontFamily())
                 ;
      }

void dumpText(const QList<TextFragment>& list)
      {
      qDebug("MScoreTextToMXML::dumpText %d fragment(s)", list.size());
      for (const TextFragment& f : list) {
            QString t = "fragment";
            if (f.format.type() == CharFormatType::TEXT) {
                  t += QString(" text '%1'").arg(f.text);
                  t += QString(" len %1").arg(f.text.size());
                  }
            else {
                  t += " syms";
                  int len = 0;
                  for (const SymId id : f.ids) {
                        t += QString(" '%1'").arg(Sym::id2name(id));
                        QString s = QString("<sym>%1</sym>").arg(Sym::id2name(id));
                        len += s.size();
                        }
                  t += QString(" len %1").arg(len);
                  }
            t += " format ";
            t += charFormat2QString(f.format);
            qDebug("%s", qPrintable(t));
            }
      }
#endif

//---------------------------------------------------------
//   MScoreTextToMXML
//---------------------------------------------------------

MScoreTextToMXML::MScoreTextToMXML(const QString& tag, const QString& attr, const CharFormat& defFmt, const QString& mtf)
      : attribs(attr), tagname(tag), oldFormat(defFmt), musicalTextFont(mtf)
      {
      // set MusicXML defaults
      oldFormat.setBold(false);
      oldFormat.setItalic(false);
      oldFormat.setUnderline(false);
      oldFormat.setStrike(false);
      }

//---------------------------------------------------------
//   toPlainText
//    convert to plain text
//    naive implementation: simply remove all chars from '<' to '>'
//    typically used to remove formatting info from fields read
//    from MuseScore 1.3 file where they are stored as html, such as
//    part name and shortName
//---------------------------------------------------------

QString MScoreTextToMXML::toPlainText(const QString& text)
      {
      QString res;
      bool inElem = false;
      foreach(QChar ch, text) {
            if (ch == '<')
                  inElem = true;
            else if (ch == '>')
                  inElem = false;
            else {
                  if (!inElem)
                        res += ch;
                  }
            }
      //qDebug("MScoreTextToMXML::toPlainText('%s') res '%s'", qPrintable(text), qPrintable(res));
      return res;
      }

//---------------------------------------------------------
//   toPlainTextPlusSymbols
//    convert to plain text plus <sym>[name]</sym> encoded symbols
//---------------------------------------------------------

QString MScoreTextToMXML::toPlainTextPlusSymbols(const QList<TextFragment>& list)
      {
      QString res;
      for (const TextFragment& f : list)
            res += f.text;
      return res;
      }

//---------------------------------------------------------
//   plainTextPlusSymbolsSize
//---------------------------------------------------------

static int plainTextPlusSymbolsFragmentSize(const TextFragment& f)
      {
      return f.columns();
      }

//---------------------------------------------------------
//   plainTextPlusSymbolsSize
//---------------------------------------------------------

static int plainTextPlusSymbolsListSize(const QList<TextFragment>& list)
      {
      int res = 0;
      for (const TextFragment& f : list) {
            res += plainTextPlusSymbolsFragmentSize(f);
            }
      return res;
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

/**
 Split \a in into \a left, \a mid and \a right. Mid starts at \a pos and is \a len characters long.
 Pos and len refer to the representation returned by toPlainTextPlusSymbols().
 TODO Make sure surrogate pairs are handled correctly
 Return true if OK, false on error.
 */

bool MScoreTextToMXML::split(const QList<TextFragment>& in, const int pos, const int len,
                             QList<TextFragment>& left, QList<TextFragment>& mid, QList<TextFragment>& right)
      {
      //qDebug("MScoreTextToMXML::split in size %d pos %d len %d", plainTextPlusSymbolsListSize(in), pos, len);
      //qDebug("-> in");
      //dumpText(in);

      if (pos < 0 || len < 0)
            return false;

      // ensure output is empty at start
      left.clear();
      mid.clear();
      right.clear();

      // set pos to begin of first fragment
      int fragmentNr = 0;
      TextFragment fragment;
      if (fragmentNr < in.size()) fragment = in.at(fragmentNr);
      QList<TextFragment>* currentDest = &left;
      int currentMaxSize = pos;

      // while text left
      while (fragmentNr < in.size()) {
            int destSize = plainTextPlusSymbolsListSize(*currentDest);
            int fragSize = plainTextPlusSymbolsFragmentSize(fragment);
            // if no room left in current destination (check applies only to left and mid)
            if ((currentDest != &right && destSize >= currentMaxSize)
                || currentDest == &right) {
                  // move to next destination
                  if (currentDest == &left) {
                        currentDest = &mid;
                        currentMaxSize = len;
                        }
                  else if (currentDest == &mid) {
                        currentDest = &right;
                        }
                  }
            // if current fragment fits in current destination (check applies only to left and mid)
            if ((currentDest != &right && destSize + fragSize <= currentMaxSize)
                || currentDest == &right) {
                  // add it
                  currentDest->append(fragment);
                  // move to next fragment
                  fragmentNr++;
                  if (fragmentNr < in.size()) fragment = in.at(fragmentNr);
                  }
            else {
                  // split current fragment
                  TextFragment rightPart = fragment.split(currentMaxSize - plainTextPlusSymbolsListSize(*currentDest));
                  // add first part to current destination
                  currentDest->append(fragment);
                  fragment = rightPart;
                  }
            }

      /*
      qDebug("-> left");
      dumpText(left);
      qDebug("-> mid");
      dumpText(mid);
      qDebug("-> right");
      dumpText(right);
       */

      return true;
      }

//---------------------------------------------------------
//   writeTextFragments
//---------------------------------------------------------

void MScoreTextToMXML::writeTextFragments(const QList<TextFragment>& fr, XmlWriter& xml)
      {
      //qDebug("MScoreTextToMXML::writeTextFragments");
      //dumpText(fr);
      bool firstTime = true; // write additional attributes only the first time characters are written
      for (const TextFragment& f : fr) {
            newFormat = f.format;
            QString formatAttr = updateFormat();
            xml.tag(tagname + (firstTime ? attribs : "") + formatAttr, f.text);
            firstTime = false;
            }
      }

//---------------------------------------------------------
//   attribute
//    add one attribute if necessary
//---------------------------------------------------------

static QString attribute(bool needed, bool value, QString trueString, QString falseString)
      {
      QString res;
      if (needed)
            res = value ? trueString : falseString;
      if (!res.isEmpty())
            res = " " + res;
      return res;
      }

//---------------------------------------------------------
//   updateFormat
//    update the text format by generating attributes
//    corresponding to the difference between old- and newFormat
//    copy newFormat to oldFormat
//---------------------------------------------------------

QString MScoreTextToMXML::updateFormat()
      {
      if (newFormat.fontFamily() == "ScoreText")
            newFormat.setFontFamily(musicalTextFont);
      QString res;
      res += attribute(newFormat.bold() != oldFormat.bold(), newFormat.bold(), "font-weight=\"bold\"", "font-weight=\"normal\"");
      res += attribute(newFormat.italic() != oldFormat.italic(), newFormat.italic(), "font-style=\"italic\"", "font-style=\"normal\"");
      res += attribute(newFormat.underline() != oldFormat.underline(), newFormat.underline(), "underline=\"1\"", "underline=\"0\"");
      res += attribute(newFormat.strike() != oldFormat.strike(), newFormat.strike(), "line-through=\"1\"", "line-through=\"0\"");
      res += attribute(newFormat.fontFamily() != oldFormat.fontFamily(), true, QString("font-family=\"%1\"").arg(newFormat.fontFamily()), "");
      bool needSize = newFormat.fontSize() < 0.99 * oldFormat.fontSize() || newFormat.fontSize() > 1.01 * oldFormat.fontSize();
      res += attribute(needSize, true, QString("font-size=\"%1\"").arg(newFormat.fontSize()), "");
      //qDebug("updateFormat() res '%s'", qPrintable(res));
      oldFormat = newFormat;
      return res;
      }

} // namespace Ms
