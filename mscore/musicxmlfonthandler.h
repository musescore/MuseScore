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
      MScoreTextToMXML(const QString& tag, const QString& attr, const QString& t, const int deffs, const int curfs);
      static QString toPlainText(const QString& text);
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
