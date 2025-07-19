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

#ifndef MU_BRAILLE_BRAILLEMODEL_H
#define MU_BRAILLE_BRAILLEMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"

#include "ibrailleconfiguration.h"
#include "inotationbraille.h"

namespace mu::engraving {
class BrailleModel : public QObject, public muse::Injectable, public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(QString brailleInfo READ brailleInfo NOTIFY brailleInfoChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentItemPositionStart READ currentItemPositionStart NOTIFY currentItemChanged)
    Q_PROPERTY(int currentItemPositionEnd READ currentItemPositionEnd NOTIFY currentItemChanged)
    Q_PROPERTY(QString keys READ keys WRITE setKeys NOTIFY keysFired)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY braillePanelEnabledChanged)
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY brailleModeChanged)
    Q_PROPERTY(QString cursorColor READ cursorColor NOTIFY brailleModeChanged)

    muse::Inject<context::IGlobalContext> context = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<braille::IBrailleConfiguration> brailleConfiguration = { this };
    muse::Inject<braille::INotationBraille> notationBraille = { this };
public:
    explicit BrailleModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString brailleInfo() const;

    int cursorPosition() const;
    void setCursorPosition(int pos);

    int currentItemPositionStart() const;
    int currentItemPositionEnd() const;

    QString keys() const;
    void setKeys(const QString& sequence);

    bool enabled() const;
    void setEnabled(bool e);

    int mode() const;
    void setMode(int mode);
    bool isNavigationMode();
    bool isBrailleInputMode();

    QString cursorColor() const;

signals:
    void brailleInfoChanged();
    void cursorPositionChanged();
    void currentItemChanged();
    void keysFired();
    void braillePanelEnabledChanged();
    void brailleModeChanged();
    void cursorColorChanged();

private:
    notation::INotationPtr notation() const;

    void onCurrentNotationChanged();

    void listenChangesInBraille();
    void listenChangesInnotationBraille();
    void listenCursorPositionChanges();
    void listenCurrentItemChanges();
    void listenKeys();
    void listenBraillePanelEnabledChanges();
    void listenBrailleModeChanges();
};
}

#endif // MU_BRAILLE_BRAILLEMODEL_H
