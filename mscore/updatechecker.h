//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

namespace Ms {

//---------------------------------------------------------
//   UpdateChecker
//---------------------------------------------------------
class UpdateCheckerBase: public QObject {
      Q_OBJECT

public:
      UpdateCheckerBase(QObject* parent);
      virtual bool hasToCheck();

private:
      virtual bool getUpdatePrefValue() = 0;
      virtual QString getUpdatePrefString() = 0;
      };

class UpdateChecker : public UpdateCheckerBase {
      Q_OBJECT

      QNetworkAccessManager* manager;
      QString os;
      QString release;
      QString _currentVersion;
      bool manual;

   public:
      void check(QString,bool);


   public slots:
      void onRequestFinished(QNetworkReply*);

   private:
      QString parseText(QXmlStreamReader&);
      virtual bool getUpdatePrefValue();
      virtual QString getUpdatePrefString();

   public:
      UpdateChecker(QObject* parent);
      };

class ExtensionsUpdateChecker : public UpdateCheckerBase {
      Q_OBJECT
public:
      ExtensionsUpdateChecker(QObject* parent);

      void check();
private:
      virtual bool getUpdatePrefValue();
      virtual QString getUpdatePrefString();
      };
}
#endif // UPDATECHECKER_H
