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

#ifndef MU_BRAILLE_NOTATIONBRAILLE_H
#define MU_BRAILLE_NOTATIONBRAILLE_H

#include "async/asyncable.h"
#include "async/notification.h"
#include "context/iglobalcontext.h"
#include "global/iglobalconfiguration.h"
#include "ibrailleconfiguration.h"
#include "inotationbraille.h"
#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "notation/notationtypes.h"
#include "playback/iplaybackcontroller.h"

#include "braille.h"
#include "brailleinput.h"

namespace mu::engraving {
class Score;
class Selection;

class NotationBraille : public mu::braille::INotationBraille, public async::Asyncable
{
    INJECT(framework::IGlobalConfiguration, globalConfiguration)
    INJECT(context::IGlobalContext, globalContext)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(braille::IBrailleConfiguration, brailleConfiguration)
    INJECT(playback::IPlaybackController, playbackController)

public:
    void init();
    void doBraille(bool force = false);

    bool addNote();
    bool setVoice(bool new_voice = false);
    bool addSlurStart();
    bool addSlurEnd();
    bool addTie();
    bool addSlur();
    bool addLongSlur();
    bool addTuplet(const mu::notation::TupletOptions& options);
    bool incDuration();
    bool decDuration();
    bool setArticulation();
    void setInputNoteDuration(notation::Duration d);
    void setTupletDuration(int tuplet, notation::Duration d);

    ValCh<std::string> brailleInfo() const override;
    ValCh<int> cursorPosition() const override;
    ValCh<int> currentItemPositionStart() const override;
    ValCh<int> currentItemPositionEnd() const override;
    ValCh<std::string> keys() const override;
    ValCh<bool> enabled() const override;
    ValCh<braille::BrailleIntervalDirection> intervalDirection() const override;
    ValCh<int> mode() const override;
    ValCh<std::string> cursorColor() const override;

    void setEnabled(const bool enabled) override;
    void setIntervalDirection(const braille::BrailleIntervalDirection direction) override;

    void setCursorPosition(const int pos) override;
    void setCurrentItemPosition(const int, const int) override;
    void setKeys(const QString&) override;

    void setMode(const braille::BrailleMode) override;
    void toggleMode() override;
    bool isNavigationMode() override;
    bool isBrailleInputMode() override;

    void setCursorColor(const QString color) override;

    EngravingItem* currentEngravingItem();
    Measure* currentMeasure();

    notation::INotationPtr notation();
    notation::INotationInteractionPtr interaction();

    BrailleEngravingItemList* brailleEngravingItemList();
    QString getBrailleStr();

    BrailleInputState* brailleInput();

private:
    Score* score();
    Selection* selection();

    void setBrailleInfo(const QString& info);
    void setCurrentEngravingItem(EngravingItem* el, bool select);

    void updateTableForLyricsFromPreferences();
    io::path_t tablesDefaultDirPath() const;

    IntervalDirection currentIntervalDirection();

    Measure* current_measure = nullptr;
    EngravingItem* current_engraving_item = nullptr;
    BrailleEngravingItem* current_bei = nullptr;
    BrailleEngravingItemList m_beil;
    BrailleInputState m_braille_input;

    ValCh<std::string> m_brailleInfo;
    ValCh<int> m_cursorPosition;
    ValCh<int> m_currentItemPositionStart;
    ValCh<int> m_currentItemPositionEnd;
    ValCh<std::string> m_keys;
    ValCh<bool> m_enabled;
    ValCh<int> m_mode;
    ValCh<braille::BrailleIntervalDirection> m_intervalDirection;
    ValCh<std::string> m_cursorColor;

    async::Notification m_selectionChanged;
};
}

#endif // MU_BRAILLE_NOTATIONBRAILLE_H
