/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"
#include "io/ifilesystem.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "project/inotationwritersregister.h"

namespace mu::notation {
class ImageCapturePanelModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<project::INotationWritersRegister> writersRegister = { this };
    muse::Inject<iex::imagesexport::IImagesExportConfiguration> exportConfiguration = { this };

    Q_OBJECT

    Q_PROPERTY(bool captureModeEnabled READ captureModeEnabled WRITE setCaptureModeEnabled NOTIFY captureModeEnabledChanged)
    Q_PROPERTY(bool hasCapture READ hasCapture NOTIFY hasCaptureChanged)
    Q_PROPERTY(QString captureInfo READ captureInfo NOTIFY captureInfoChanged)
    Q_PROPERTY(int exportFormatIndex READ exportFormatIndex WRITE setExportFormatIndex NOTIFY exportFormatIndexChanged)
    Q_PROPERTY(QString exportButtonText READ exportButtonText NOTIFY exportButtonTextChanged)

public:
    explicit ImageCapturePanelModel(QObject* parent = nullptr);

    bool captureModeEnabled() const;
    void setCaptureModeEnabled(bool enabled);

    bool hasCapture() const;
    QString captureInfo() const;

    int exportFormatIndex() const;
    void setExportFormatIndex(int index);

    QString exportButtonText() const;

    Q_INVOKABLE void exportCapture();
    Q_INVOKABLE void clearCapture();

signals:
    void captureModeEnabledChanged();
    void hasCaptureChanged();
    void captureInfoChanged();
    void exportFormatIndexChanged();
    void exportButtonTextChanged();

private:
    INotationPtr currentNotation() const;
    INotationInteractionPtr currentInteraction() const;

    void onNotationChanged();
    void updateCaptureInfo();

    bool m_captureModeEnabled = false;
    int m_exportFormatIndex = 0; // 0 = PNG, 1 = SVG
};
}
