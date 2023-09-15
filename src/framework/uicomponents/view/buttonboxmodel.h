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

#ifndef MU_UICOMPONENTS_BUTTONBOXMODEL_H
#define MU_UICOMPONENTS_BUTTONBOXMODEL_H

#include <QObject>
#include <QQuickItem>

#include "translation.h"
#include "qmllistproperty.h"

namespace mu::uicomponents {
class ButtonBoxModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ButtonLayout buttonLayout READ buttonLayout WRITE setButtonLayout NOTIFY buttonLayoutChanged)

    Q_PROPERTY(QQmlListProperty<QQuickItem> buttonsItems READ buttonsItems NOTIFY buttonsItemsChanged)

public:
    explicit ButtonBoxModel(QObject* parent = nullptr);

    enum ButtonType {
        NoButton,
        Ok,
        Continue,
        RestoreDefaults,
        Reset,
        Apply,
        Help,
        Discard,
        Cancel,
        Close,
        Ignore,
        Retry,
        Abort,
        NoToAll,
        No,
        YesToAll,
        Yes,
        Open,
        DontSave,
        SaveAll,
        Save,
        Next,
        Back,
        Select,
        Clear,
        Done,

        CustomButton,

        FirstButton       = Ok,
        LastButton        = CustomButton
    };
    Q_ENUM(ButtonType)

    enum ButtonRole { // Keep updated with buttonRoleLayouts
        AcceptRole,
        RejectRole,
        DestructiveRole,
        ResetRole,
        ApplyRole,
        RetryRole,
        HelpRole,
        ContinueRole,
        BackRole,
        CustomRole
    };
    Q_ENUM(ButtonRole)
    Q_ENUMS(ButtonBoxModel::ButtonRole)

    enum ButtonLayout {
        UnknownLayout = -1,
        WinLayout,
        MacLayout,
        LinuxLayout
    };
    Q_ENUM(ButtonLayout)
    Q_ENUMS(ButtonBoxModel::ButtonLayout)

    Q_INVOKABLE QList<int> load();
    Q_INVOKABLE void setButtons(const QVariantList& buttons);

    ButtonLayout buttonLayout() const;
    void setButtonLayout(ButtonLayout newButtonLayout);

    QQmlListProperty<QQuickItem> buttonsItems();

signals:
    void buttonLayoutChanged();
    void buttonsItemsChanged();

    void addButton(QString text, int buttonType, ButtonRole buttonRole, bool isAccent, bool isLeftSide);
    void reloadRequested();

private:
    const std::vector<ButtonRole> chooseButtonLayoutType();

    struct LayoutButton {
        QString text;
        int buttonType = int(ButtonType::NoButton);
        ButtonRole buttonRole = ButtonRole::CustomRole;
        bool isAccent = false;
        bool isLeftSide = false;

        LayoutButton(QString _text, int _buttonType,  ButtonRole _buttonRole, bool _isAccent, bool _isLeftSide = false)
            : text(_text), buttonType(_buttonType),  buttonRole(_buttonRole), isAccent(_isAccent), isLeftSide(_isLeftSide)
        {
        }
    };

    LayoutButton* layoutButton(const QQuickItem* item) const;

    const char* c = "uicomponents"; //To make Following Hash look neat

    QHash<ButtonType, LayoutButton*> m_layoutButtons {
        { Ok,              new LayoutButton(qtrc(c, "OK"),               Ok,              AcceptRole,      true) },
        { Save,            new LayoutButton(qtrc(c, "Save"),             Save,            ApplyRole,       true) },
        { SaveAll,         new LayoutButton(qtrc(c, "Save all"),         SaveAll,         ApplyRole,       false) },
        { DontSave,        new LayoutButton(qtrc(c, "Don't save"),       DontSave,        DestructiveRole, false) },
        { Open,            new LayoutButton(qtrc(c, "Open"),             Open,            AcceptRole,      true) },
        { Yes,             new LayoutButton(qtrc(c, "Yes"),              Yes,             AcceptRole,      true) },
        { YesToAll,        new LayoutButton(qtrc(c, "Yes to all"),       YesToAll,        AcceptRole,      false) },
        { No,              new LayoutButton(qtrc(c, "No"),               No,              RejectRole,      false) },
        { NoToAll,         new LayoutButton(qtrc(c, "No to all"),        NoToAll,         RejectRole,      false) },
        { Abort,           new LayoutButton(qtrc(c, "Abort"),            Abort,           RejectRole,      false) },
        { Retry,           new LayoutButton(qtrc(c, "Retry"),            Retry,           RetryRole,       false) },
        { Ignore,          new LayoutButton(qtrc(c, "Ignore"),           Ignore,          DestructiveRole, false) },
        { Close,           new LayoutButton(qtrc(c, "Close"),            Close,           RejectRole,      false) },
        { Cancel,          new LayoutButton(qtrc(c, "Cancel"),           Cancel,          RejectRole,      false) },
        { Discard,         new LayoutButton(qtrc(c, "Discard"),          Discard,         DestructiveRole, false) },
        { Help,            new LayoutButton(qtrc(c, "Help"),             Help,            HelpRole,        false) },
        { Apply,           new LayoutButton(qtrc(c, "Apply"),            Apply,           ApplyRole,       false) },
        { Reset,           new LayoutButton(qtrc(c, "Reset"),            Reset,           ResetRole,       false) },
        { RestoreDefaults, new LayoutButton(qtrc(c, "Restore defaults"), RestoreDefaults, ResetRole,       false) },
        { Continue,        new LayoutButton(qtrc(c, "Continue"),         Continue,        ContinueRole,    true) },
        { Next,            new LayoutButton(qtrc(c, "Next"),             Next,            ContinueRole,    false) },
        { Back,            new LayoutButton(qtrc(c, "Back"),             Back,            BackRole,        false) },
        { Select,          new LayoutButton(qtrc(c, "Select"),           Select,          AcceptRole,      true) },
        { Clear,           new LayoutButton(qtrc(c, "Clear"),            Clear,           DestructiveRole, false) },
        { Done,            new LayoutButton(qtrc(c, "Done"),             Done,            AcceptRole,      true) }
    };

    std::vector<std::vector <ButtonRole>> buttonRoleLayouts = {
        // WinLayout
        std::vector <ButtonRole> { CustomRole, ResetRole, RetryRole, BackRole, AcceptRole, ApplyRole, ContinueRole, DestructiveRole,
                                   RejectRole, HelpRole },

        // MacLayout
        std::vector <ButtonRole> { CustomRole, HelpRole, ResetRole, RetryRole, DestructiveRole, RejectRole, BackRole, AcceptRole,
                                   ApplyRole, ContinueRole },

        // KdeLayout (Linux / Unix Layout)
        std::vector <ButtonRole> { CustomRole, HelpRole, ResetRole, RetryRole, BackRole, AcceptRole, ApplyRole, ContinueRole,
                                   DestructiveRole, RejectRole }
    };

    ButtonLayout m_buttonLayout = ButtonLayout::UnknownLayout;
    QmlListProperty<QQuickItem> m_buttonsItems;
};
}

#endif // MU_UICOMPONENTS_BUTTONBOXMODEL_H
