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
#ifndef MU_AUTOBOT_AUTOBOTMODEL_H
#define MU_AUTOBOT_AUTOBOTMODEL_H

#include <QObject>
#include <QVariant>

#include "modularity/ioc.h"
#include "iautobot.h"
#include "async/asyncable.h"
#include "abfilesmodel.h"

namespace mu::autobot {
class AutobotModel : public QObject, public async::Asyncable
{
    INJECT(autobot, IAutobot, autobot)

    Q_OBJECT
    Q_PROPERTY(QVariantList testCases READ testCases CONSTANT)
    Q_PROPERTY(AbFilesModel * files READ files CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit AutobotModel(QObject* parent = nullptr);

    QVariantList testCases() const;
    AbFilesModel* files() const;
    QString status() const;

    Q_INVOKABLE void runAll(const QString& testCaseName);
    Q_INVOKABLE void runFile(const QString& testCaseName, int fileIndex);
    Q_INVOKABLE void stop();

signals:
    void statusChanged();

private:

    AbFilesModel* m_files = nullptr;
};
}

#endif // MU_AUTOBOT_AUTOBOTMODEL_H
