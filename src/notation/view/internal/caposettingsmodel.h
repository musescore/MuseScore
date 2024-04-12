/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_NOTATION_CAPOSETTINGSMODEL_H
#define MU_NOTATION_CAPOSETTINGSMODEL_H

#include <QObject>

#include "view/abstractelementpopupmodel.h"

namespace mu::engraving {
class Capo;
}

namespace mu::notation {
class StringItem;
class CapoSettingsModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(bool capoIsOn READ capoIsOn WRITE setCapoIsOn NOTIFY capoIsOnChanged)
    Q_PROPERTY(int fretPosition READ fretPosition WRITE setFretPosition NOTIFY fretPositionChanged)
    Q_PROPERTY(QList<StringItem*> strings READ strings NOTIFY stringsChanged)
    Q_PROPERTY(int capoPlacement READ capoPlacement WRITE setCapoPlacement NOTIFY capoPlacementChanged)
    Q_PROPERTY(
        bool capoTextSpecifiedByUser READ capoTextSpecifiedByUser WRITE setCapoTextSpecifiedByUser NOTIFY capoTextSpecifiedByUserChanged)
    Q_PROPERTY(QString userCapoText READ userCapoText WRITE setUserCapoText NOTIFY userCapoTextChanged)

public:
    explicit CapoSettingsModel(QObject* parent = nullptr);

    bool capoIsOn() const;
    int fretPosition() const;
    QList<StringItem*> strings() const;
    int capoPlacement() const;
    bool capoTextSpecifiedByUser() const;
    QString userCapoText() const;

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void toggleCapoForString(int stringIndex);

    Q_INVOKABLE QVariantList possibleCapoPlacements() const;

public slots:
    void setCapoIsOn(bool isOn);
    void setFretPosition(int position);
    void setCapoPlacement(int placement);
    void setCapoTextSpecifiedByUser(bool value);
    void setUserCapoText(const QString& text);

signals:
    void capoIsOnChanged(bool isOn);
    void fretPositionChanged(int position);
    void stringsChanged(QList<StringItem*> strings);
    void capoPlacementChanged(int placement);
    void capoTextSpecifiedByUserChanged(bool value);
    void userCapoTextChanged(QString text);

private:
    const mu::engraving::CapoParams& params() const;

    QList<StringItem*> m_strings;
};

class StringItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool applyCapo READ applyCapo NOTIFY applyCapoChanged)

public:
    explicit StringItem(QObject* parent = nullptr);

    bool applyCapo() const;
    void setApplyCapo(bool apply);

signals:
    void applyCapoChanged(bool apply);

private:
    bool m_applyCapo = false;
};
} //namespace mu::notation

#endif // MU_NOTATION_CAPOSETTINGSMODEL_H
