//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"

namespace Ms {

//---------------------------------------------------------
//   read300
//---------------------------------------------------------

Score::FileError MasterScore::read300(XmlReader& e)
      {
      qDebug("read300");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readInt());
            else if (tag == "Score") {
                  if (!read(e))
                        return FileError::FILE_BAD_FORMAT;
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }

      int id = 1;
      for (LinkedElements* le : e.linkIds())
            le->setLid(this, id++);

      for (Staff* s : staves())
            s->updateOttava();

      setCreated(false);
      return FileError::FILE_NO_ERROR;
      }

}

