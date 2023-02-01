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
#include "diagnosticsapi.h"

using namespace mu;
using namespace mu::api;

DiagnosticsApi::DiagnosticsApi(IApiEngine* e)
    : ApiObject(e)
{
}

JSRet DiagnosticsApi::generateDrawData(const QString& scoresDir, const QString& outDir)
{
    Ret ret = diagnosticDrawProvider()->generateDrawData(scoresDir, outDir);
    return retToJs(ret);
}

JSRet DiagnosticsApi::compareDrawData(const QString& ref, const QString& test, const QString& outDiff)
{
    Ret ret = diagnosticDrawProvider()->compareDrawData(ref, test, outDiff);
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
