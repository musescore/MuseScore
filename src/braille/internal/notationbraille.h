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

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "ui/iuiconfiguration.h"
#include "engraving/iengravingconfiguration.h"

#include "inotationbraille.h"
#include "notation/notationtypes.h"

#include "async/asyncable.h"
#include "async/notification.h"

#include "braille/internal/braille.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"
#include "ibrailleconfiguration.h"

namespace mu::engraving {
class Score;
class Selection;

class NotationBraille : public mu::braille::INotationBraille, public async::Asyncable
{
    INJECT(framework::IGlobalConfiguration, globalConfiguration)
    INJECT(context::IGlobalContext, globalContext)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(braille::IBrailleConfiguration, brailleConfiguration)

public:
    void init();
    void doBraille(bool force = false);

    ValCh<std::string> brailleInfo() const override;
    ValCh<int> cursorPosition() const override;
    ValCh<int> currentItemPositionStart() const override;
    ValCh<int> currentItemPositionEnd() const override;
    ValCh<std::string> shortcut() const override;
    ValCh<bool> enabled() const override;

    void setEnabled(bool enabled) override;

    void setCursorPosition(const int pos) override;
    void setCurrentItemPosition(const int, const int) override;
    void setShortcut(const QString&) override;

    notation::INotationPtr notation();
    notation::INotationInteractionPtr interaction();

    BrailleEngravingItems* brailleEngravingItems();
    QString getBrailleStr();

private:
    Score* score();
    Selection* selection();

    Measure* current_measure = nullptr;

    void setBrailleInfo(const QString& info);
    void setCurrentShortcut(const QString& sequence);

    void updateTableForLyricsFromPreferences();
    io::path_t tablesDefaultDirPath() const;

    ValCh<std::string> m_brailleInfo;
    ValCh<int> m_cursorPosition;
    ValCh<int> m_currentItemPositionStart;
    ValCh<int> m_currentItemPositionEnd;
    ValCh<std::string> m_shortcut;
    ValCh<bool> m_enabled;

    BrailleEngravingItems m_bei;
    async::Notification m_selectionChanged;
};
}

#endif // MU_BRAILLE_NOTATIONBRAILLE_H
