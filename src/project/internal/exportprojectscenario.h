/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <deque>

#include <QObject>

#include "modularity/ioc.h"

#include "iexportprojectscenario.h"
#include "iprojectconfiguration.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "inotationwritersregister.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "project/iprojectsession.h"
#include "io/ifilesystem.h"
#include "async/asyncable.h"
#include "notation/types/viewmode.h"

namespace mu::project {
class ExportProjectScenario : public IExportProjectScenario, public muse::async::Asyncable, public muse::Contextable
{
public:
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<iex::imagesexport::IImagesExportConfiguration> imagesExportConfiguration;
    muse::GlobalInject<INotationWritersRegister> writers;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<project::IProjectSession> context = { this };

    ExportProjectScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx)
    {
    }

    std::vector<INotationWriter::UnitType> supportedUnitTypes(const ExportType& exportType) const override;

    muse::async::Promise<muse::io::path_t> askExportPath(const notation::INotationPtrList& notations, const ExportType& exportType,
                                                         INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                                                         muse::io::path_t defaultDirPath = "") const override;

    muse::async::Promise<muse::Ret> exportScores(notation::INotationPtrList notations, const muse::io::path_t destinationPath,
                                                 INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                                                 bool openDestinationFolderOnExport = false) const override;

    muse::async::Channel<muse::io::path_t> fileExported() const override;

    const ExportInfo& exportInfo() const override;
    void setExportInfo(const ExportInfo& exportInfo) override;

private:
    enum class FileConflictPolicy {
        Undefined,
        SkipAll,
        ReplaceAll
    };

    struct ExportFile {
        muse::io::path_t path;
        notation::INotationPtrList notations; // one notation, or a list of them if it is multi-part
        bool isMultiPart = false;
        INotationWriter::Options options;
    };

    struct ExportState;
    using ExportStatePtr = std::shared_ptr<ExportState>;

    /// When the user is trying to export a part that is a "potential excerpt", the corresponding score
    /// is not initialized yet, so we can't be certain about the page count. We should not initialize
    /// these scores either, until the user really starts the export, because initializing these scores
    /// means making changes to the file, which can't be done without the user's consent.
    bool guessIsCreatingOnlyOneFile(const notation::INotationPtrList& notations, INotationWriter::UnitType unitType) const;

    bool isMainNotation(notation::INotationPtr notation) const;
    notation::IMasterNotationPtr masterNotation() const;

    muse::io::path_t completeExportPath(const muse::io::path_t& basePath, notation::INotationPtr notation, bool isMain,
                                        bool isExportingOnlyOneScore, int pageIndex = -1) const;

    std::vector<ExportFile> makeExportFiles(const notation::INotationPtrList& notations, const muse::io::path_t& destinationPath,
                                            INotationWriter::UnitType unitType, bool isCreatingOnlyOneFile) const;

    void startNextExportLater() const;
    void startExport(const notation::INotationPtrList& notations, const muse::io::path_t& destinationPath,
                     INotationWriter::UnitType unitType, bool openDestinationFolderOnExport,
                     const muse::async::Promise<muse::Ret>::Resolve& resolve) const;

    void exportNextFiles(const ExportStatePtr& state) const;
    void finishExport(const ExportStatePtr& state, const muse::Ret& ret) const;

    void askForReplaceFile(const ExportStatePtr& state) const;
    void askForRetry(const ExportStatePtr& state) const;
    void continueExportLater(const ExportStatePtr& state) const;

    muse::async::Promise<muse::Ret> writeFile(const INotationWriterPtr& writer, const ExportFile& file) const;

    //! Returns true if the export should go on with the next file
    bool onFileWritten(const ExportStatePtr& state, const muse::Ret& ret) const;

    void showExportProgress(bool isAudioExport) const;

    void openFolder(const muse::io::path_t& path) const;

    std::vector<notation::ViewMode> viewModes(const notation::INotationPtrList& notations) const;
    void setViewModes(const notation::INotationPtrList& notations, const std::vector<notation::ViewMode>& viewModes) const;
    void setViewModes(const notation::INotationPtrList& notations, notation::ViewMode viewMode) const;

    mutable muse::Progress m_exportProgress;
    mutable muse::async::Channel<muse::io::path_t> m_fileExported;
    ExportInfo m_exportInfo;

    mutable std::deque<std::function<void()> > m_pendingExports;
    mutable bool m_isExporting = false;

    //! NOTE The context of the timers, so that they do not outlive the scenario
    mutable QObject m_timersContext;
};
}
