//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

#include "musescore.h"
#include "libmscore/score.h"

class Cursor;

//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

class QmlPlugin : public QDeclarativeItem {
      Q_OBJECT
      Q_PROPERTY(QString menuPath READ menuPath WRITE setMenuPath)
      Q_PROPERTY(bool hasGui READ hasGui WRITE setHasGui)
      Q_PROPERTY(int mscoreVersion READ mscoreVersion)
      Q_PROPERTY(Score* curScore READ curScore)

      QString _menuPath;
      bool _hasGui;

   signals:
      void run();

   public:
      QmlPlugin(QDeclarativeItem* parent = 0);
      ~QmlPlugin();

      void setMenuPath(const QString& s) { _menuPath = s;    }
      QString menuPath() const           { return _menuPath; }
      bool hasGui() const                { return _hasGui;   }
      void setHasGui(bool val)           { _hasGui = val;    }
      int mscoreVersion() const;
      void runPlugin()                   { emit run();       }
      Score* curScore()                  { return mscore->currentScore(); }
      Q_INVOKABLE Cursor* newCursor();
      };


#endif

