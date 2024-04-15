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

#ifndef MU_IMPORTEXPORT_CONTINIOUSELEMENTSBUILDER_H
#define MU_IMPORTEXPORT_CONTINIOUSELEMENTSBUILDER_H

#include "engraving/dom/line.h"

namespace mu::iex::guitarpro {
class ContiniousElementsBuilder
{
public:
    enum class ImportType {
        NONE,
        LET_RING,
        PALM_MUTE,
        WHAMMY_BAR,
        RASGUEADO,
        PICK_SCRAPE,

        /// harmonics
        HARMONIC_ARTIFICIAL,
        HARMONIC_PINCH,
        HARMONIC_TAP,
        HARMONIC_SEMI,
        HARMONIC_FEEDBACK,

        /// ottavas
        OTTAVA_MA15,
        OTTAVA_VA8,
        OTTAVA_VB8,
        OTTAVA_MB15,

        /// trill
        TRILL,

        /// hairpin
        HAIRPIN_CRESCENDO,
        HAIRPIN_DIMINUENDO,

        /// vibrato
        VIBRATO_LEFT_HAND_SLIGHT,
        VIBRATO_LEFT_HAND_WIDE,
        VIBRATO_W_TREM_BAR_SLIGHT,
        VIBRATO_W_TREM_BAR_WIDE
    };

    using sub_type_t = uint8_t;

    enum VibratoSubType : sub_type_t {
        LEFT_HAND_SLIGHT = 0,
        LEFT_HAND_WIDE,
        W_TREM_BAR_SLIGHT,
        W_TREM_BAR_WIDE,
    };

    enum HarmonicMarkSubType : sub_type_t {
        ARTIFICIAL = 0,
        PINCH,
        TAP,
        SEMI,
        FEEDBACK
    };

    ContiniousElementsBuilder(mu::engraving::Score* score);

    /**
     * Making the current element of continious type (octave, let ring, trill etc.. inherited from SLine) longer or starting a new one
     *
     * @param cr ChordRest to which element will be added
     * @param elements vector storing current continious elements for each existing track
     * @param muType type from MU
     * @param importType type of imported element
     * @param elemExists indicates if element exists in imported file on current beat
     * @param subType is used when elements of same MU type with different subtypes can overlap each other (ex, vibrato)
     */
    void buildContiniousElement(mu::engraving::ChordRest* cr, mu::engraving::ElementType muType, ImportType importType, bool elemExists,
                                sub_type_t subType = 0);
    void notifyUncompletedMeasure();
    void addElementsToScore();

private:

    using track_idx_t = mu::engraving::track_idx_t;

    enum class ContiniousElementState {
        UNDEFINED = -1,
        CHORD_NO_ELEMENT,      // there is no imported type of element
        CONTINUE_CURRENT_LINE, // element exists on current beat and was before, line will be continued
        REST_BREAK,            // element should be broken in rests, new element starts later
        REST_CONTINUE,         // element shouln't be broken in rests, end of element will be found later
        CREATE_NEW_ITEM,       // new element should be created on this beat
        ELEMENT_ON_REST,       // element was imported for Rest
    };

    void setupAddedElement(track_idx_t trackIdx, ImportType importType);
    ContiniousElementState calculateState(bool isRest, bool elemExists, bool splitByRests, bool importTypeChanged) const;

    struct ContiniousElement {
        std::vector<mu::engraving::SLine*> elements;
        bool endedOnRest = false;
    };

    std::unordered_map<ImportType, std::vector<mu::engraving::SLine*> > m_elementsByType;

    std::unordered_map<mu::engraving::ElementType,
                       std::unordered_map<track_idx_t, std::unordered_map<sub_type_t, ImportType> > > m_lastImportTypes;
    std::unordered_map<track_idx_t, std::unordered_map<ImportType, ContiniousElement> > m_elementsToAddToScore;
    std::vector<mu::engraving::SLine*> m_orderedAddedElements;

    mu::engraving::Score* m_score = nullptr;
};
} // mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_CONTINIOUSELEMENTSBUILDER_H
