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

#include "engraving/libmscore/score.h"
#include "engraving/libmscore/excerpt.h"
#include "engraving/libmscore/part.h"
#include "engraving/libmscore/scorefont.h"
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

static MigrationType migrationTypeFromAppVersion(const QString& appVersion)
{
    QVersionNumber version = QVersionNumber::fromString(appVersion);
    int major = version.majorVersion();
    int minor = version.minorVersion();

    if (major < 3) {
        return MigrationType::Pre300;
    }

    if (major >= 3 && minor < 6) {
        return MigrationType::Post300AndPre362;
    }

    if (major < 4) {
        return MigrationType::Ver362;
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
    MigrationType migrationType = migrationTypeFromAppVersion(project->appVersion());
    m_resetStyleSettings = true;

    MigrationOptions migrationOptions = configuration()->migrationOptions(migrationType);
    if (migrationOptions.isAskAgain) {
        Ret ret = askAboutMigration(migrationOptions, project->appVersion(), migrationType);

        if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
            return make_ok();
        }

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
    query.addParam("isApplyAutoSpacing", Val(out.isApplyAutoSpacing));
    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();
    out.appVersion = Ms::MSCVERSION;
    out.isApplyMigration = vals.value("isApplyMigration").toBool();
    out.isAskAgain = vals.value("isAskAgain").toBool();
    out.isApplyLeland = vals.value("isApplyLeland").toBool();
    out.isApplyEdwin = vals.value("isApplyEdwin").toBool();
    out.isApplyAutoSpacing = vals.value("isApplyAutoSpacing").toBool();

    return true;
}

void ProjectMigrator::fixHarmonicaIds(Ms::MasterScore* score)
{
    for (Ms::Part* part : score->parts()) {
        for (auto pair : part->instruments()) {
            QString id = pair.second->id();
            // incorrect instrument IDs in pre-4.0
            if (id == "Winds") {
                id = "winds";
            } else if (id == "harmonica-d12high-g") {
                id = "harmonica-d10high-g";
            } else if (id == "harmonica-d12f") {
                id = "harmonica-d10f";
            } else if (id == "harmonica-d12d") {
                id = "harmonica-d10d";
            } else if (id == "harmonica-d12c") {
                id = "harmonica-d10c";
            } else if (id == "harmonica-d12a") {
                id = "harmonica-d10a";
            } else if (id == "harmonica-d12-g") {
                id = "harmonica-d10g";
            }
            pair.second->setId(id);
        }
    }
}

void ProjectMigrator::resetStyleSettings(Ms::MasterScore* score)
{
    // there are a few things that need to be updated no matter which version the score is from (#10499)
    // primarily, the differences made concerning barline thickness and distance
    // these updates take place no matter whether or not the other migration options are checked
    qreal sp = score->spatium();
    Ms::MStyle* style = &score->style();
    style->set(Ms::Sid::dynamicsFontSize, 10.0);
    qreal doubleBarDistance = style->styleMM(Ms::Sid::doubleBarDistance);
    doubleBarDistance -= style->styleMM(Ms::Sid::doubleBarWidth);
    style->set(Ms::Sid::doubleBarDistance, doubleBarDistance / sp);
    qreal endBarDistance = style->styleMM(Ms::Sid::endBarDistance);
    endBarDistance -= (style->styleMM(Ms::Sid::barWidth) + style->styleMM(Ms::Sid::endBarWidth)) / 2;
    style->set(Ms::Sid::endBarDistance, endBarDistance / sp);
    qreal repeatBarlineDotSeparation = style->styleMM(Ms::Sid::repeatBarlineDotSeparation);
    qreal dotWidth = score->scoreFont()->width(Ms::SymId::repeatDot, 1.0);
    repeatBarlineDotSeparation -= (style->styleMM(Ms::Sid::barWidth) + dotWidth) / 2;
    style->set(Ms::Sid::repeatBarlineDotSeparation, repeatBarlineDotSeparation / sp);
    score->resetStyleValue(Ms::Sid::measureSpacing);
    score->setResetDefaults();
}

Ret ProjectMigrator::migrateProject(engraving::EngravingProjectPtr project, const MigrationOptions& opt)
{
    TRACEFUNC;

    Ms::MasterScore* score = project->masterScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::InternalError);
    }

    score->startCmd();

    bool ok = true;
    if (opt.isApplyLeland) {
        ok = applyLelandStyle(score);
        m_resetStyleSettings = false;
    }

    if (ok && opt.isApplyEdwin) {
        ok = applyEdwinStyle(score);
        m_resetStyleSettings = false;
    }

    if (ok && opt.isApplyAutoSpacing) {
        ok = resetAllElementsPositions(score);
    }
    if (score->mscVersion() <= 302) {
        fixHarmonicaIds(score);
    }
    if (ok && score->mscVersion() != Ms::MSCVERSION) {
        score->undo(new Ms::ChangeMetaText(score, "mscVersion", MSC_VERSION));
    }

    if (ok && m_resetStyleSettings) {
        resetStyleSettings(score);
    }

    score->endCmd();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

bool ProjectMigrator::applyLelandStyle(Ms::MasterScore* score)
{
    for (Ms::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(LELAND_STYLE_PATH, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(LELAND_STYLE_PATH, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::applyEdwinStyle(Ms::MasterScore* score)
{
    for (Ms::Excerpt* excerpt : score->excerpts()) {
        if (!excerpt->excerptScore()->loadStyle(EDWIN_STYLE_PATH, /*ign*/ false, /*overlap*/ true)) {
            return false;
        }
    }

    return score->loadStyle(EDWIN_STYLE_PATH, /*ign*/ false, /*overlap*/ true);
}

bool ProjectMigrator::resetAllElementsPositions(Ms::MasterScore* score)
{
    score->setResetAutoplace();
    return true;
}
