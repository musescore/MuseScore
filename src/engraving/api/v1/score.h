/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include "scoreelement.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "engraving/dom/masterscore.h"

// api
#include "excerpt.h"
#include "apistructs.h"
#include "style.h"
#include "part.h"
#include "excerpt.h"

Q_MOC_INCLUDE("engraving/api/v1/selection.h")

namespace mu::engraving {
class InstrumentTemplate;
class Selection;
}

namespace mu::engraving::apiv1 {
class Cursor;
class Segment;
class Measure;
class Page;
class MeasureBase;
class System;
class Selection;
class Score;
class Staff;

extern Selection* selectionWrap(mu::engraving::Selection* select);

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public apiv1::ScoreElement, public muse::Injectable
{
    Q_OBJECT

    /// Composer of the score, as taken from the score properties (read only).\n \since MuseScore 3.2
    Q_PROPERTY(QString composer READ composer)
    /// Duration of score in seconds (read only).\n \since MuseScore 3.2
    Q_PROPERTY(int duration READ duration)
    /// List of the excerpts (linked parts) (read only)
    Q_PROPERTY(QQmlListProperty<apiv1::Excerpt> excerpts READ excerpts)
    /// First measure of the score (read only)
    Q_PROPERTY(apiv1::Measure * firstMeasure READ firstMeasure)
    /// First multimeasure rest measure of the score (read only).
    /// \see \ref Measure.nextMeasureMM
    /// \since MuseScore 3.2
    Q_PROPERTY(apiv1::Measure * firstMeasureMM READ firstMeasureMM)
    /// Number of harmony items (chord symbols) in the score (read only).\n \since MuseScore 3.2
    Q_PROPERTY(int harmonyCount READ harmonyCount)
    /// Whether score has harmonies (chord symbols) (read only).\n \since MuseScore 3.2
    Q_PROPERTY(bool hasHarmonies READ hasHarmonies)
    /// Whether score has lyrics (read only).\n \since MuseScore 3.2
    Q_PROPERTY(bool hasLyrics READ hasLyrics)
    /// Key signature at the start of the score, in number of accidentals,
    /// negative for flats, positive for sharps (read only).\n \since MuseScore 3.2
    Q_PROPERTY(int keysig READ keysig)
    /// Last measure of the score (read only)
    Q_PROPERTY(apiv1::Measure * lastMeasure READ lastMeasure)
    /// Last multimeasure rest measure of the score (read only).
    /// \see \ref Measure.prevMeasureMM
    /// \since MuseScore 3.2
    Q_PROPERTY(apiv1::Measure * lastMeasureMM READ lastMeasureMM)
    /// Last score segment (read only)
    Q_PROPERTY(apiv1::Segment * lastSegment READ lastSegment)                // TODO: make it function? Was property in 2.X, but firstSegment is a function...
    /// Number of lyrics items (syllables) in the score (read only).\n \since MuseScore 3.2
    Q_PROPERTY(int lyricCount READ lyricCount)
    /// Name of the score, without path leading to it and extension.\n \since MuseScore 3.2
    Q_PROPERTY(QString scoreName READ name WRITE setName)
    /// Number of measures (read only)
    Q_PROPERTY(int nmeasures READ nmeasures)
    /// Number of pages (read only)
    Q_PROPERTY(int npages READ npages)
    /// Number of staves (read only)
    Q_PROPERTY(int nstaves READ nstaves)
    /// Number of tracks (#nstaves * 4) (read only)
    Q_PROPERTY(int ntracks READ ntracks)
//      Q_PROPERTY(mu::engraving::PageFormat*                pageFormat        READ pageFormat     WRITE undoChangePageFormat)
    /// The list of parts
    Q_PROPERTY(QQmlListProperty<apiv1::Part> parts READ parts)
    /// Lyricist of score, as taken from the score properties.\n \since MuseScore 3.2
    Q_PROPERTY(QString lyricist READ lyricist)
//      Q_PROPERTY(QString                        subtitle          READ subtitle)
    /// Title of score, as taken from the score properties' workTitle (read only).\n \since MuseScore 3.2
    Q_PROPERTY(QString title READ title)
    /// MuseScore version the score has been last saved with (includes autosave) (read only)
    Q_PROPERTY(QString mscoreVersion READ mscoreVersion)
    /// MuseScore revision the score has been last saved with (includes autosave) (read only)
    Q_PROPERTY(QString mscoreRevision READ mscoreRevision)
    /// Current selections for the score. \since MuseScore 3.3
    Q_PROPERTY(apiv1::Selection * selection READ selection)
    /// Style settings for this score. \since MuseScore 3.5
    Q_PROPERTY(apiv1::MStyle * style READ style)
    /// Page numbering offset. The user-visible number of the given \p page is defined as
    /// \code
    /// page.pagenumber + 1 + score.pageNumberOffset
    /// \endcode
    /// \since MuseScore 3.5
    /// \see Page::pagenumber
    Q_PROPERTY(int pageNumberOffset READ pageNumberOffset WRITE setPageNumberOffset)

    /// The current layout mode, a PluginAPI::LayoutMode value.
    /// \since MuseScore 4.6
    Q_PROPERTY(int layoutMode READ layoutMode WRITE setLayoutMode)
    /// Whether vertical frames are visible in the current layout mode.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showVerticalFrames READ isShowVBox WRITE setShowVBox)
    /// Whether invisible elements are shown in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showInvisible READ isShowInvisible WRITE setShowInvisible)
    /// Whether formatting elements are displayed in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showUnprintable READ showUnprintable WRITE setShowUnprintable)
    /// Whether frames are displayed in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showFrames READ showFrames WRITE setShowFrames)
    /// Whether page borders are displayed in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showPageborders READ showPageborders WRITE setShowPageborders)
    /// Whether sound flags are displayed in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showSoundFlags READ showSoundFlags WRITE setShowSoundFlags)
    /// Whether corrupted measures are marked in the score.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool markIrregularMeasures READ showSoundFlags WRITE setMarkIrregularMeasures)
    /// Whether instrument names are displayed
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showInstrumentNames READ showInstrumentNames WRITE setShowInstrumentNames)
    /// List of staves in this score.
    /// \since MuseScore 3.6.3
    Q_PROPERTY(QQmlListProperty<apiv1::Staff> staves READ staves)
    /// List of pages in this score.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::Page> pages READ pages)
    /// List of systems in this score.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::System> systems READ systems)

    muse::Inject<mu::context::IGlobalContext> context = { this };

public:
    /// \cond MS_INTERNAL
    Score(mu::engraving::Score* s, Ownership o = Ownership::SCORE)
        : ScoreElement(s, o), muse::Injectable(s->iocContext()) {}

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
    apiv1::Selection* selection() { return selectionWrap(&score()->selection()); }
    apiv1::MStyle* style() { return styleWrap(&score()->style(), score()); }

    int pageNumberOffset() const { return score()->pageNumberOffset(); }
    void setPageNumberOffset(int offset) { score()->undoChangePageNumberOffset(offset); }

    bool isShowInvisible() { return score()->isShowInvisible(); }
    void setShowInvisible(bool v) { score()->setShowInvisible(v); }
    bool showUnprintable() { return score()->showUnprintable(); }
    void setShowUnprintable(bool v) { score()->setShowUnprintable(v); }
    bool showFrames() { return score()->showFrames(); }
    void setShowFrames(bool v) { score()->setShowFrames(v); }
    bool showPageborders() { return score()->showPageborders(); }
    void setShowPageborders(bool v) { score()->setShowPageborders(v); }
    bool showSoundFlags() { return score()->showSoundFlags(); }
    void setShowSoundFlags(bool v) { score()->setShowSoundFlags(v); }
    bool markIrregularMeasures() { return score()->markIrregularMeasures(); }
    void setMarkIrregularMeasures(bool v) { score()->setMarkIrregularMeasures(v); }
    bool showInstrumentNames() { return score()->showInstrumentNames(); }
    void setShowInstrumentNames(bool v) { score()->setShowInstrumentNames(v); }

    int layoutMode() { return int(score()->layoutMode()); }
    void setLayoutMode(int mode) { score()->setLayoutMode(mu::engraving::LayoutMode(mode)); }
    bool isShowVBox() { return score()->layoutOptions().isShowVBox; }
    void setShowVBox(bool show) { score()->setShowVBox(show); }

    /// \endcond

    /// muse::Returns as a string the metatag named \p tag
    Q_INVOKABLE QString metaTag(const QString& tag) const { return score()->metaTag(tag); }
    /// Sets the metatag named \p tag to \p val
    Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { score()->setMetaTag(tag, val); }

    /// Appends a part with the instrument defined by \p instrumentId
    /// to this score.
    /// \param instrumentId - ID of the instrument to be added, as listed in
    /// [`instruments.xml`](https://github.com/musescore/MuseScore/blob/3.x/share/instruments/instruments.xml)
    /// file.
    /// \since MuseScore 3.5
    Q_INVOKABLE void appendPart(const QString& instrumentId);
    /// Appends a part with the instrument defined by the given MusicXML ID
    /// to this score.
    /// \param instrumentMusicXmlId -
    /// [MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/)
    /// of the instrument to be added.
    /// \see \ref apiv1::Part::instrumentId, \ref apiv1::Instrument::instrumentId
    /// \since MuseScore 3.5
    Q_INVOKABLE void appendPartByMusicXmlId(const QString& instrumentMusicXmlId);

    /// Appends a number of measures to this score.
    Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
    Q_INVOKABLE void addText(const QString& type, const QString& text);
    /// Creates and returns a cursor to be used to navigate in the score
    Q_INVOKABLE apiv1::Cursor* newCursor();

    /// The first segment of a given type in the score.
    /// Before MuseScore 4.6, the type could not be specified.
    /// \param segmentType If not specified, defaults to all types.
    Q_INVOKABLE apiv1::Segment* firstSegment(int segmentType = int(mu::engraving::SegmentType::All));
    /// \cond MS_INTERNAL
    Segment* lastSegment();

    Measure* firstMeasure();
    Measure* firstMeasureMM();
    Measure* lastMeasure();
    Measure* lastMeasureMM();

    QString name() const;
    void setName(const QString& name);
    /// \endcond

    /// The measure at a given tick in the score.
    /// \param tick Tick to search for the measure
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::Measure* tick2measure(apiv1::FractionWrapper* tick);

    /// Looks for a segment of a given type at a given tick.
    /// Does not create a segment or modify the score.
    /// \param types Determines the types of segments to look for.
    /// \param tick Determines where to look for the segment
    /// \returns A segment with the given criteria, if such exists.
    /// \see \ref mu::plugins::api::Segment::segmentType
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::Segment* findSegmentAtTick(int types, apiv1::FractionWrapper* tick);

    /// Extracts all lyrics in the score and returns them in a single string.
    Q_INVOKABLE QString extractLyrics() { return score()->extractLyrics(); }

    /// \cond MS_INTERNAL
    int nmeasures() const { return static_cast<int>(score()->nmeasures()); }
    int npages() const { return static_cast<int>(score()->npages()); }
    int nstaves() const { return static_cast<int>(score()->nstaves()); }
    int ntracks() const { return static_cast<int>(score()->ntracks()); }
    /// \endcond

    /// For MuseScore 4 and for "dock" type plugins: to be used before score
    /// modifications to make them undoable, and to avoid corruptions or crashes.
    /// Starts an undoable command. Must be accompanied by
    /// a corresponding endCmd() call.
    /// \param qActionName - Optional action name that appears in Undo/Redo
    /// menus, palettes, and lists.
    Q_INVOKABLE void startCmd(const QString& qActionName = {});
    /// For MuseScore 4 and for "dock" type plugins: to be used after score
    /// modifications to make them undoable, and to avoid corruptions or crashes.
    /// Ends an undoable command.
    /// \param rollback If true, reverts all the changes
    /// made since the last startCmd() invocation.
    Q_INVOKABLE void endCmd(bool rollback = false);

    /// Create PlayEvents for all notes based on ornamentation.
    /// You need to call this if you are manipulating PlayEvent's
    /// so that all ornamentations are populated into Note's
    /// PlayEvent lists.
    /// \note PlayEvents don't have a playback effect in MuseScore 4.
    /// \since 3.3
    Q_INVOKABLE void createPlayEvents();

    /// \brief Force the score to layout itself.
    /// The score is laid out automatically at the end of a command,
    /// however this method can be called to layout mid-command.
    /// Layout the whole score with:
    /// \code
    /// curScore.doLayout(fraction(0, 1), fraction(-1, 1))
    /// \endcode
    /// \param startTick Fraction from which to start the layout
    /// \param endTick Fraction at which to end the layout
    /// \since MuseScore 4.6
    Q_INVOKABLE void doLayout(apiv1::FractionWrapper* startTick, apiv1::FractionWrapper* endTick);

    /// \brief Put an element in the user's view.
    /// \param element The element to put into view.
    /// \param staffIdx If provided, the specific staff to put into view.
    /// \since MuseScore 4.6
    Q_INVOKABLE void showElementInScore(apiv1::EngravingItem* element, int staffIdx = -1);

    /// Add or remove system locks to the current selection.
    /// \param interval Specifies after how many measures locks should be added.
    /// \param lock If \p true, adds locks, else removes them.
    /// \since MuseScore 4.6
    Q_INVOKABLE void addRemoveSystemLocks(int interval, bool lock) { score()->addRemoveSystemLocks(interval, lock); }

    /// Create a (locked) system from two \ref MeasureBase objects.
    /// \param first The first MeasureBase in the system.
    /// \param last The last MeasureBase in the system.
    /// \since MuseScore 4.6
    Q_INVOKABLE void makeIntoSystem(apiv1::MeasureBase* first, apiv1::MeasureBase* last);

    /// \cond MS_INTERNAL
    QString mscoreVersion() { return score()->mscoreVersion(); }
    QString mscoreRevision() { return QString::number(score()->mscoreRevision(), /* base */ 16); }

    QQmlListProperty<apiv1::Part> parts() { return wrapContainerProperty<apiv1::Part>(this, score()->parts()); }
    QQmlListProperty<apiv1::Excerpt> excerpts()
    {
        return wrapExcerptsContainerProperty<apiv1::Excerpt>(this, score()->masterScore()->excerpts());
    }

    QQmlListProperty<apiv1::Staff> staves();
    QQmlListProperty<apiv1::Page> pages();
    QQmlListProperty<apiv1::System> systems();

    static const mu::engraving::InstrumentTemplate* instrTemplateFromName(const QString& name);   // used by PluginAPI::newScore()
    /// \endcond

private:
    mu::notation::INotationPtr notation() const;
    mu::notation::INotationUndoStackPtr undoStack() const;
};
}
