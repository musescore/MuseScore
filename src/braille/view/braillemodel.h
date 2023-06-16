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

#ifndef MU_BRAILLE_BRAILLEMODEL_H
#define MU_BRAILLE_BRAILLEMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "inotationbraille.h"
#include "ibrailleconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class BrailleModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(braille::IBrailleConfiguration, brailleConfiguration)
    INJECT(braille::INotationBraille, notationBraille)

    Q_PROPERTY(QString brailleInfo READ brailleInfo NOTIFY brailleInfoChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentItemPositionStart READ currentItemPositionStart NOTIFY currentItemChanged)
    Q_PROPERTY(int currentItemPositionEnd READ currentItemPositionEnd NOTIFY currentItemChanged)
    Q_PROPERTY(QString shorcut READ shortcut WRITE setShortcut NOTIFY shortcutFired)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY braillePanelEnabledChanged)

public:
    explicit BrailleModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString brailleInfo() const;

    int cursorPosition() const;
    void setCursorPosition(int pos) const;

    int currentItemPositionStart() const;
    int currentItemPositionEnd() const;

    QString shortcut() const;
    void setShortcut(const QString& sequence) const;

    bool enabled() const;
    void setEnabled(bool e) const;

signals:
    void brailleInfoChanged() const;
    void cursorPositionChanged() const;
    void currentItemChanged() const;
    void shortcutFired() const;
    void braillePanelEnabledChanged() const;

private:
    INotationPtr notation() const;

    void onCurrentNotationChanged();

    void listenChangesInBraille();
    void listenChangesInnotationBraille();
    void listenCursorPositionChanges();
    void listenCurrentItemChanges();
    void listenShortcuts();
    void listenBraillePanelEnabledChanges();
};
}

#endif // MU_BRAILLE_BRAILLEMODEL_H
