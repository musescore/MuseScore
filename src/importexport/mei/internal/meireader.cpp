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

#include <QFileInfo>
#include <QMessageBox>

#include "meireader.h"

#include "meiimporter.h"

#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::mei;
using namespace mu::engraving;

//---------------------------------------------------------
//   MeiImportErrorDialog
//---------------------------------------------------------

mu::Ret MeiReader::read(MasterScore* score, const io::path_t& path, const Options& options)
{
    if (!QFileInfo::exists(path.toQString())) {
        return make_ret(Err::FileNotFound, path);
    }

    MeiImporter importer(score);
    if (!importer.read(path.toQString())) {
        return make_ret(Err::FileCriticallyCorrupted, path);
    }

    bool forceMode = false;
    if (options.count(INotationReader::OptionKey::ForceMode)) {
        Val val = options.at(INotationReader::OptionKey::ForceMode);
        forceMode = (!val.isNull() && val.toBool());
    }

    // Unused for now
    UNUSED(forceMode);

    return make_ok();
}

/**
 Show a dialog displaying the MEI import problem(s) and ask whether to load or cancel.
 */
bool MeiReader::askToLoadDespiteWarnings(QString text, QString detailedText)
{
    IInteractive::Button btn = interactive()->warning(
        text.toStdString(), "Do you want to try to load this MEI file anyway?", detailedText.toStdString(), {
        interactive()->buttonData(IInteractive::Button::Cancel),
        interactive()->buttonData(IInteractive::Button::Yes)
    }, (int)IInteractive::Button::Yes).standardButton();

    return btn == IInteractive::Button::Yes;
}
