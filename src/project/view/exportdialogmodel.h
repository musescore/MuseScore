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
#ifndef MU_PROJECT_EXPORTDIALOGMODEL_H
#define MU_PROJECT_EXPORTDIALOGMODEL_H

#include "modularity/ioc.h"

#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"

#include "projecttypes.h"
#include "iprojectconfiguration.h"
#include "inotationwritersregister.h"
#include "internal/iexportprojectscenario.h"

class QItemSelectionModel;

namespace mu::project {
class ExportDialogModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(project, framework::IInteractive, interactive)
    INJECT(project, context::IGlobalContext, context)
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, INotationWritersRegister, writers)
    INJECT(project, iex::imagesexport::IImagesExportConfiguration, imageExportConfiguration)
    INJECT(project, iex::musicxml::IMusicXmlConfiguration, musicXmlConfiguration)
    INJECT(project, iex::midi::IMidiImportExportConfiguration, midiImportExportConfiguration)
    INJECT(project, iex::audioexport::IAudioExportConfiguration, audioExportConfiguration)
    INJECT(project, IExportProjectScenario, exportProjectScenario)

    Q_PROPERTY(int selectionLength READ selectionLength NOTIFY selectionChanged)

    Q_PROPERTY(QVariantMap selectedExportType READ selectedExportType NOTIFY selectedExportTypeChanged)

    Q_PROPERTY(QVariantList availableUnitTypes READ availableUnitTypes NOTIFY selectedExportTypeChanged)
    Q_PROPERTY(int selectedUnitType READ selectedUnitType WRITE setUnitType NOTIFY selectedUnitTypeChanged)

    Q_PROPERTY(int pdfResolution READ pdfResolution WRITE setPdfResolution NOTIFY pdfResolutionChanged)
    Q_PROPERTY(int pngResolution READ pngResolution WRITE setPngResolution NOTIFY pngResolutionChanged)
    Q_PROPERTY(
        bool pngTransparentBackground READ pngTransparentBackground WRITE setPngTransparentBackground NOTIFY pngTransparentBackgroundChanged)

    Q_PROPERTY(bool normalizeAudio READ normalizeAudio WRITE setNormalizeAudio NOTIFY normalizeAudioChanged)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(int bitRate READ bitRate WRITE setBitRate NOTIFY bitRateChanged)

    Q_PROPERTY(bool midiExpandRepeats READ midiExpandRepeats WRITE setMidiExpandRepeats NOTIFY midiExpandRepeatsChanged)
    Q_PROPERTY(bool midiExportRpns READ midiExportRpns WRITE setMidiExportRpns NOTIFY midiExportRpnsChanged)

    Q_PROPERTY(MusicXmlLayoutType musicXmlLayoutType READ musicXmlLayoutType WRITE setMusicXmlLayoutType NOTIFY musicXmlLayoutTypeChanged)

    Q_PROPERTY(bool shouldDestinationFolderBeOpenedOnExport READ shouldDestinationFolderBeOpenedOnExport
               WRITE setShouldDestinationFolderBeOpenedOnExport NOTIFY shouldDestinationFolderBeOpenedOnExportChanged)

public:
    explicit ExportDialogModel(QObject* parent = nullptr);
    ~ExportDialogModel();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

    Q_INVOKABLE void setSelected(int scoreIndex, bool selected = true);
    Q_INVOKABLE void setAllSelected(bool selected);
    Q_INVOKABLE void selectCurrentNotation();
    int selectionLength() const;

    Q_INVOKABLE QVariantList exportTypeList() const;
    QVariantMap selectedExportType() const;
    void setExportType(const ExportType& type);
    Q_INVOKABLE void selectExportTypeById(const QString& id);

    QVariantList availableUnitTypes() const;
    int selectedUnitType() const;
    void setUnitType(int unitType);
    void setUnitType(project::INotationWriter::UnitType unitType);

    Q_INVOKABLE bool exportScores();

    int pdfResolution() const;
    void setPdfResolution(const int& resolution);

    int pngResolution() const;
    void setPngResolution(const int& resolution);

    bool pngTransparentBackground() const;
    void setPngTransparentBackground(const bool& transparent);

    bool normalizeAudio() const;
    void setNormalizeAudio(bool normalizeAudio);

    Q_INVOKABLE QList<int> availableSampleRates() const;
    int sampleRate() const;
    void setSampleRate(int sampleRate);

    Q_INVOKABLE QList<int> availableBitRates() const;
    int bitRate() const;
    void setBitRate(int bitRate);

    bool midiExpandRepeats() const;
    void setMidiExpandRepeats(bool expandRepeats);

    bool midiExportRpns() const;
    void setMidiExportRpns(bool exportRpns);

    enum class MusicXmlLayoutType {
        AllLayout,
        AllBreaks,
        ManualBreaks,
        None
    };
    Q_ENUM(MusicXmlLayoutType)

    Q_INVOKABLE QVariantList musicXmlLayoutTypes() const;
    MusicXmlLayoutType musicXmlLayoutType() const;
    void setMusicXmlLayoutType(MusicXmlLayoutType layoutType);

    bool shouldDestinationFolderBeOpenedOnExport() const;
    void setShouldDestinationFolderBeOpenedOnExport(bool enabled);

signals:
    void selectionChanged();

    void selectedExportTypeChanged(QVariantMap newExportType);
    void selectedUnitTypeChanged(project::INotationWriter::UnitType newUnitType);

    void pdfResolutionChanged(int resolution);
    void pngResolutionChanged(int resolution);
    void pngTransparentBackgroundChanged(bool transparent);

    void normalizeAudioChanged(bool normalizeAudio);
    void availableSampleRatesChanged();
    void sampleRateChanged(int sampleRate);
    void availableBitRatesChanged();
    void bitRateChanged(int bitRate);

    void midiExpandRepeatsChanged(bool expandRepeats);
    void midiExportRpnsChanged(bool exportRpns);

    void musicXmlLayoutTypeChanged(MusicXmlLayoutType layoutType);

    void shouldDestinationFolderBeOpenedOnExportChanged(bool shouldDestinationFolderBeOpenedOnExport);

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsMain
    };

    bool isIndexValid(int index) const;

    bool isMainNotation(notation::INotationPtr notation) const;
    notation::IMasterNotationPtr masterNotation() const;

    QList<notation::INotationPtr> m_notations {};
    QItemSelectionModel* m_selectionModel = nullptr;

    ExportTypeList m_exportTypeList {};
    ExportType m_selectedExportType = ExportType();
    project::INotationWriter::UnitType m_selectedUnitType = project::INotationWriter::UnitType::PER_PART;
};
}

#endif // MU_PROJECT_EXPORTDIALOGMODEL_H
