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
#include "projectmigrator.h"

#include <QVersionNumber>

#include "mdlmigrator.h"

#include "engraving/types/constants.h"
#include "engraving/dom/score.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/undo.h"
#include "engraving/rw/compat/readstyle.h"

#include "io/file.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::engraving;
using namespace mu::engraving::compat;
using namespace muse;

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

    UNREACHABLE;
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
    MigrationType migrationType = migrationTypeFromMscVersion(project->mscVersion());
    m_resetStyleSettings = true;

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
    query.addParam("isRemapPercussion", Val(out.isRemapPercussion));

    RetVal<Val> rv = interactive()->openSync(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();
    out.appVersion = mu::engraving::Constants::MSC_VERSION;
    out.isApplyMigration = vals.value("isApplyMigration").toBool();
    out.isAskAgain = vals.value("isAskAgain").toBool();
    out.isApplyLeland = vals.value("isApplyLeland").toBool();
    out.isApplyEdwin = vals.value("isApplyEdwin").toBool();
    out.isRemapPercussion = vals.value("isRemapPercussion").toBool();

    return true;
}

void ProjectMigrator::resetStyleSettings(mu::engraving::MasterScore* score)
{
    // there are a few things that need to be updated no matter which version the score is from (#10499)
    // primarily, the differences made concerning barline thickness and distance
    // these updates take place no matter whether or not the other migration options are checked
    qreal sp = score->style().spatium();
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

bool ProjectMigrator::resetCrossBeams(engraving::MasterScore* score)
{
    score->setResetCrossBeams();
    return true;
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
        m_resetStyleSettings = false;
    }

    if (ok && opt.isApplyEdwin) {
        ok = applyEdwinStyle(score);
        m_resetStyleSettings = false;
    }

    if (ok && opt.isRemapPercussion) {
        MdlMigrator(score).remapPercussion();
    }

    if (ok && score->mscVersion() < 300) {
        ok = resetAllElementsPositions(score);
    }

    if (ok && score->mscVersion() <= 206) {
        ok = resetCrossBeams(score);
    }

    if (ok && score->mscVersion() != mu::engraving::Constants::MSC_VERSION) {
        score->undo(new mu::engraving::ChangeMetaText(score, u"mscVersion", String::fromAscii(mu::engraving::Constants::MSC_VERSION_STR)));
    }

    if (ok && m_resetStyleSettings) {
        resetStyleSettings(score);
        score->setLayoutAll();
    }
    score->endCmd();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

bool ProjectMigrator::applyLelandStyle(mu::engraving::MasterScore* score)
{
    muse::io::File styleFile(LELAND_STYLE_PATH);
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(styleFile, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(styleFile, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::applyEdwinStyle(mu::engraving::MasterScore* score)
{
    muse::io::File styleFile(EDWIN_STYLE_PATH);
    for (mu::engraving::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(styleFile, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(styleFile, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::resetAllElementsPositions(mu::engraving::MasterScore* score)
{
    score->setResetAutoplace();
    return true;
}
