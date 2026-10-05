/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "mnxtestutils.h"

#include <gtest/gtest.h>

#include <memory>

#include "engraving/dom/masterscore.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "framework/global/modularity/ioc.h"
#include "importexport/mnx/imnxconfiguration.h"
#include "importexport/mnx/internal/export/mnxexporter.h"
#include "importexport/mnx/internal/import/mnximporter.h"
#include "log.h"
#include "types/ret.h"

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

using namespace mu::engraving;
using namespace muse;

namespace mu::iex::mnxio {
void fixupAndLayoutScore(MasterScore* score)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("MNX test fixup"), [&](Transaction&) {
        score->connectTies();
        score->masterScore()->rebuildMidiMapping();
        score->doLayout();
    });
}

std::string exportMnxJson(Score* score)
{
    auto mnxConfiguration = muse::modularity::globalIoc()->resolve<IMnxConfiguration>("iex_mnx");
    const bool exportBeams = mnxConfiguration ? mnxConfiguration->mnxExportBeams() : true;
    const bool exportRestPositions = mnxConfiguration ? mnxConfiguration->mnxExportRestPositions() : false;
    LOGI() << "MNX export initiated; exportBeams=" << (exportBeams ? "true" : "false")
           << " exportRestPositions=" << (exportRestPositions ? "true" : "false");
    MnxExporter exporter(score, exportBeams, exportRestPositions);
    Ret ret = exporter.exportMnx();
    if (!ret.success()) {
        return {};
    }

    return exporter.mnxDocument().root()->dump(2);
}

MasterScore* importMnxFromJson(const std::string& json, const String& virtualPath)
{
    auto score = std::unique_ptr<MasterScore>(
        compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(muse::io::path_t(virtualPath)));

    try {
        auto doc = mnx::Document::create(json.data(), json.size());
        if (!mnx::validation::schemaValidate(doc)) {
            ADD_FAILURE() << "MNX is not valid: " << virtualPath.toStdString();
            return nullptr;
        }
        if (doc.global().measures().empty()) {
            ADD_FAILURE() << "MNX contains no measures: " << virtualPath.toStdString();
            return nullptr;
        }
        MnxImporter importer(score.get(), std::move(doc));
        importer.importMnx();
    } catch (const std::exception& ex) {
        ADD_FAILURE() << "MNX failed to parse: " << ex.what();
        return nullptr;
    }

    return score.release();
}
} // namespace mu::iex::mnxio
