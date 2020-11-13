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

#ifndef MU_WORKSPACE_NEWWORKSPACEMODEL_H
#define MU_WORKSPACE_NEWWORKSPACEMODEL_H

#include <QObject>

namespace mu::workspace {
class NewWorkspaceModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString workspaceName READ workspaceName WRITE setWorkspaceName NOTIFY workspaceNameChanged)
    Q_PROPERTY(bool importUiPreferences READ importUiPreferences WRITE setImportUiPreferences NOTIFY importUiPreferencesChanged)
    Q_PROPERTY(bool importUiArrangement READ importUiArrangement WRITE setImportUiArrangement NOTIFY importUiArrangementChanged)
    Q_PROPERTY(bool importPalettes READ importPalettes WRITE setImportPalettes NOTIFY importPalettesChanged)
    Q_PROPERTY(bool importToolbarCustomization READ importToolbarCustomization WRITE setImportToolbarCustomization NOTIFY importToolbarCustomizationChanged)

public:
    explicit NewWorkspaceModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariant createWorkspace();

    QString workspaceName() const;
    bool importUiPreferences() const;
    bool importUiArrangement() const;
    bool importPalettes() const;
    bool importToolbarCustomization() const;

public slots:
    void setWorkspaceName(const QString& name);
    void setImportUiPreferences(bool needImport);
    void setImportUiArrangement(bool needImport);
    void setImportPalettes(bool needImport);
    void setImportToolbarCustomization(bool needImport);

signals:
    void workspaceNameChanged(QString name);
    void importUiPreferencesChanged(bool needImport);
    void importUiArrangementChanged(bool needImport);
    void importPalettesChanged(bool needImport);
    void importToolbarCustomizationChanged(bool needImport);

private:
    QString m_workspaceName;
    bool m_importUiPreferences = false;
    bool m_importUiArrangement = false;
    bool m_importPalettes = false;
    bool m_importToolbarCustomization = false;
};
}

#endif // MU_WORKSPACE_NEWWORKSPACEMODEL_H
