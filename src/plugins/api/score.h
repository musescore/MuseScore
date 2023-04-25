/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __PLUGIN_API_SCORE_H__
#define __PLUGIN_API_SCORE_H__

#include "scoreelement.h"
#include "part.h"
#include "style.h"
#include "excerpt.h"
#include "libmscore/masterscore.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::engraving {
class InstrumentTemplate;
}

namespace mu::plugins::api {
class Cursor;
class Segment;
class Measure;
class Selection;
class Score;
class Staff;

extern Selection* selectionWrap(mu::engraving::Selection* select);

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public mu::plugins::api::ScoreElement
{
    Q_OBJECT

    INJECT(plugins, mu::context::IGlobalContext, context)

    /** Composer of the score, as taken from the score properties (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(QString composer READ composer)
    /** Duration of score in seconds (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(int duration READ duration)
    /** List of the excerpts (linked parts) (read only) */
    Q_PROPERTY(QQmlListProperty<mu::plugins::api::Excerpt> excerpts READ excerpts)
    /** First measure of the score (read only) */
    Q_PROPERTY(mu::plugins::api::Measure * firstMeasure READ firstMeasure)
    /**
     * First multimeasure rest measure of the score (read only).
     * \see \ref Measure.nextMeasureMM
     * \since MuseScore 3.2
     */
    Q_PROPERTY(mu::plugins::api::Measure * firstMeasureMM READ firstMeasureMM)
    /** Number of harmony items (chord symbols) in the score (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(int harmonyCount READ harmonyCount)
    /** Whether score has harmonies (chord symbols) (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(bool hasHarmonies READ hasHarmonies)
    /** Whether score has lyrics (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(bool hasLyrics READ hasLyrics)
    /// Key signature at the start of the score, in number of accidentals,
    /// negative for flats, positive for sharps (read only).\n \since MuseScore 3.2
    Q_PROPERTY(int keysig READ keysig)
    /** Last measure of the score (read only) */
    Q_PROPERTY(mu::plugins::api::Measure * lastMeasure READ lastMeasure)
    /**
     * Last multimeasure rest measure of the score (read only).
     * \see \ref Measure.prevMeasureMM
     * \since MuseScore 3.2
     */
    Q_PROPERTY(mu::plugins::api::Measure * lastMeasureMM READ lastMeasureMM)
    /** Last score segment (read only) */
    Q_PROPERTY(mu::plugins::api::Segment * lastSegment READ lastSegment)                // TODO: make it function? Was property in 2.X, but firstSegment is a function...
    /** Number of lyrics items (syllables) in the score (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(int lyricCount READ lyricCount)
    /** Name of the score, without path leading to it and extension.\n \since MuseScore 3.2 */
    Q_PROPERTY(QString scoreName READ name WRITE setName)
    /** Number of measures (read only) */
    Q_PROPERTY(int nmeasures READ nmeasures)
    /** Number of pages (read only) */
    Q_PROPERTY(int npages READ npages)
    /** Number of staves (read only) */
    Q_PROPERTY(int nstaves READ nstaves)
    /** Number of tracks (#nstaves * 4) (read only) */
    Q_PROPERTY(int ntracks READ ntracks)
//      Q_PROPERTY(mu::engraving::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
    /** The list of parts */
    Q_PROPERTY(QQmlListProperty<mu::plugins::api::Part> parts READ parts)
    /** Lyricist of score, as taken from the score properties.\n \since MuseScore 3.2 */
    Q_PROPERTY(QString lyricist READ lyricist)
//      Q_PROPERTY(QString                        subtitle          READ subtitle)
    /** Title of score, as taken from the score properties' workTitle (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(QString title READ title)
    /** MuseScore version the score has been last saved with (includes autosave) (read only) */
    Q_PROPERTY(QString mscoreVersion READ mscoreVersion)
    /** MuseScore revision the score has been last saved with (includes autosave) (read only) */
    Q_PROPERTY(QString mscoreRevision READ mscoreRevision)
    /** Current selections for the score. \since MuseScore 3.3 */
    Q_PROPERTY(mu::plugins::api::Selection * selection READ selection)
    /** Style settings for this score. \since MuseScore 3.5 */
    Q_PROPERTY(mu::plugins::api::MStyle * style READ style)
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
     * \since MuseScore 3.6.3
     */
    Q_PROPERTY(QQmlListProperty<mu::plugins::api::Staff> staves READ staves)

public:
    /// \cond MS_INTERNAL
    Score(mu::engraving::Score* s = nullptr, Ownership o = Ownership::SCORE)
        : ScoreElement(s, o) {}

    mu::engraving::Score* score() { return toScore(e); }
    const mu::engraving::Score* score() const { return toScore(e); }

    QString composer() { return score()->metaTag(u"composer"); }
    int duration() { return score()->duration(); }
    int harmonyCount() { return score()->harmonyCount(); }
    bool hasHarmonies() { return score()->hasHarmonies(); }
    bool hasLyrics() { return score()->hasLyrics(); }
    int keysig() { return score()->keysig(); }
    int lyricCount() { return score()->lyricCount(); }
    QString lyricist() { return score()->metaTag(u"lyricist"); }   // not the meanwhile obsolete "poet"
    QString title() { return score()->metaTag(u"workTitle"); }
    mu::plugins::api::Selection* selection() { return selectionWrap(&score()->selection()); }
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
     * \see \ref mu::plugins::api::Part::instrumentId, \ref mu::plugins::api::Instrument::instrumentId
     * \since MuseScore 3.5
     */
    Q_INVOKABLE void appendPartByMusicXmlId(const QString& instrumentMusicXmlId);

    /// Appends a number of measures to this score.
    Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
    Q_INVOKABLE void addText(const QString& type, const QString& text);
    /// Creates and returns a cursor to be used to navigate in the score
    Q_INVOKABLE mu::plugins::api::Cursor* newCursor();

    Q_INVOKABLE mu::plugins::api::Segment* firstSegment();   // TODO: segment type
    /// \cond MS_INTERNAL
    Segment* lastSegment();

    Measure* firstMeasure();
    Measure* firstMeasureMM();
    Measure* lastMeasure();
    Measure* lastMeasureMM();

    QString name() const { return score()->masterScore()->name(); }
    void setName(const QString& /*name*/) { NOT_IMPLEMENTED; }
    /// \endcond

    Q_INVOKABLE QString extractLyrics() { return score()->extractLyrics(); }

    /// \cond MS_INTERNAL
    int nmeasures() const { return static_cast<int>(score()->nmeasures()); }
    int npages() const { return static_cast<int>(score()->npages()); }
    int nstaves() const { return static_cast<int>(score()->nstaves()); }
    int  ntracks() const { return static_cast<int>(score()->ntracks()); }
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
    Q_INVOKABLE void endCmd(bool rollback = false);

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

    QQmlListProperty<Part> parts() { return wrapContainerProperty<Part>(this, score()->parts()); }
    QQmlListProperty<Excerpt> excerpts() { return wrapExcerptsContainerProperty<Excerpt>(this, score()->masterScore()->excerpts()); }
    QQmlListProperty<Staff> staves();

    static const mu::engraving::InstrumentTemplate* instrTemplateFromName(const QString& name);   // used by PluginAPI::newScore()
    /// \endcond

private:
    mu::notation::INotationPtr notation() const;
    mu::notation::INotationUndoStackPtr undoStack() const;
};
} // namespace mu::plugins::api

#endif
