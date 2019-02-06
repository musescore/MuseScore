//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_SCORE_H__
#define __PLUGIN_API_SCORE_H__

#include "scoreelement.h"
#include "part.h"
#include "excerpt.h"
#include "libmscore/score.h"

namespace Ms {
namespace PluginAPI {

class Cursor;
class Segment;
class Measure;

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public Ms::PluginAPI::ScoreElement {
      Q_OBJECT
//       Q_PROPERTY(QString                        composer          READ composer)
//       Q_PROPERTY(int                            duration          READ duration)
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Excerpt>  excerpts   READ excerpts)
      Q_PROPERTY(Ms::PluginAPI::Measure*       firstMeasure      READ firstMeasure)
//       Q_PROPERTY(Ms::Measure*                   firstMeasureMM    READ firstMeasureMM)
//       Q_PROPERTY(int                            harmonyCount      READ harmonyCount)
//       Q_PROPERTY(bool                           hasHarmonies      READ hasHarmonies)
//       Q_PROPERTY(bool                           hasLyrics         READ hasLyrics)
//       Q_PROPERTY(int                            keysig            READ keysig)
      Q_PROPERTY(Ms::PluginAPI::Measure*       lastMeasure       READ lastMeasure)
//       Q_PROPERTY(Ms::Measure*                   lastMeasureMM     READ lastMeasureMM)
      Q_PROPERTY(Ms::PluginAPI::Segment*       lastSegment       READ lastSegment) // TODO: make it function? Was property in 2.X, but firstSegment is a function...
//       Q_PROPERTY(int                            lyricCount        READ lyricCount)
// //       Q_PROPERTY(QString                        name              READ name           WRITE setName)
      Q_PROPERTY(int                            nmeasures         READ nmeasures)
      Q_PROPERTY(int                            npages            READ npages)
      Q_PROPERTY(int                            nstaves           READ nstaves)
      Q_PROPERTY(int                            ntracks           READ ntracks)
//       Q_PROPERTY(Ms::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
//       Q_PROPERTY(QQmlListProperty<Ms::Part>     parts             READ qmlParts)
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Part>     parts      READ parts)
//       Q_PROPERTY(QString                        poet              READ poet)
//       Q_PROPERTY(QString                        subtitle          READ subtitle)
//       Q_PROPERTY(QString                        title             READ title)
      Q_PROPERTY(QString                        mscoreVersion     READ mscoreVersion)
      Q_PROPERTY(QString                        mscoreRevision    READ mscoreRevision)

   public:
      Score(Ms::Score* s = nullptr, Ownership o = Ownership::SCORE)
         : ScoreElement(s, o) {}

      Ms::Score* score() { return toScore(e); }
      const Ms::Score* score() const { return toScore(e); }

      //@ returns as a string the metatag named 'tag'
      Q_INVOKABLE QString metaTag(const QString& tag) const { return score()->metaTag(tag); }
      //@ sets the metatag named 'tag' to 'val'
      Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { score()->setMetaTag(tag, val); }

//       //@ appends to the score a named part as last part
//       Q_INVOKABLE void appendPart(const QString&);
      //@ appends to the score a number of measures
      Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
      //@ ??
      Q_INVOKABLE void addText(const QString& type, const QString& text);
      //@ creates and returns a cursor to be used to navigate the score
      Q_INVOKABLE Ms::PluginAPI::Cursor* newCursor();

      Q_INVOKABLE Ms::PluginAPI::Segment* firstSegment(); // TODO: segment type
      Segment* lastSegment();

      Measure* firstMeasure();
      Measure* lastMeasure();

// //       QString name() const { return score()->name(); }
// //       void setName(const QString& name) { score()->setName(name); } // TODO: MasterScore

      Q_INVOKABLE QString extractLyrics() { return score()->extractLyrics(); }

//       //@ ??
//       Q_INVOKABLE void updateRepeatList(bool expandRepeats) { score()->updateRepeatList(); } // TODO: needed?

      int nmeasures() const { return score()->nmeasures(); }
      int npages() const { return score()->npages(); }
      int nstaves() const { return score()->nstaves(); }
      int ntracks() const { return score()->ntracks(); }

      Q_INVOKABLE void startCmd() { score()->startCmd(); }
      Q_INVOKABLE void endCmd(bool rollback = false) { score()->endCmd(rollback); }

      QString mscoreVersion() { return score()->mscoreVersion(); }
      QString mscoreRevision() { return QString::number(score()->mscoreRevision(), /* base */ 16); }

      QQmlListProperty<Part> parts() { return wrapContainerProperty<Part>(this, score()->parts());   }
      QQmlListProperty<Excerpt> excerpts() { return wrapExcerptsContainerProperty<Excerpt>(this, score()->excerpts());   }
      };
} // namespace PluginAPI
} // namespace Ms
#endif
