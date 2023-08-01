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

#include "meireader.h"

#include "iinteractive.h"
#include "meiimporter.h"

#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"
#include "translation.h"

using namespace mu::iex::mei;
using namespace mu::engraving;

mu::Ret MeiReader::read(MasterScore* score, const io::path_t& path, const Options& options)
{
    Err result = this->import(score, path, options);
    return (result == Err::NoError) ? make_ok() : make_ret(result, path);
}

Err MeiReader::import(MasterScore* score, const io::path_t& path, const Options& options)
{
    if (!fileSystem()->exists(path)) {
        return Err::FileNotFound;
    }

    MeiImporter importer(score);
    if (!importer.read(path)) {
        return Err::FileCriticallyCorrupted;
    }

    bool forceMode = false;
    if (options.count(INotationReader::OptionKey::ForceMode)) {
        Val val = options.at(INotationReader::OptionKey::ForceMode);
        forceMode = (!val.isNull() && val.toBool());
    }

    bool hasWarnings = (Convert::logs.size() > 0);

    if (!forceMode && !MScore::noGui && hasWarnings) {
        const String text
            = qtrc("iex_mei", "%n problem(s) occured and the import may be incomplete.", nullptr, static_cast<int>(Convert::logs.size()));
        if (!this->askToLoadDespiteWarnings(text, Convert::logs.join(u"\n"))) {
            return Err::FileBadFormat;
        }
    }

    return Err::NoError;
}

/**
 * Show a dialog displaying the MEI import problem(s) and ask whether to load or cancel.
 */
bool MeiReader::askToLoadDespiteWarnings(const String& text, const String& detailedText)
{
    using framework::IInteractive;
    IInteractive::Button btn = interactive()->warning(
        text.toStdString(), trc("iex_mei", "Do you want to try to load this MEI file anyway?"), detailedText.toStdString(), {
        interactive()->buttonData(IInteractive::Button::Cancel),
        interactive()->buttonData(IInteractive::Button::Yes)
    }, (int)IInteractive::Button::Yes).standardButton();

    return btn == IInteractive::Button::Yes;
}
