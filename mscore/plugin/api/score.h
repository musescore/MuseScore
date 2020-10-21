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
#include "style.h"
#include "excerpt.h"
#include "libmscore/score.h"

namespace Ms {

class InstrumentTemplate;

namespace PluginAPI {

class Cursor;
class Segment;
class Measure;
class Selection;
class Score;
class Staff;

extern Selection* selectionWrap(Ms::Selection* select);

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public Ms::PluginAPI::ScoreElement {
      Q_OBJECT
      /** Composer of the score, as taken from the score properties (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(QString                        composer          READ composer)
      /** Duration of score in seconds (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(int                            duration          READ duration)
      /** List of the excerpts (linked parts) (read only) */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Excerpt>  excerpts   READ excerpts)
      /** First measure of the score (read only) */
      Q_PROPERTY(Ms::PluginAPI::Measure*        firstMeasure      READ firstMeasure)
      /**
       * First multimeasure rest measure of the score (read only).
       * \see \ref Measure.nextMeasureMM
       * \since MuseScore 3.2
       */
      Q_PROPERTY(Ms::PluginAPI::Measure*        firstMeasureMM    READ firstMeasureMM)
      /** Number of harmony items (chord symbols) in the score (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(int                            harmonyCount      READ harmonyCount)
      /** Whether score has harmonies (chord symbols) (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(bool                           hasHarmonies      READ hasHarmonies)
      /** Whether score has lyrics (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(bool                           hasLyrics         READ hasLyrics)
      /// Key signature at the start of the score, in number of accidentals,
      /// negative for flats, postitive for sharps (read only).\n \since MuseScore 3.2
      Q_PROPERTY(int                            keysig            READ keysig)
      /** Last measure of the score (read only) */
      Q_PROPERTY(Ms::PluginAPI::Measure*        lastMeasure       READ lastMeasure)
      /**
       * Last multimeasure rest measure of the score (read only).
       * \see \ref Measure.prevMeasureMM
       * \since MuseScore 3.2
       */
      Q_PROPERTY(Ms::PluginAPI::Measure*        lastMeasureMM     READ lastMeasureMM)
      /** Last score segment (read only) */
      Q_PROPERTY(Ms::PluginAPI::Segment*        lastSegment       READ lastSegment) // TODO: make it function? Was property in 2.X, but firstSegment is a function...
      /** Number of lyrics items (syllables) in the score (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(int                            lyricCount        READ lyricCount)
      /** Name of the score, without path leading to it and extension.\n \since MuseScore 3.2 */
      Q_PROPERTY(QString                        scoreName         READ name           WRITE setName)
      /** Number of measures (read only) */
      Q_PROPERTY(int                            nmeasures         READ nmeasures)
      /** Number of pages (read only) */
      Q_PROPERTY(int                            npages            READ npages)
      /** Number of staves (read only) */
      Q_PROPERTY(int                            nstaves           READ nstaves)
      /** Number of tracks (#nstaves * 4) (read only) */
      Q_PROPERTY(int                            ntracks           READ ntracks)
//      Q_PROPERTY(Ms::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
      /** The list of parts */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Part>     parts      READ parts)
      /** Lyricist of score, as taken from the score properties.\n \since MuseScore 3.2 */
      Q_PROPERTY(QString                        lyricist              READ lyricist)
//      Q_PROPERTY(QString                        subtitle          READ subtitle)
      /** Title of score, as taken from the score properties' workTitle (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(QString                        title             READ title)
      /** MuseScore version the score has been last saved with (includes autosave) (read only) */
      Q_PROPERTY(QString                        mscoreVersion     READ mscoreVersion)
      /** MuseScore revision the score has been last saved with (includes autosave) (read only) */
      Q_PROPERTY(QString                        mscoreRevision    READ mscoreRevision)
      /** Current selections for the score. \since MuseScore 3.3 */
      Q_PROPERTY(Ms::PluginAPI::Selection*      selection         READ selection)
      /** Style settings for this score. \since MuseScore 3.5 */
      Q_PROPERTY(Ms::PluginAPI::MStyle*         style             READ style)
      /**
       * Page numbering offset. The user-visible number of the given \p page is defined as
       * \code
       * page.pagenumber + 1 + score.pageNumberOffset
       * \endcode
       * \since MuseScore 3.5
       * \see Page::pagenumber
       */
      Q_PROPERTY(int pageNumberOffset READ pageNumberOffset WRITE setPageNumberOffset)

      /**
       * List of staves in this score.
       * \since MuseScore 3.5
       */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Staff> staves    READ staves)

   public:
      /// \cond MS_INTERNAL
      Score(Ms::Score* s = nullptr, Ownership o = Ownership::SCORE)
         : ScoreElement(s, o) {}

      Ms::Score* score() { return toScore(e); }
      const Ms::Score* score() const { return toScore(e); }

      QString composer() { return score()->metaTag("composer"); }
      int duration() { return score()->duration(); }
      int harmonyCount() { return score()->harmonyCount(); }
      bool hasHarmonies() { return score()->hasHarmonies(); }
      bool hasLyrics() { return score()->hasLyrics(); }
      int keysig() { return score()->keysig(); }
      int lyricCount() { return score()->lyricCount(); }
      QString lyricist() { return score()->metaTag("lyricist"); } // not the meanwhile obsolete "poet"
      QString title() { return score()->metaTag("workTitle"); }
      Ms::PluginAPI::Selection* selection() { return selectionWrap(&score()->selection()); }
      MStyle* style() { return wrap(&score()->style(), score()); }

      int pageNumberOffset() const { return score()->pageNumberOffset(); }
      void setPageNumberOffset(int offset) { score()->undoChangePageNumberOffset(offset); }

      /// \endcond

      /// Returns as a string the metatag named \p tag
      Q_INVOKABLE QString metaTag(const QString& tag) const { return score()->metaTag(tag); }
      /// Sets the metatag named \p tag to \p val
      Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { score()->setMetaTag(tag, val); }

      /**
       * Appends a part with the instrument defined by \p instrumentId
       * to this score.
       * \param instrumentId - ID of the instrument to be added, as listed in
       * [`instruments.xml`](https://github.com/musescore/MuseScore/blob/3.x/share/instruments/instruments.xml)
       * file.
       * \since MuseScore 3.5
       */
      Q_INVOKABLE void appendPart(const QString& instrumentId);
      /**
       * Appends a part with the instrument defined by the given MusicXML ID
       * to this score.
       * \param instrumentMusicXmlId -
       * [MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/)
       * of the instrument to be added.
       * \see \ref Ms::PluginAPI::Part::instrumentId, \ref Ms::PluginAPI::Instrument::instrumentId
       * \since MuseScore 3.5
       */
      Q_INVOKABLE void appendPartByMusicXmlId(const QString& instrumentMusicXmlId);

      /// Appends a number of measures to this score.
      Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
      Q_INVOKABLE void addText(const QString& type, const QString& text);
      /// Creates and returns a cursor to be used to navigate in the score
      Q_INVOKABLE Ms::PluginAPI::Cursor* newCursor();

      Q_INVOKABLE Ms::PluginAPI::Segment* firstSegment(); // TODO: segment type
      /// \cond MS_INTERNAL
      Segment* lastSegment();

      Measure* firstMeasure();
      Measure* firstMeasureMM();
      Measure* lastMeasure();
      Measure* lastMeasureMM();

      QString name() const { return score()->masterScore()->title(); }
      void setName(const QString& name) { score()->masterScore()->setName(name); }
      /// \endcond

      Q_INVOKABLE QString extractLyrics() { return score()->extractLyrics(); }

//      //@ ??
//      Q_INVOKABLE void updateRepeatList(bool expandRepeats) { score()->updateRepeatList(); } // TODO: needed?

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
      Q_INVOKABLE void startCmd();
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

      /**
       * Create PlayEvents for all notes based on ornamentation.
       * You need to call this if you are manipulating PlayEvent's
       * so that all ornamentations are populated into Note's
       * PlayEvent lists.
       * \since 3.3
       */
      Q_INVOKABLE void createPlayEvents() { score()->createPlayEvents(); }

      /// \cond MS_INTERNAL
      QString mscoreVersion() { return score()->mscoreVersion(); }
      QString mscoreRevision() { return QString::number(score()->mscoreRevision(), /* base */ 16); }

      QQmlListProperty<Part> parts() { return wrapContainerProperty<Part>(this, score()->parts());   }
      QQmlListProperty<Excerpt> excerpts() { return wrapExcerptsContainerProperty<Excerpt>(this, score()->excerpts());   }
      QQmlListProperty<Staff> staves();

      static const Ms::InstrumentTemplate* instrTemplateFromName(const QString& name); // used by PluginAPI::newScore()
      /// \endcond
      };
} // namespace PluginAPI
} // namespace Ms
#endif
