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
#include "imagecapturepanelmodel.h"

#include "types/translatablestring.h"
#include "project/inotationwriter.h"
#include "project/inotationwritersregister.h"
#include "engraving/engravingproject.h"
#include "io/path.h"
#include "io/file.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;

ImageCapturePanelModel::ImageCapturePanelModel(QObject* parent)
    : QObject(parent)
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    onNotationChanged();
}

void ImageCapturePanelModel::onNotationChanged()
{
    INotationInteractionPtr interaction = currentInteraction();
    if (!interaction) {
        return;
    }

    interaction->selectionChanged().onNotify(this, [this]() {
        updateCaptureInfo();
    });

    updateCaptureInfo();
}

bool ImageCapturePanelModel::captureModeEnabled() const
{
    return m_captureModeEnabled;
}

void ImageCapturePanelModel::setCaptureModeEnabled(bool enabled)
{
    if (m_captureModeEnabled == enabled) {
        return;
    }

    m_captureModeEnabled = enabled;

    INotationInteractionPtr interaction = currentInteraction();
    if (interaction) {
        interaction->setImageCaptureMode(enabled);
    }

    emit captureModeEnabledChanged();

    // Update capture info when mode changes
    updateCaptureInfo();
}

bool ImageCapturePanelModel::hasCapture() const
{
    INotationInteractionPtr interaction = currentInteraction();
    if (!interaction) {
        return false;
    }

    muse::RectF bounds = interaction->captureBounds();
    return !bounds.isEmpty();
}

QString ImageCapturePanelModel::captureInfo() const
{
    INotationInteractionPtr interaction = currentInteraction();
    if (!interaction) {
        return QString();
    }

    muse::RectF bounds = interaction->captureBounds();
    if (bounds.isEmpty()) {
        if (m_captureModeEnabled) {
            return muse::qtrc("notation", "Shift+drag to create capture region");
        }
        return muse::qtrc("notation", "Enable capture mode to begin");
    }

    // Show dimensions in staff spaces (sp)
    return muse::qtrc("notation", "Region: %1 × %2 sp")
           .arg(QString::number(bounds.width(), 'f', 1))
           .arg(QString::number(bounds.height(), 'f', 1));
}

void ImageCapturePanelModel::exportCapture()
{
    INotationPtr notation = currentNotation();
    INotationInteractionPtr interaction = currentInteraction();
    if (!notation || !interaction) {
        LOGE() << "No notation or interaction available";
        return;
    }

    muse::RectF bounds = interaction->captureBounds();
    LOGI() << "Export capture bounds: " << bounds.x() << ", " << bounds.y()
           << " size: " << bounds.width() << " x " << bounds.height();
    if (bounds.isEmpty()) {
        LOGW() << "No capture region defined";
        return;
    }

    // Show file save dialog
    io::path_t defaultPath = notation->project()->path();
    if (!defaultPath.empty()) {
        muse::io::path_t baseName = muse::io::basename(defaultPath);
        defaultPath = muse::io::dirpath(defaultPath) + "/" + baseName.toString() + "_capture.png";
    } else {
        defaultPath = "capture.png";
    }

    io::path_t selectedPath = interactive()->selectSavingFileSync(
        muse::trc("notation", "Export Image Capture"),
        defaultPath,
        { muse::trc("project/export", "PNG image") + " (*.png)" }
        );

    if (selectedPath.empty()) {
        return; // User cancelled
    }

    // Get PNG writer
    project::INotationWriterPtr writer = writersRegister()->writer("png");
    if (!writer) {
        LOGE() << "PNG writer not available";
        interactive()->error(muse::trc("notation", "Export failed"),
                             muse::trc("notation", "PNG export format is not available."));
        return;
    }

    // Prepare export options
    project::INotationWriter::Options options;
    options[project::INotationWriter::OptionKey::TRANSPARENT_BACKGROUND] = Val(exportConfiguration()->exportPngWithTransparentBackground());
    // Add capture rectangle option - pass components separately since Val doesn't preserve RectF
    options[project::INotationWriter::OptionKey::CAPTURE_RECT_X] = Val(bounds.x());
    options[project::INotationWriter::OptionKey::CAPTURE_RECT_Y] = Val(bounds.y());
    options[project::INotationWriter::OptionKey::CAPTURE_RECT_W] = Val(bounds.width());
    options[project::INotationWriter::OptionKey::CAPTURE_RECT_H] = Val(bounds.height());
    LOGI() << "Stored capture rect components in options";

    // Export to file
    muse::io::File file(selectedPath);
    if (!file.open(muse::io::File::WriteOnly)) {
        LOGE() << "Failed to open file for writing: " << selectedPath;
        interactive()->error(muse::trc("notation", "Export failed"),
                             muse::trc("notation", "Could not open file for writing."));
        return;
    }

    file.setMeta("file_path", selectedPath.toStdString());

    Ret ret = writer->write(notation, file, options);
    file.close();

    if (!ret) {
        LOGE() << "Export failed: " << ret.toString();
        interactive()->error(muse::trc("notation", "Export failed"), ret.text());
    } else {
        LOGI() << "Successfully exported capture to: " << selectedPath;
        interactive()->info(muse::trc("notation", "Export successful"),
                            muse::trc("notation", "Image capture exported successfully."));
    }
}

void ImageCapturePanelModel::clearCapture()
{
    INotationInteractionPtr interaction = currentInteraction();
    if (interaction) {
        interaction->clearImageCapture();
        updateCaptureInfo();
    }
}

INotationPtr ImageCapturePanelModel::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr ImageCapturePanelModel::currentInteraction() const
{
    INotationPtr notation = currentNotation();
    return notation ? notation->interaction() : nullptr;
}

void ImageCapturePanelModel::updateCaptureInfo()
{
    emit hasCaptureChanged();
    emit captureInfoChanged();
}
