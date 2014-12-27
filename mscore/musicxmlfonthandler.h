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

#ifndef __MUSICXMLFONTHANDLER_H__
#define __MUSICXMLFONTHANDLER_H__

#include "libmscore/text.h"

namespace Ms {

//---------------------------------------------------------
//   MScoreTextToMXML
//---------------------------------------------------------

class MScoreTextToMXML {

public:
      MScoreTextToMXML(const QString& tag, const QString& attr, const QString& t, const TextStyle& defTs, const TextStyle& actTs);
      MScoreTextToMXML(const QString& tag, const QString& attr, const CharFormat& defFmt);
      static QString toPlainText(const QString& text);
      static QString toPlainTextPlusSymbols(const QList<TextFragment>& list);
      static bool split(const QList<TextFragment>& in, const int pos, const int len,
                        QList<TextFragment>& left, QList<TextFragment>& mid, QList<TextFragment>& right);
      void writeTextFragments(const QList<TextFragment>& fr, Xml& xml);
      void write(Xml& xml);

private:
      void handleStartElement(QXmlStreamReader& r);
      void handleEndElement(QXmlStreamReader& r);
      QString updateFormat();
      QString attribs;
      QString tagname;
      QString text;
      CharFormat oldFormat;
      CharFormat newFormat;
};

} // namespace Ms

#endif
