/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QAbstractListModel>

#include "modularity/ioc.h"

#include "async/asyncable.h"

#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"
#include "importexport/mei/imeiconfiguration.h"

#include "inotationwritersregister.h"
#include "iprojectconfiguration.h"
#include "internal/iexportprojectscenario.h"
#include "types/projecttypes.h"

class QItemSelectionModel;

namespace mu::project {
class ExportDialogModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(muse::IInteractive, interactive)
    INJECT(context::IGlobalContext, context)
    INJECT(IProjectConfiguration, configuration)
    INJECT(INotationWritersRegister, writers)
    INJECT(iex::imagesexport::IImagesExportConfiguration, imageExportConfiguration)
    INJECT(iex::musicxml::IMusicXmlConfiguration, musicXmlConfiguration)
    INJECT(iex::midi::IMidiImportExportConfiguration, midiImportExportConfiguration)
    INJECT(iex::audioexport::IAudioExportConfiguration, audioExportConfiguration)
    INJECT(iex::mei::IMeiConfiguration, meiConfiguration)
    INJECT(IExportProjectScenario, exportProjectScenario)

    Q_PROPERTY(int selectionLength READ selectionLength NOTIFY selectionChanged)

    Q_PROPERTY(QVariantMap selectedExportType READ selectedExportType NOTIFY selectedExportTypeChanged)

    Q_PROPERTY(QVariantList availableUnitTypes READ availableUnitTypes NOTIFY selectedExportTypeChanged)
    Q_PROPERTY(int selectedUnitType READ selectedUnitType WRITE setUnitType NOTIFY selectedUnitTypeChanged)

    Q_PROPERTY(int pdfResolution READ pdfResolution WRITE setPdfResolution NOTIFY pdfResolutionChanged)
    Q_PROPERTY(
        bool pdfTransparentBackground READ pdfTransparentBackground WRITE setPdfTransparentBackground NOTIFY pdfTransparentBackgroundChanged)

    Q_PROPERTY(int pngResolution READ pngResolution WRITE setPngResolution NOTIFY pngResolutionChanged)
    Q_PROPERTY(
        bool pngTransparentBackground READ pngTransparentBackground WRITE setPngTransparentBackground NOTIFY pngTransparentBackgroundChanged)

    Q_PROPERTY(
        bool svgTransparentBackground READ svgTransparentBackground WRITE setSvgTransparentBackground NOTIFY svgTransparentBackgroundChanged)
    Q_PROPERTY(bool svgIllustratorCompat READ svgIllustratorCompat WRITE setSvgIllustratorCompat NOTIFY svgIllustratorCompatChanged FINAL)

    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(int bitRate READ bitRate WRITE setBitRate NOTIFY bitRateChanged)

    Q_PROPERTY(bool midiExpandRepeats READ midiExpandRepeats WRITE setMidiExpandRepeats NOTIFY midiExpandRepeatsChanged)
    Q_PROPERTY(bool midiExportRpns READ midiExportRpns WRITE setMidiExportRpns NOTIFY midiExportRpnsChanged)
    Q_PROPERTY(bool midiSpaceLyrics READ midiSpaceLyrics WRITE setMidiSpaceLyrics NOTIFY midiSpaceLyricsChanged)

    Q_PROPERTY(MusicXmlLayoutType musicXmlLayoutType READ musicXmlLayoutType WRITE setMusicXmlLayoutType NOTIFY musicXmlLayoutTypeChanged)

    Q_PROPERTY(int meiExportLayout READ meiExportLayout WRITE setMeiExportLayout NOTIFY meiExportLayoutChanged)
    Q_PROPERTY(int meiUseMuseScoreIds READ meiUseMuseScoreIds WRITE setMeiUseMuseScoreIds NOTIFY meiUseMuseScoreIdsChanged)

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

    bool pdfTransparentBackground() const;
    void setPdfTransparentBackground(const bool& transparent);

    int pngResolution() const;
    void setPngResolution(const int& resolution);

    bool pngTransparentBackground() const;
    void setPngTransparentBackground(const bool& transparent);

    bool svgTransparentBackground() const;
    void setSvgTransparentBackground(const bool& transparent);

    bool svgIllustratorCompat() const;
    void setSvgIllustratorCompat(bool compat);

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

    bool midiSpaceLyrics() const;
    void setMidiSpaceLyrics(bool spaceLyrics);

    bool meiExportLayout() const;
    void setMeiExportLayout(bool exportLayout);

    bool meiUseMuseScoreIds() const;
    void setMeiUseMuseScoreIds(bool useMuseScoreIds);

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

    Q_INVOKABLE void updateExportInfo();

signals:
    void selectionChanged();

    void selectedExportTypeChanged(QVariantMap newExportType);
    void selectedUnitTypeChanged(project::INotationWriter::UnitType newUnitType);

    void pdfResolutionChanged(int resolution);
    void pdfTransparentBackgroundChanged(bool transparent);

    void pngResolutionChanged(int resolution);
    void pngTransparentBackgroundChanged(bool transparent);

    void svgTransparentBackgroundChanged(bool transparent);
    void svgIllustratorCompatChanged(bool compat);

    void availableSampleRatesChanged();
    void sampleRateChanged(int sampleRate);
    void availableBitRatesChanged();
    void bitRateChanged(int bitRate);

    void midiExpandRepeatsChanged(bool expandRepeats);
    void midiExportRpnsChanged(bool exportRpns);
    void midiSpaceLyricsChanged(bool spaceLyrics);

    void musicXmlLayoutTypeChanged(MusicXmlLayoutType layoutType);

    void meiExportLayoutChanged(bool exportLayout);
    void meiUseMuseScoreIdsChanged(bool useMuseScoreIds);

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

    void selectSavedNotations();

    QList<notation::INotationPtr> m_notations {};
    QItemSelectionModel* m_selectionModel = nullptr;

    ExportTypeList m_exportTypeList {};
    ExportType m_selectedExportType = ExportType();
    muse::io::path_t m_exportPath;
    project::INotationWriter::UnitType m_selectedUnitType = project::INotationWriter::UnitType::PER_PART;
};
}

#endif // MU_PROJECT_EXPORTDIALOGMODEL_H
