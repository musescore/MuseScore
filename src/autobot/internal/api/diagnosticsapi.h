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
#ifndef MU_API_DIAGNOSTICSAPI_H
#define MU_API_DIAGNOSTICSAPI_H

#include <QString>

#include "apiobject.h"
#include "jsretval.h"

#include "modularity/ioc.h"
#include "diagnostics/idiagnosticdrawprovider.h"

namespace mu::api {
class DiagnosticsApi : public ApiObject
{
    Q_OBJECT

    INJECT(diagnostics::IDiagnosticDrawProvider, diagnosticDrawProvider)

public:
    DiagnosticsApi(IApiEngine* e);

    Q_INVOKABLE JSRet generateDrawData(const QString& scoresDir, const QString& outDir, const QJSValue& opt = QJSValue());
    Q_INVOKABLE JSRet compareDrawData(const QString& ref, const QString& test, const QString& outDiff, const QJSValue& opt = QJSValue());
    Q_INVOKABLE JSRet drawDataToPng(const QString& dataFile, const QString& outFile);
    Q_INVOKABLE JSRet drawDiffToPng(const QString& diffFile, const QString& refFile, const QString& outFile);
};
}

#endif // MU_API_DIAGNOSTICSAPI_H
