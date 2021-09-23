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
#include "projectmigrator.h"

#include "log.h"

using namespace mu::project;

static const Uri MIGRATION_DIALOG_URI("musescore://project/migration");

Ret ProjectMigrator::migrateEngravingProjectIfNeed(engraving::EngravingProjectPtr project)
{
    UriQuery query(MIGRATION_DIALOG_URI);
    query.addParam("title", Val(project->title()));
    query.addParam("version", Val("3.0"));
    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();

    LOGI() << vals;
    bool isApply = vals.value("apply").toBool();

    return true;
}
