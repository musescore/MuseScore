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
class Lyrics;
class Spanner;

extern Selection* selectionWrap(mu::engraving::Selection* select);

//---------------------------------------------------------
//   Score
//---------------------------------------------------------
/** APIDOC
 * Class representing a score.
 * We can get the current score by calling the `Engraving.curScore`
 * @class Score
 * @memberof Engraving
 * @hideconstructor
*/
class Score : public apiv1::ScoreElement, public muse::Injectable
{
    Q_OBJECT

    /** APIDOC
     * Name of the score, without path leading to it and extension.
     * @q_property {String}
     */
    Q_PROPERTY(QString scoreName READ name WRITE setName)

    /** APIDOC
     * Title of score, as taken from the score properties `workTitle`
     * @readonly
     * @q_property {String}
     */
    Q_PROPERTY(QString title READ title)

    /** APIDOC
     * Composer of the score, as taken from the score properties
     * @readonly
     * @q_property {String}
     */
    Q_PROPERTY(QString composer READ composer)

    /** APIDOC
     * Lyricist of score, as taken from the score properties.
     * @readonly
     * @q_property {String}
     */
    Q_PROPERTY(QString lyricist READ lyricist)

    /** APIDOC
     * Duration of score in seconds
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int duration READ duration)

    /** APIDOC
     * MuseScore version the score has been last saved with (includes autosave)
     * @readonly
     * @q_property {String}
     */
    Q_PROPERTY(QString mscoreVersion READ mscoreVersion)

    /** APIDOC
     * MuseScore revision the score has been last saved with (includes autosave)
     * @readonly
     * @q_property {String}
     */
    Q_PROPERTY(QString mscoreRevision READ mscoreRevision)

    /** APIDOC
     * Style settings for this score.
     * @readonly
     * @q_property {Engraving.Style}
     */
    Q_PROPERTY(apiv1::MStyle * style READ style)

    /** APIDOC
     * Key signature at the start of the score, in number of accidentals,
     * negative for flats, positive for sharps
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int keysig READ keysig)

    /** APIDOC
     * Number of pages
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int npages READ npages)

    /** APIDOC
     * List of pages in this score.
     * @readonly
     * @q_property {Engraving.Page[]}
     * @since 4.6
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Page> pages READ pages)

    /** APIDOC
     * Page numbering offset. The user-visible number of the given page is defined as
     * ```
     * page.pagenumber + 1 + score.pageNumberOffset
     * ```
     * @q_property {Number}
     */
    Q_PROPERTY(int pageNumberOffset READ pageNumberOffset WRITE setPageNumberOffset)

    /** APIDOC
     * The list of parts
     * @readonly
     * @q_property {Engraving.Part[]}
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Part> parts READ parts)

    /** APIDOC
     * Number of staves
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int nstaves READ nstaves)

    /** APIDOC
     * List of staves in this score.
     * @readonly
     * @q_property {Engraving.Staff[]}
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Staff> staves READ staves)

    /** APIDOC
     * Number of tracks
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int ntracks READ ntracks)

    /** APIDOC
     * List of systems in this score.
     * @readonly
     * @q_property {Engraving.System[]}
     * @since 4.6
     */
    Q_PROPERTY(QQmlListProperty<apiv1::System> systems READ systems)

    /** APIDOC
     * List of spanners (hairpins, slurs, etc.) in this score.
     * @readonly
     * @q_property {Engraving.Spanner[]}
     * @since 4.7
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Spanner> spanners READ spanners)

    /** APIDOC
     * Whether score has harmonies (chord symbols)
     * @readonly
     * @q_property {Boolean}
     */
    Q_PROPERTY(bool hasHarmonies READ hasHarmonies)

    /** APIDOC
     * Number of harmony items (chord symbols) in the score
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int harmonyCount READ harmonyCount)

    /** APIDOC
     * Whether score has lyrics
     * @readonly
     * @q_property {Boolean}
     */
    Q_PROPERTY(bool hasLyrics READ hasLyrics)

    /** APIDOC
     * Number of lyrics items (syllables) in the score
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int lyricCount READ lyricCount)

    /** APIDOC
     * List of lyrics in this score.
     * @readonly
     * @q_property {Engraving.Lyrics[]}
     * @since 4.7
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Lyrics> lyrics READ lyrics)

    /** APIDOC
     * Number of measures
     * @readonly
     * @q_property {Number}
     */
    Q_PROPERTY(int nmeasures READ nmeasures)

    /** APIDOC
     * First measure of the score
     * @readonly
     * @q_property {Engraving.Measure}
     */
    Q_PROPERTY(apiv1::Measure * firstMeasure READ firstMeasure)

    /** APIDOC
     * First multimeasure rest measure of the score
     * @readonly
     * @q_property {Engraving.Measure}
     * @see Engraving.Measure.nextMeasureMM
     */
    Q_PROPERTY(apiv1::Measure * firstMeasureMM READ firstMeasureMM)

    /** APIDOC
     * Last measure of the score
     * @readonly
     * @q_property {Engraving.Measure}
     */
    Q_PROPERTY(apiv1::Measure * lastMeasure READ lastMeasure)

    /** APIDOC
     * Last multimeasure rest measure of the score
     * @readonly
     * @q_property {Engraving.Measure}
     * @see Engraving.Measure.prevMeasureMM
     */
    Q_PROPERTY(apiv1::Measure * lastMeasureMM READ lastMeasureMM)

    /** APIDOC
     * Last score segment
     * @readonly
     * @q_property {Engraving.Segment}
     */
    Q_PROPERTY(apiv1::Segment * lastSegment READ lastSegment)                // TODO: make it function? Was property in 2.X, but firstSegment is a function...

    /** APIDOC
     * The current layout mode, a PluginAPI::LayoutMode value.
     * @q_property {Number}
     * @since 4.6
     */
    Q_PROPERTY(int layoutMode READ layoutMode WRITE setLayoutMode)

    /** APIDOC
     * Whether vertical frames are visible in the current layout mode.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showVerticalFrames READ isShowVBox WRITE setShowVBox)

    /** APIDOC
     * Whether invisible elements are shown in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showInvisible READ isShowInvisible WRITE setShowInvisible)

    /** APIDOC
     * Whether formatting elements are displayed in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showUnprintable READ showUnprintable WRITE setShowUnprintable)

    /** APIDOC
     * Whether frames are displayed in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showFrames READ showFrames WRITE setShowFrames)

    /** APIDOC
     * Whether page borders are displayed in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showPageborders READ showPageborders WRITE setShowPageborders)

    /** APIDOC
     * Whether sound flags are displayed in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showSoundFlags READ showSoundFlags WRITE setShowSoundFlags)

    /** APIDOC
     * Whether corrupted measures are marked in the score.
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool markIrregularMeasures READ showSoundFlags WRITE setMarkIrregularMeasures)

    /** APIDOC
     * Whether instrument names are displayed
     * @q_property {Boolean}
     * @since 4.6
     */
    Q_PROPERTY(bool showInstrumentNames READ showInstrumentNames WRITE setShowInstrumentNames)

    /** APIDOC
     * Current selections for the score.
     * @readonly
     * @q_property {Engraving.Selection}
     */
    Q_PROPERTY(apiv1::Selection * selection READ selection)

    /** APIDOC
     * List of the excerpts (linked parts)
     * @readonly
     * @q_property {Engraving.Excerpt[]}
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Excerpt> excerpts READ excerpts)

    muse::Inject<mu::context::IGlobalContext> context = { this };

public:
    Score(mu::engraving::Score* s, Ownership o = Ownership::SCORE)
        : ScoreElement(s, o), muse::Injectable(s->iocContext()) {}

    QString name() const;
    void setName(const QString& name);
    QString title() const { return score()->metaTag(u"workTitle"); }
    QString composer() const { return score()->metaTag(u"composer"); }
    QString lyricist() const { return score()->metaTag(u"lyricist"); }   // not the meanwhile obsolete "poet"
    /** APIDOC
    * Get the value of a meta tag by name
    * @method
    * @param {String} tag Tag
    * @return {String} Tag value
    */
    Q_INVOKABLE QString metaTag(const QString& tag) const { return score()->metaTag(tag); }
    /** APIDOC
    * Set a value of a meta tag by name
    * @method
    * @param {String} tag Tag
    * @param {String} value Value
    */
    Q_INVOKABLE void setMetaTag(const QString& tag, const QString& val) { score()->setMetaTag(tag, val); }

    int duration() const { return score()->duration(); }
    QString mscoreVersion() const { return score()->mscoreVersion(); }
    QString mscoreRevision() const { return QString::number(score()->mscoreRevision(), /* base */ 16); }
    apiv1::MStyle* style() { return styleWrap(&score()->style(), score()); }

    int keysig() const { return score()->keysig(); }

    int npages() const { return static_cast<int>(score()->npages()); }
    QQmlListProperty<apiv1::Page> pages() const;
    int pageNumberOffset() const { return score()->pageNumberOffset(); }
    void setPageNumberOffset(int offset) { score()->undoChangePageNumberOffset(offset); }

    QQmlListProperty<apiv1::Part> parts() const;
    /** APIDOC
    * Appends a part with the instrument defined by `instrumentId` to this score.
    * @method
    * @param {String} instrumentId ID of the instrument to be added,
    * as listed in {@link https://github.com/musescore/MuseScore/blob/master/share/instruments/instruments.xml|instruments.xml}
    */
    Q_INVOKABLE void appendPart(const QString& instrumentId);
    /** APIDOC
    * Appends a part with the instrument defined by the given MusicXML ID to this score.
    * @method
    * @param {String} instrumentMusicXmlId {@link https://www.musicxml.com/for-developers/standard-sounds/|MusicXML Sound ID}
    * of the instrument to be added.
    */
    Q_INVOKABLE void appendPartByMusicXmlId(const QString& instrumentMusicXmlId);

    int nstaves() const { return static_cast<int>(score()->nstaves()); }
    QQmlListProperty<apiv1::Staff> staves() const;

    int ntracks() const { return static_cast<int>(score()->ntracks()); }

    QQmlListProperty<apiv1::System> systems() const;
    /** APIDOC
    * Add or remove system locks to the current selection.
    * @method
    * @param {Number} interval Specifies after how many measures locks should be added.
    * @param {boolean} lock If `true`, adds locks, else removes them.
    * @since 4.6
    */
    Q_INVOKABLE void addRemoveSystemLocks(int interval, bool lock);

    /** APIDOC
    * Create a (locked) system from two {@link Engraving.Measure} objects.
    * @method
    * @param {Engraving.Measure} first The first `Measure` in the system.
    * @param {Engraving.Measure} last The last `Measure` in the system.
    * @since 4.6
    */
    Q_INVOKABLE void makeIntoSystem(apiv1::MeasureBase* first, apiv1::MeasureBase* last);

    QQmlListProperty<apiv1::Spanner> spanners();

    int harmonyCount() const { return score()->harmonyCount(); }
    bool hasHarmonies() const { return score()->hasHarmonies(); }

    bool hasLyrics() const;
    int lyricCount() const;
    QQmlListProperty<apiv1::Lyrics> lyrics() const;
    /** APIDOC
    * Extracts all lyrics in the score and returns them in a single string.
    * @method
    * @return {String} lyrics string
    */
    Q_INVOKABLE QString extractLyrics() const;

    int nmeasures() const { return static_cast<int>(score()->nmeasures()); }
    Measure* firstMeasure();
    Measure* firstMeasureMM();
    Measure* lastMeasure();
    Measure* lastMeasureMM();
    /** APIDOC
    * Appends a number of measures to this score.
    * @method
    * @param {Number} n number of measures
    */
    Q_INVOKABLE void appendMeasures(int n) { score()->appendMeasures(n); }
    /** APIDOC
     * The measure at a given tick in the score.
     * @method
     * @param {Engraving.Fraction} tick Tick to search for the measure
     * @since 4.6
    */
    Q_INVOKABLE apiv1::Measure* tick2measure(apiv1::Fraction* tick);

    /** APIDOC
    * The first segment of a given type in the score.
    * Before MuseScore 4.6, the type could not be specified.
    * @method
    * @param {Number} segmentType If not specified, defaults to all types.
    * @return {Engraving.Segment} Segment object
    */
    Q_INVOKABLE apiv1::Segment* firstSegment(int segmentType = int(mu::engraving::SegmentType::All));

    /** APIDOC
    * Looks for a segment of a given type at a given tick.
    * Does not create a segment or modify the score.
    * @method
    * @param {Number} types Determines the types of segments to look for.
    * @param {Engraving.Fraction} tick Determines where to look for the segment
    * @return {Engraving.Segment} Segment object
    * @since 4.6
    */
    Q_INVOKABLE apiv1::Segment* findSegmentAtTick(int types, apiv1::Fraction* tick);

    /** APIDOC
    * Add the text
    * @method
    * @param {Engraving.TextStyleType} type Stype type of text
    * @param {String} text Value of text
    */
    Q_INVOKABLE void addText(const QString& type, const QString& text);

    int layoutMode() { return int(score()->layoutMode()); }
    void setLayoutMode(int mode) { score()->setLayoutMode(mu::engraving::LayoutMode(mode)); }
    bool isShowVBox() const { return score()->layoutOptions().isShowVBox; }
    void setShowVBox(bool show) { score()->setShowVBox(show); }
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

    /** APIDOC
     * Force the score to layout itself.
     * The score is laid out automatically at the end of a command,
     * however this method can be called to layout mid-command.
     * Layout the whole score with:
     * ```
     * curScore.doLayout(fraction(0, 1), fraction(-1, 1))
     * ```
     * @method
     * @param {Engraving.Fraction} startTick Fraction from which to start the layout
     * @param {Engraving.Fraction} endTick Fraction at which to end the layout
     * @since 4.6
    */
    Q_INVOKABLE void doLayout(apiv1::Fraction* startTick, apiv1::Fraction* endTick);

    /** APIDOC
     * Creates and returns a cursor to be used to navigate in the score
     * @method
     * @returns {Cursor} cursor
    */
    Q_INVOKABLE apiv1::Cursor* newCursor();

    /** APIDOC
     * For MuseScore 4 and for "dock" type plugins: to be used before score
     * modifications to make them undoable, and to avoid corruptions or crashes.
     * Starts an undoable command. Must be accompanied by a corresponding `endCmd()` call.
     * @method
     * @param {String} qActionName Optional action name that appears in Undo/Redo menus, palettes, and lists.
    */
    Q_INVOKABLE void startCmd(const QString& qActionName = {});

    /** APIDOC
     * For MuseScore 4 and for "dock" type plugins: to be used after score
     * modifications to make them undoable, and to avoid corruptions or crashes.
     * Ends an undoable command.
     * @method
     * @param {Boolean} rollback If true, reverts all the changes made since the last `startCmd()` invocation.
    */
    Q_INVOKABLE void endCmd(bool rollback = false);

    apiv1::Selection* selection() { return selectionWrap(&score()->selection()); }

    QQmlListProperty<apiv1::Excerpt> excerpts() const;

    /** APIDOC
     * Put an item in the user's view.
     * @method
     * @param {Engraving.EngravingItem} item The item to put into view.
     * @param {Number} staffIdx If provided, the specific staff to put into view.
     * @since 4.6
    */
    Q_INVOKABLE void showElementInScore(apiv1::EngravingItem* element, int staffIdx = -1);

    /** APIDOC
     * Create PlayEvents for all notes based on ornamentation.
     * You need to call this if you are manipulating PlayEvent's
     * so that all ornamentations are populated into Note's PlayEvent lists.
     * @method
     * @deprecated PlayEvents don't have a playback effect in MuseScore 4.
    */
    Q_INVOKABLE void createPlayEvents();

    static const mu::engraving::InstrumentTemplate* instrTemplateFromName(const QString& name);   // used by PluginAPI::newScore()

    mu::engraving::Score* score() { return toScore(e); }
    const mu::engraving::Score* score() const { return toScore(e); }
private:

    Segment* lastSegment();

    mu::notation::INotationPtr notation() const;
    mu::notation::INotationUndoStackPtr undoStack() const;
};
}
