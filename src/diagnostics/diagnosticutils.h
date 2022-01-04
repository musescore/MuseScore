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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICUTILS_H
#define MU_DIAGNOSTICS_DIAGNOSTICUTILS_H

#include <QObject>

#include "uri.h"
#include "log.h"

namespace mu::diagnostics {
inline bool isDiagnosticObject(const QObject* obj, bool print = false)
{
    if (print) {
        LOGI() << "objectName: " << obj->objectName() << ", className: " << obj->metaObject()->className();
    }
    if (obj->objectName().toLower().contains("diagnostic")) {
        return true;
    }
    return false;
}

inline bool isDiagnosticChild(const QObject* obj, bool print = false)
{
    for (const QObject* ch : obj->children()) {
        if (isDiagnosticObject(ch, print)) {
            return true;
        }

        if (isDiagnosticChild(ch, print)) {
            return true;
        }
    }

    return false;
}

inline bool isDiagnosticParent(const QObject* obj, bool print = false)
{
    QObject* prn = obj->parent();
    if (!prn) {
        return false;
    }

    if (isDiagnosticObject(prn, print)) {
        return true;
    }

    return isDiagnosticParent(prn, print);
}

inline bool isDiagnosticHierarchy(const QObject* obj, bool print = false)
{
    if (isDiagnosticObject(obj, print)) {
        return true;
    }
    if (isDiagnosticParent(obj, print)) {
        return true;
    }
    if (isDiagnosticChild(obj, print)) {
        return true;
    }
    return false;
}

inline std::vector<Uri> removeDiagnosticsUri(const std::vector<Uri>& uris)
{
    std::vector<Uri> nuris;
    for (const Uri& uri : uris) {
        if (!QString::fromStdString(uri.toString()).startsWith("musescore://diagnostics")) {
            nuris.push_back(uri);
        }
    }
    return nuris;
}

inline Uri diagnosticCurrentUri(const std::vector<Uri>& stack)
{
    std::vector<Uri> uris = removeDiagnosticsUri(stack);
    if (!uris.empty()) {
        return uris.back();
    }
    return Uri();
}
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICUTILS_H
