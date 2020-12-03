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
#ifndef MU_FRAMEWORK_INTERACTIVETESTSMODEL_H
#define MU_FRAMEWORK_INTERACTIVETESTSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "async/asyncable.h"

namespace mu {
namespace framework {
class InteractiveTestsModel : public QObject, async::Asyncable
{
    Q_OBJECT

    INJECT(ui, IInteractive, interactive)

    Q_PROPERTY(QString currentUri READ currentUri NOTIFY currentUriChanged)

public:
    explicit InteractiveTestsModel(QObject* parent = nullptr);

    QString currentUri() const;

    Q_INVOKABLE void openSampleDialog();
    Q_INVOKABLE void openSampleDialogAsync();

    Q_INVOKABLE void openWidgetDialog();
    Q_INVOKABLE void openWidgetDialogAsync();

    Q_INVOKABLE void question();
    Q_INVOKABLE void customQuestion();

    Q_INVOKABLE void information();
    Q_INVOKABLE void warning();
    Q_INVOKABLE void critical();

    Q_INVOKABLE void require();

signals:
    void currentUriChanged(QString currentUri);

private:
    void setCurrentUri(const Uri& uri);

    QString m_currentUri;
};
}
}

#endif // MU_FRAMEWORK_INTERACTIVETESTSMODEL_H
