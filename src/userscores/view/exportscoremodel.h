//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_USERSCORES_EXPORTSCOREMODEL_H
#define MU_USERSCORES_EXPORTSCOREMODEL_H

#include "modularity/ioc.h"

#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "iuserscoresconfiguration.h"
#include "notation/inotationwritersregister.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "iexportscoreservice.h"

namespace mu::uicomponents {
class ItemMultiSelectionModel;
}

namespace mu::userscores {
class ExportScoreModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(userscores, framework::IInteractive, interactive)
    INJECT(userscores, context::IGlobalContext, context)
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, notation::INotationWritersRegister, writers)
    INJECT(userscores, iex::imagesexport::IImagesExportConfiguration, imageExportConfiguration)
    INJECT(userscores, IExportScoreService, exportScoreService)

    Q_PROPERTY(int selectionLength READ selectionLength NOTIFY selectionChanged)
    Q_PROPERTY(int pdfResolution READ pdfResolution WRITE setPdfResolution)
    Q_PROPERTY(int pngResolution READ pngResolution WRITE setPngResolution)
    Q_PROPERTY(bool pngTransparentBackground READ pngTransparentBackground WRITE setPngTransparentBackground)

public:
    explicit ExportScoreModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

    Q_INVOKABLE void setSelected(int scoreIndex, bool selected = true);
    Q_INVOKABLE void toggleSelected(int scoreIndex);
    Q_INVOKABLE void setAllSelected(bool selected);
    Q_INVOKABLE void selectCurrentNotation();
    int selectionLength() const;

    Q_INVOKABLE QString exportPath() const;
    Q_INVOKABLE void setExportPath(QString path);
    Q_INVOKABLE QString chooseExportPath();

    Q_INVOKABLE void setExportSuffix(QString suffix);

    Q_INVOKABLE bool exportScores();

    int pdfResolution() const;
    void setPdfResolution(const int& resolution);

    int pngResolution() const;
    void setPngResolution(const int& resolution);

    bool pngTransparentBackground() const;
    void setPngTransparentBackground(const bool& transparent);

signals:
    void selectionChanged();

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsMain
    };

    bool isMainNotation(notation::INotationPtr notation) const;

    bool isNotationIndexValid(int index) const;

    notation::IMasterNotationPtr masterNotation() const;
    QList<int> selectedRows() const;

    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
    QList<notation::INotationPtr> m_notations;

    QString exportFliter() const;

    io::path m_exportPath;
};
}

#endif // MU_USERSCORES_EXPORTSCOREMODEL_H
