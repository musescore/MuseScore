/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "diagnosticsapi.h"

using namespace mu;
using namespace mu::api;
using namespace mu::diagnostics;

DiagnosticsApi::DiagnosticsApi(IApiEngine* e)
    : ApiObject(e)
{
}

JSRet DiagnosticsApi::generateDrawData(const QString& scoresDir, const QString& outDir, const QJSValue& obj)
{
    GenOpt opt;
    if (obj.hasProperty("pageSize")) {
        QJSValue ps = obj.property("pageSize");
        opt.pageSize.setWidth(ps.property("width").toNumber());
        opt.pageSize.setHeight(ps.property("height").toNumber());
    }

    Ret ret = diagnosticDrawProvider()->generateDrawData(scoresDir, outDir, opt);
    return retToJs(ret);
}

JSRet DiagnosticsApi::compareDrawData(const QString& ref, const QString& test, const QString& outDiff, const QJSValue& obj)
{
    ComOpt opt;
    if (obj.hasProperty("isCopySrc")) {
        opt.isCopySrc = obj.property("isCopySrc").toBool();
    }

    if (obj.hasProperty("isMakePng")) {
        opt.isMakePng = obj.property("isMakePng").toBool();
    }

    Ret ret = diagnosticDrawProvider()->compareDrawData(ref, test, outDiff, opt);
    return retToJs(ret);
}

JSRet DiagnosticsApi::drawDataToPng(const QString& dataFile, const QString& outFile)
{
    Ret ret = diagnosticDrawProvider()->drawDataToPng(dataFile, outFile);
    return retToJs(ret);
}

JSRet DiagnosticsApi::drawDiffToPng(const QString& diffFile, const QString& refFile, const QString& outFile)
{
    Ret ret = diagnosticDrawProvider()->drawDiffToPng(diffFile, refFile, outFile);
    return retToJs(ret);
}
