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
#ifndef MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
#define MU_ENGRAVING_IENGRAVINGCONFIGURATION_H

#include "types/string.h"
#include "io/path.h"
#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "async/notification.h"
#include "engraving/types/types.h"

namespace muse::draw {
class Color;
}

namespace mu::engraving {
class IEngravingConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingConfiguration)
public:
    virtual ~IEngravingConfiguration() = default;

    virtual muse::io::path_t appDataPath() const = 0;

    virtual muse::io::path_t defaultStyleFilePath() const = 0;
    virtual void setDefaultStyleFilePath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> defaultStyleFilePathChanged() const = 0;

    virtual muse::io::path_t partStyleFilePath() const = 0;
    virtual void setPartStyleFilePath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> partStyleFilePathChanged() const = 0;

    virtual SizeF defaultPageSize() const = 0;

    virtual String iconsFontFamily() const = 0;

    virtual Color defaultColor() const = 0;
    virtual Color scoreInversionColor() const = 0;
    virtual Color lassoColor() const = 0;
    virtual Color warningColor() const = 0;
    virtual Color warningSelectedColor() const = 0;
    virtual Color criticalColor() const = 0;
    virtual Color criticalSelectedColor() const = 0;
    virtual Color thumbnailBackgroundColor() const = 0;
    virtual Color noteBackgroundColor() const = 0;
    virtual Color fontPrimaryColor() const = 0;
    virtual Color voiceColor(voice_idx_t voiceIdx) const = 0;

    virtual double guiScaling() const = 0;

    virtual Color selectionColor(voice_idx_t voiceIndex = 0, bool itemVisible = true, bool itemIsUnlinkedFromScore = false) const = 0;
    virtual void setSelectionColor(voice_idx_t voiceIndex, Color color) = 0;
    virtual muse::async::Channel<voice_idx_t, Color> selectionColorChanged() const = 0;

    virtual bool scoreInversionEnabled() const = 0;
    virtual void setScoreInversionEnabled(bool value) = 0;
    virtual muse::async::Notification scoreInversionChanged() const = 0;

    virtual bool dynamicsApplyToAllVoices() const = 0;
    virtual void setDynamicsApplyToAllVoices(bool v) = 0;
    virtual muse::async::Channel<bool> dynamicsApplyToAllVoicesChanged() const = 0;

    virtual Color formattingColor() const = 0;
    virtual muse::async::Channel<Color> formattingColorChanged() const = 0;

    virtual Color invisibleColor() const = 0;
    virtual muse::async::Channel<Color> invisibleColorChanged() const = 0;

    virtual Color unlinkedColor() const = 0;
    virtual muse::async::Channel<Color> unlinkedColorChanged() const = 0;

    virtual Color frameColor() const = 0;
    virtual muse::async::Channel<Color> frameColorChanged() const = 0;

    virtual Color scoreGreyColor() const = 0;

    virtual Color highlightSelectionColor(voice_idx_t voiceIndex = 0) const = 0;

    struct DebuggingOptions {
        bool showElementBoundingRects = false;
        bool colorElementShapes = false;
        bool showSegmentShapes = false;
        bool colorSegmentShapes = false;
        bool showSkylines = false;
        bool showSystemBoundingRects = false;
        bool showElementMasks = false;
        bool markCorruptedMeasures = true;

        bool anyEnabled() const
        {
            return showElementBoundingRects
                   || colorElementShapes
                   || showSegmentShapes
                   || colorSegmentShapes
                   || showSkylines
                   || showSystemBoundingRects
                   || showElementMasks
                   || markCorruptedMeasures
            ;
        }
    };

    virtual const DebuggingOptions& debuggingOptions() const = 0;
    virtual void setDebuggingOptions(const DebuggingOptions& options) = 0;
    virtual muse::async::Notification debuggingOptionsChanged() const = 0;

    virtual bool isAccessibleEnabled() const = 0;

    virtual bool doNotSaveEIDsForBackCompat() const = 0;
    virtual void setDoNotSaveEIDsForBackCompat(bool doNotSave) = 0;

    /// these configurations will be removed after solving https://github.com/musescore/MuseScore/issues/14294
    virtual bool guitarProImportExperimental() const = 0;
    virtual bool shouldAddParenthesisOnStandardStaff() const = 0;
    virtual bool negativeFretsAllowed() const = 0;
    virtual bool crossNoteHeadAlwaysBlack() const = 0;
    virtual void setGuitarProMultivoiceEnabled(bool multiVoice) = 0;
    virtual bool guitarProMultivoiceEnabled() const = 0;
    virtual bool minDistanceForPartialSkylineCalculated() const = 0;
    virtual bool specificSlursLayoutWorkaround() const = 0;
};
}

#endif // MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
