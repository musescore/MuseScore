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
#ifndef MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
#define MU_ENGRAVING_IENGRAVINGCONFIGURATION_H

#include "types/string.h"
#include "io/path.h"
#include "modularity/imoduleexport.h"
#include "async/channel.h"
#include "async/notification.h"
#include "engraving/types/types.h"

namespace mu::draw {
class Color;
}

namespace mu::engraving {
class IEngravingConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingConfiguration)
public:
    virtual ~IEngravingConfiguration() = default;

    virtual io::path_t appDataPath() const = 0;

    virtual io::path_t defaultStyleFilePath() const = 0;
    virtual void setDefaultStyleFilePath(const io::path_t& path) = 0;

    virtual io::path_t partStyleFilePath() const = 0;
    virtual void setPartStyleFilePath(const io::path_t& path) = 0;

    virtual SizeF defaultPageSize() const = 0;

    virtual String iconsFontFamily() const = 0;

    virtual draw::Color defaultColor() const = 0;
    virtual draw::Color scoreInversionColor() const = 0;
    virtual draw::Color invisibleColor() const = 0;
    virtual draw::Color lassoColor() const = 0;
    virtual draw::Color warningColor() const = 0;
    virtual draw::Color warningSelectedColor() const = 0;
    virtual draw::Color criticalColor() const = 0;
    virtual draw::Color criticalSelectedColor() const = 0;
    virtual draw::Color formattingMarksColor() const = 0;
    virtual draw::Color thumbnailBackgroundColor() const = 0;
    virtual draw::Color noteBackgroundColor() const = 0;

    virtual double guiScaling() const = 0;

    virtual draw::Color selectionColor(voice_idx_t voiceIndex = 0, bool itemVisible = true) const = 0;
    virtual void setSelectionColor(voice_idx_t voiceIndex, draw::Color color) = 0;
    virtual async::Channel<voice_idx_t, draw::Color> selectionColorChanged() const = 0;

    virtual bool scoreInversionEnabled() const = 0;
    virtual void setScoreInversionEnabled(bool value) = 0;
    virtual async::Notification scoreInversionChanged() const = 0;

    virtual draw::Color highlightSelectionColor(voice_idx_t voiceIndex = 0) const = 0;

    struct DebuggingOptions {
        bool showElementBoundingRects = false;
        bool colorElementShapes = false;
        bool showSegmentShapes = false;
        bool colorSegmentShapes = false;
        bool showSkylines = false;
        bool showSystemBoundingRects = false;
        bool showCorruptedMeasures = true;
    };

    virtual const DebuggingOptions& debuggingOptions() const = 0;
    virtual void setDebuggingOptions(const DebuggingOptions& options) = 0;
    virtual async::Notification debuggingOptionsChanged() const = 0;

    virtual bool isAccessibleEnabled() const = 0;

    /// these configurations will be removed after solving https://github.com/musescore/MuseScore/issues/14294
    virtual bool guitarProImportExperimental() const = 0;
    virtual bool negativeFretsAllowed() const = 0;
    virtual bool tablatureParenthesesZIndexWorkaround() const = 0;
    virtual bool crossNoteHeadAlwaysBlack() const = 0;
    virtual bool enableExperimentalFretCircle() const = 0;
    virtual void setGuitarProMultivoiceEnabled(bool multiVoice) = 0;
    virtual bool guitarProMultivoiceEnabled() const = 0;
};
}

#endif // MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
