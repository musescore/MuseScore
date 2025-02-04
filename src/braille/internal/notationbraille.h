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

#ifndef MU_BRAILLE_NOTATIONBRAILLE_H
#define MU_BRAILLE_NOTATIONBRAILLE_H

#include "async/asyncable.h"
#include "async/notification.h"
#include "context/iglobalcontext.h"
#include "global/iglobalconfiguration.h"
#include "ibrailleconfiguration.h"
#include "inotationbraille.h"
#include "modularity/ioc.h"
#include "notation/notationtypes.h"
#include "playback/iplaybackcontroller.h"
#include "actions/iactionsdispatcher.h"

#include "braille.h"
#include "brailleinput.h"

namespace mu::engraving {
class Score;
class Selection;

class NotationBraille : public mu::braille::INotationBraille, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<braille::IBrailleConfiguration> brailleConfiguration = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    NotationBraille(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

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

    muse::ValCh<std::string> brailleInfo() const override;
    muse::ValCh<int> cursorPosition() const override;
    muse::ValCh<int> currentItemPositionStart() const override;
    muse::ValCh<int> currentItemPositionEnd() const override;
    muse::ValCh<std::string> keys() const override;
    muse::ValCh<bool> enabled() const override;
    muse::ValCh<braille::BrailleIntervalDirection> intervalDirection() const override;
    muse::ValCh<int> mode() const override;
    muse::ValCh<std::string> cursorColor() const override;

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
    muse::io::path_t tablesDefaultDirPath() const;

    IntervalDirection currentIntervalDirection();

    Measure* current_measure = nullptr;
    EngravingItem* current_engraving_item = nullptr;
    BrailleEngravingItem* current_bei = nullptr;
    BrailleEngravingItemList m_beil;
    BrailleInputState m_braille_input;

    muse::ValCh<std::string> m_brailleInfo;
    muse::ValCh<int> m_cursorPosition;
    muse::ValCh<int> m_currentItemPositionStart;
    muse::ValCh<int> m_currentItemPositionEnd;
    muse::ValCh<std::string> m_keys;
    muse::ValCh<bool> m_enabled;
    muse::ValCh<int> m_mode;
    muse::ValCh<braille::BrailleIntervalDirection> m_intervalDirection;
    muse::ValCh<std::string> m_cursorColor;

    muse::async::Notification m_selectionChanged;
};
}

#endif // MU_BRAILLE_NOTATIONBRAILLE_H
