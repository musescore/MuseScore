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
#include "projectmigrator.h"

#include <QVersionNumber>

#include "io/path.h"
#include "mdlmigrator.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/masterscore.h"
#include "engraving/editing/editscoreproperties.h"
#include "engraving/editing/editstyle.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/rw/compat/readstyle.h"

#include "io/file.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::engraving;
using namespace mu::engraving::compat;
using namespace muse;

static const io::path_t LELAND_STYLE_PATH(":/engraving/styles/migration-306-style-Leland.mss");
static const io::path_t EDWIN_STYLE_PATH(":/engraving/styles/migration-306-style-Edwin.mss");

MigrationType ProjectMigrator::migrationType(int mscVersion) const
{
    if (mscVersion < 302) {
        return MigrationType::Pre_3_6;
    }

    if (mscVersion < 400) {
        return MigrationType::Ver_3_6;
    }

    return MigrationType::Unknown;
}

Ret ProjectMigrator::migrateEngravingProjectIfNeed(engraving::EngravingProjectPtr project)
{
    if (project->mscVersion() >= 400) {
        return true;
    }
    //! NOTE If the migration is not done, then the default style for the score is determined by the version.
    //! When migrating, the version becomes the current one, so remember the version of the default style before migrating
    project->masterScore()->style().setDefaultStyleVersion(ReadStyleHook::styleDefaultByMscVersion(project->mscVersion()));
    const MigrationOptions migrationOptions = configuration()->migrationOptions(migrationType(project->mscVersion()));
    if (!migrationOptions.isApplyMigration) {
        return true;
    }

    Ret ret = migrateProject(project, migrationOptions);
    if (!ret) {
        LOGE() << "failed migration";
    } else {
        LOGI() << "success migration";
    }

    return ret;
}

Ret ProjectMigrator::migrateProject(engraving::EngravingProjectPtr project, const MigrationOptions& opt)
{
    TRACEFUNC;

    mu::engraving::MasterScore* score = project->masterScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::InternalError);
    }

    score->startCmd(TranslatableString("undoableAction", "Migrate project"));

    bool ok = true;
    if (opt.isApplyLeland) {
        ok = applyLelandStyle(score);
    }

    if (ok && opt.isApplyEdwin) {
        ok = applyEdwinStyle(score);
    }

    if (ok && opt.isRemapPercussion) {
        MdlMigrator(score).remapPercussion();
    }

    score->endCmd();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

bool ProjectMigrator::applyLelandStyle(mu::engraving::MasterScore* score)
{
    muse::io::File styleFile(LELAND_STYLE_PATH);
    mu::engraving::Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!mu::engraving::EditStyle::loadStyle(tx, excerpt->excerptScore(), styleFile, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return mu::engraving::EditStyle::loadStyle(tx, score, styleFile, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::applyEdwinStyle(mu::engraving::MasterScore* score)
{
    muse::io::File styleFile(EDWIN_STYLE_PATH);
    mu::engraving::Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!mu::engraving::EditStyle::loadStyle(tx, excerpt->excerptScore(), styleFile, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return mu::engraving::EditStyle::loadStyle(tx, score, styleFile, /*ign*/ false, /*overlap*/ true);
}
