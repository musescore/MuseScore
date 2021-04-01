//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_SCOREPREFERENCESMODEL_H
#define MU_APPSHELL_SCOREPREFERENCESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "instruments/iinstrumentsconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "audio/iaudioconfiguration.h"

namespace mu::appshell {
class ScorePreferencesModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, instruments::IInstrumentsConfiguration, instrumentsConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, audio::IAudioConfiguration, audioConfiguration)

    Q_PROPERTY(bool isShowMIDIControls READ isShowMIDIControls WRITE setIsShowMIDIControls NOTIFY isShowMIDIControlsChanged)

public:
    explicit ScorePreferencesModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE QString fileDirectory(const QString& filePath) const;

    bool isShowMIDIControls() const;

public slots:
    void setIsShowMIDIControls(bool value);

signals:
    void isShowMIDIControlsChanged(bool isShowMIDIControls);

private:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PathRole,
        PathFilterRole,
        ChooseTitleRole
    };

    enum class DefaultFileType {
        Undefined,
        FirstInstrumentList,
        SecondInstrumentList,
        FirstScoreOrderList,
        SecondScoreOrderList,
        Style,
        PartStyle
    };

    struct DefaultFileInfo {
        DefaultFileType type = DefaultFileType::Undefined;
        QString title;
        QString path;
        QString pathFilter;
        QString chooseTitle;
    };

    void savePath(DefaultFileType fileType, const QString& path);

    QString firstInstrumentListPath() const;
    void setFirstInstrumentListPath(const QString& path);

    QString secondInstrumentListPath() const;
    void setSecondInstrumentListPath(const QString& path);

    QString firstScoreOrderListPath() const;
    void setFirstScoreOrderListPath(const QString& path);

    QString secondScoreOrderListPath() const;
    void setSecondScoreOrderListPath(const QString& path);

    QString stylePath() const;
    QString partStylePath() const;

    QString instrumentPathFilter() const;
    QString scoreOrderPathFilter() const;
    QString stylePathFilter() const;

    QString instrumentChooseTitle() const;
    QString scoreOrderChooseTitle() const;
    QString styleChooseTitle() const;
    QString partStyleChooseTitle() const;

    void setPath(DefaultFileType fileType, const QString& path);
    QModelIndex fileIndex(DefaultFileType fileType);

    QList<DefaultFileInfo> m_defaultFiles;
};
}

#endif // MU_APPSHELL_SCOREPREFERENCESMODEL_H
