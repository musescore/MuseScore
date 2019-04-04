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
      /** The list of the excerpts (linked parts) */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Excerpt>  excerpts   READ excerpts)
      /** The first measure of the score */
      Q_PROPERTY(Ms::PluginAPI::Measure*       firstMeasure      READ firstMeasure)
//       Q_PROPERTY(Ms::Measure*                   firstMeasureMM    READ firstMeasureMM)
//       Q_PROPERTY(int                            harmonyCount      READ harmonyCount)
//       Q_PROPERTY(bool                           hasHarmonies      READ hasHarmonies)
//       Q_PROPERTY(bool                           hasLyrics         READ hasLyrics)
//       Q_PROPERTY(int                            keysig            READ keysig)
      /** The last measure of the score */
      Q_PROPERTY(Ms::PluginAPI::Measure*       lastMeasure       READ lastMeasure)
//       Q_PROPERTY(Ms::Measure*                   lastMeasureMM     READ lastMeasureMM)
      /** The last score segment */
      Q_PROPERTY(Ms::PluginAPI::Segment*       lastSegment       READ lastSegment) // TODO: make it function? Was property in 2.X, but firstSegment is a function...
//       Q_PROPERTY(int                            lyricCount        READ lyricCount)
// //       Q_PROPERTY(QString                        name              READ name           WRITE setName)
      /** Number of measures */
      Q_PROPERTY(int                            nmeasures         READ nmeasures)
      /** Number of pages */
      Q_PROPERTY(int                            npages            READ npages)
      /** Number of staves */
      Q_PROPERTY(int                            nstaves           READ nstaves)
      /** Number of tracks */
      Q_PROPERTY(int                            ntracks           READ ntracks)
//       Q_PROPERTY(Ms::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
      /** The list of parts */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Part>     parts      READ parts)
//       Q_PROPERTY(QString                        poet              READ poet)
//       Q_PROPERTY(QString                        subtitle          READ subtitle)
//       Q_PROPERTY(QString                        title             READ title)
      Q_PROPERTY(QString                        mscoreVersion     READ mscoreVersion)
      Q_PROPERTY(QString                        mscoreRevision    READ mscoreRevision)

   public:
      /// \cond MS_INTERNAL
      Score(Ms::Score* s = nullptr, Ownership o = Ownership::SCORE)
         : ScoreElement(s, o) {}

      Ms::Score* score() { return toScore(e); }
      const Ms::Score* score() const { return toScore(e); }
      /// \endcond

      /// Returns as a string the metatag named \p tag
      Q_INVOKABLE QString metaTag(const QString& tag) const { return score()->metaTag(tag); }
      /// Sets the metatag named \p tag to \p val
      Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { score()->setMetaTag(tag, val); }

//       //@ appends to the score a named part as last part
//       Q_INVOKABLE void appendPart(const QString&);
      /// Appends a number of measures to this score.
      Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
      Q_INVOKABLE void addText(const QString& type, const QString& text);
      /// Creates and returns a cursor to be used to navigate the score
      Q_INVOKABLE Ms::PluginAPI::Cursor* newCursor();

      Q_INVOKABLE Ms::PluginAPI::Segment* firstSegment(); // TODO: segment type
      /// \cond MS_INTERNAL
      Segment* lastSegment();

      Measure* firstMeasure();
      Measure* lastMeasure();
      /// \endcond

// //       QString name() const { return score()->name(); }
// //       void setName(const QString& name) { score()->setName(name); } // TODO: MasterScore

      Q_INVOKABLE QString extractLyrics() { return score()->extractLyrics(); }

//       //@ ??
//       Q_INVOKABLE void updateRepeatList(bool expandRepeats) { score()->updateRepeatList(); } // TODO: needed?

      /// \cond MS_INTERNAL
      int nmeasures() const { return score()->nmeasures(); }
      int npages() const { return score()->npages(); }
      int nstaves() const { return score()->nstaves(); }
      int ntracks() const { return score()->ntracks(); }
      /// \endcond

      /**
       * For "dock" type plugins: to be used before score
       * modifications to make them undoable.
       * Starts an undoable command. Must be accompanied by
       * a corresponding endCmd() call. Should be used at
       * least once by "dock" type plugins in case they
       * modify the score.
       */
      Q_INVOKABLE void startCmd() { score()->startCmd(); }
      /**
       * For "dock" type plugins: to be used after score
       * modifications to make them undoable.
       * Ends an undoable command. Should be used at least
       * once by "dock" type plugins in case they modify
       * the score.
       * \param rollback If true, reverts all the changes
       * made since the last startCmd() invocation.
       */
      Q_INVOKABLE void endCmd(bool rollback = false) { score()->endCmd(rollback); }

      /// \cond MS_INTERNAL
      QString mscoreVersion() { return score()->mscoreVersion(); }
      QString mscoreRevision() { return QString::number(score()->mscoreRevision(), /* base */ 16); }

      QQmlListProperty<Part> parts() { return wrapContainerProperty<Part>(this, score()->parts());   }
      QQmlListProperty<Excerpt> excerpts() { return wrapExcerptsContainerProperty<Excerpt>(this, score()->excerpts());   }
      /// \endcond
      };
} // namespace PluginAPI
} // namespace Ms
#endif
