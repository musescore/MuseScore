/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/engravingerrors.h"
#include "engraving/dom/masterscore.h"

#include "meiimporter.h"
#include "meiconverter.h"

using namespace muse;
using namespace mu::iex::mei;
using namespace mu::engraving;

Ret MeiReader::read(MasterScore* score, const muse::io::path_t& path, const Options& options)
{
    Err result = this->import(score, path, options);
    if (result == Err::NoError) {
        return muse::make_ok();
    }

    if (result == Err::FileCorrupted) {
        return make_ret(result, Convert::logs.join(u"\n"));
    }

    return make_ret(result, path);
}

Err MeiReader::import(MasterScore* score, const muse::io::path_t& path, const Options& options)
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

    //! NOTE The user decides whether to load the file anyway, then it is loaded again in the force mode
    if (!forceMode && !MScore::noGui && hasWarnings) {
        return Err::FileCorrupted;
    }

    return Err::NoError;
}
