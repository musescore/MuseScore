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

#include "engraving/types/constants.h"
#include "engraving/libmscore/score.h"
#include "engraving/libmscore/excerpt.h"
#include "engraving/libmscore/undo.h"

#include "rw/compat/readstyle.h"

#include "log.h"

#include <QVersionNumber>

using namespace mu;
using namespace mu::project;
using namespace mu::engraving::compat;

static const Uri MIGRATION_DIALOG_URI("musescore://project/migration");
static const QString LELAND_STYLE_PATH(":/engraving/styles/migration-306-style-Leland.mss");
static const QString EDWIN_STYLE_PATH(":/engraving/styles/migration-306-style-Edwin.mss");

static MigrationType migrationTypeFromMscVersion(int mscVersion)
{
    if (mscVersion < 302) {
        return MigrationType::Pre_3_6;
    }

    if (mscVersion < 400) {
        return MigrationType::Ver_3_6;
    }

    if (mscVersion < 410) {
        return MigrationType::Ver_4_0;
    }

    LOGE() << "Unhandled MSC_VERSION for migration: " << mscVersion;
    UNREACHABLE;
    return MigrationType::Unknown;
}

Ret ProjectMigrator::migrateEngravingProjectIfNeed(engraving::EngravingProjectPtr project)
{
    if (project->mscVersion() >= engraving::Constants::MSC_VERSION) {
        return true;
    }

    //! NOTE If the migration is not done, then the default style for the score is determined by the version.
    //! When migrating, the version becomes the current one, so remember the version of the default style before migrating
    project->masterScore()->style().setDefaultStyleVersion(ReadStyleHook::styleDefaultByMscVersion(project->mscVersion()));
    MigrationType migrationType = migrationTypeFromMscVersion(project->mscVersion());

    MigrationOptions migrationOptions = configuration()->migrationOptions(migrationType);
    if (migrationOptions.isAskAgain) {
        Ret ret = askAboutMigration(migrationOptions, project->appVersion(), migrationType);

        if (!ret) {
            return ret;
        }

        configuration()->setMigrationOptions(migrationType, migrationOptions);
    }

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

Ret ProjectMigrator::askAboutMigration(MigrationOptions& out, const QString& appVersion, MigrationType migrationType)
{
    UriQuery query(MIGRATION_DIALOG_URI);
    query.addParam("appVersion", Val(appVersion));
    query.addParam("migrationType", Val(migrationType));
    query.addParam("isApplyLeland", Val(out.isApplyLeland));
    query.addParam("isApplyEdwin", Val(out.isApplyEdwin));
    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();
    out.appVersion = mu::engraving::Constants::MSC_VERSION;
    out.isApplyMigration = vals.value("isApplyMigration").toBool();
    out.isAskAgain = vals.value("isAskAgain").toBool();
    out.isApplyLeland = vals.value("isApplyLeland").toBool();
    out.isApplyEdwin = vals.value("isApplyEdwin").toBool();

    return true;
}

void ProjectMigrator::fixStyleSettingsPre400(mu::engraving::MasterScore* score)
{
    // there are a few things that need to be updated no matter which version the score is from (#10499)
    // primarily, the differences made concerning barline thickness and distance
    // these updates take place no matter whether or not the other migration options are checked
    qreal sp = score->spatium();
    mu::engraving::MStyle* style = &score->style();
    style->set(mu::engraving::Sid::dynamicsFontSize, 10.0);
    qreal doubleBarDistance = style->styleMM(mu::engraving::Sid::doubleBarDistance);
    doubleBarDistance -= style->styleMM(mu::engraving::Sid::doubleBarWidth);
    style->set(mu::engraving::Sid::doubleBarDistance, doubleBarDistance / sp);
    qreal endBarDistance = style->styleMM(mu::engraving::Sid::endBarDistance);
    endBarDistance -= (style->styleMM(mu::engraving::Sid::barWidth) + style->styleMM(mu::engraving::Sid::endBarWidth)) / 2;
    style->set(mu::engraving::Sid::endBarDistance, endBarDistance / sp);
    qreal repeatBarlineDotSeparation = style->styleMM(mu::engraving::Sid::repeatBarlineDotSeparation);
    qreal dotWidth = score->engravingFont()->width(mu::engraving::SymId::repeatDot, 1.0);
    repeatBarlineDotSeparation -= (style->styleMM(mu::engraving::Sid::barWidth) + dotWidth) / 2;
    style->set(mu::engraving::Sid::repeatBarlineDotSeparation, repeatBarlineDotSeparation / sp);
    score->resetStyleValue(mu::engraving::Sid::measureSpacing);
}

Ret ProjectMigrator::migrateProject(engraving::EngravingProjectPtr project, const MigrationOptions& opt)
{
    TRACEFUNC;

    mu::engraving::MasterScore* score = project->masterScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::InternalError);
    }

    score->startCmd();

    bool ok = true;
    bool needFixStylePre400 = true;

    if (score->mscVersion() < 302 && opt.isApplyLeland) {
        ok = applyLelandStyle(score);

        // The affected style settings are already overridden by the Leland style sheet
        needFixStylePre400 = false;
    }

    if (ok && score->mscVersion() < 302 && opt.isApplyEdwin) {
        ok = applyEdwinStyle(score);

        // The affected style settings are already overridden by the Edwin style sheet
        needFixStylePre400 = false;
    }

    if (ok && score->mscVersion() < 300) {
        score->mutLayoutOptions().resetAutoplace = true;
    }

    if (ok && score->mscVersion() < 400 && needFixStylePre400) {
        // TODO: this should happen while reading the file, and for every file!
        // Not as part of an optional migration process triggered from the UI
        fixStyleSettingsPre400(score);
    }

    if (ok && score->mscVersion() < 400) {
        score->mutLayoutOptions().resetDefaultsPre400 = true;
    }

    if (ok && score->mscVersion() != mu::engraving::Constants::MSC_VERSION) {
        // TODO: either this should happen automatically when *saving* the file,
        // or we should not store this info in meta tags at all
        score->undo(new mu::engraving::ChangeMetaText(score, u"mscVersion", String::fromAscii(mu::engraving::Constants::MSC_VERSION_STR)));
    }

    score->endCmd();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

bool ProjectMigrator::applyLelandStyle(mu::engraving::MasterScore* score)
{
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(LELAND_STYLE_PATH, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(LELAND_STYLE_PATH, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::applyEdwinStyle(mu::engraving::MasterScore* score)
{
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(EDWIN_STYLE_PATH, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(EDWIN_STYLE_PATH, /*ign*/ false, /*overlap*/ true);
}
