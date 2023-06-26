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

/**
 Show a dialog displaying the MEI import error(s).
 */

static int meiImportErrorDialog(QString text, QString detailedText)
{
    QMessageBox errorDialog;
    errorDialog.setIcon(QMessageBox::Warning);
    errorDialog.setTextFormat(Qt::AutoText);
    errorDialog.setText(text);
    errorDialog.setInformativeText(mu::qtrc("iex_mei", "Do you want to try to load this MEI file anyway?"));
    errorDialog.setDetailedText(detailedText);
    errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    errorDialog.setDefaultButton(QMessageBox::No);
    return errorDialog.exec();
}


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
