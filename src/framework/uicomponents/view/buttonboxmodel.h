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
#include "translation.h"

namespace mu::uicomponents {
class ButtonBoxModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    Q_PROPERTY(ButtonLayout buttonLayout READ buttonLayout WRITE setButtonLayout NOTIFY buttonLayoutChanged)

public:
    explicit ButtonBoxModel(QObject* parent = nullptr);

    enum ButtonType {
        NoButton          = 0x0000000,
        Ok                = 0x0000001,
        Continue          = 0x0000002,
        RestoreDefaults   = 0x0000004,
        Reset             = 0x0000008,
        Apply             = 0x0000010,
        Help              = 0x0000020,
        Discard           = 0x0000040,
        Cancel            = 0x0000080,
        Close             = 0x0000100,
        Ignore            = 0x0000200,
        Retry             = 0x0000400,
        Abort             = 0x0000800,
        NoToAll           = 0x0001000,
        No                = 0x0002000,
        YesToAll          = 0x0004000,
        Yes               = 0x0008000,
        Open              = 0x0010000,
        DontSave          = 0x0020000,
        SaveAll           = 0x0040000,
        Save              = 0x0080000,
        Next              = 0x0100000,
        Back              = 0x0200000,
        Select            = 0x0400000,
        Clear             = 0x0800000,
        Done              = 0x1000000,

        CustomButton      = 0x2000000,

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

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int buttons() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void addCustomButton(int index, QString text, int role, bool isAccent, bool isLeftSide, QString navigationName);

    ButtonLayout buttonLayout() const;
    void setButtonLayout(ButtonLayout newButtonLayout);

public slots:
    void setButtons(const int& buttons);

signals:
    void buttonsChanged(const int& buttons);

    void buttonLayoutChanged();

private:
    enum Roles {
        ButtonText = Qt::UserRole + 1,
        Type,
        Accent,
        IsLeftSide,
        CustomButtonIndex,
        NavigationName,
        NavigationColumn
    };

    const std::vector<ButtonRole> chooseButtonLayoutType();

    struct LayoutButton {
        QString text;
        ButtonType buttonType;
        ButtonRole buttonRole;
        bool isAccent;
        bool isLeftSide = false;
        int customButtonIndex = -1;
        QString navigationName = "";
        int navigationColumn = 0;

        LayoutButton(QString _text, ButtonType _buttonType,  ButtonRole _buttonRole, bool _isAccent, bool _isLeftSide = false,
                     QString _navigationName = "")
            : text(_text), buttonType(_buttonType),  buttonRole(_buttonRole), isAccent(_isAccent), isLeftSide(_isLeftSide),
            navigationName(_navigationName)
        {
        }
    };

    const char* c = "uicomponents"; //To make Following Hash look neat

    QHash<ButtonType, LayoutButton*> m_layoutButtons {
        { Ok,              new LayoutButton(qtrc(c, "OK"),               Ok,              AcceptRole,      true) },
        { Save,            new LayoutButton(qtrc(c, "Save"),             Save,            ApplyRole,       true) },
        { SaveAll,         new LayoutButton(qtrc(c, "Save all"),         SaveAll,         ApplyRole,       false, false, "SaveAll") },
        { DontSave,        new LayoutButton(qtrc(c, "Don't save"),       DontSave,        DestructiveRole, false, false, "DontSave") },
        { Open,            new LayoutButton(qtrc(c, "Open"),             Open,            AcceptRole,      true) },
        { Yes,             new LayoutButton(qtrc(c, "Yes"),              Yes,             AcceptRole,      true) },
        { YesToAll,        new LayoutButton(qtrc(c, "Yes to all"),       YesToAll,        AcceptRole,      false, false, "YesToAll") },
        { No,              new LayoutButton(qtrc(c, "No"),               No,              RejectRole,      false) },
        { NoToAll,         new LayoutButton(qtrc(c, "No to all"),        NoToAll,         RejectRole,      false, false, "NoToAll") },
        { Abort,           new LayoutButton(qtrc(c, "Abort"),            Abort,           RejectRole,      false) },
        { Retry,           new LayoutButton(qtrc(c, "Retry"),            Retry,           RetryRole,       false) },
        { Ignore,          new LayoutButton(qtrc(c, "Ignore"),           Ignore,          DestructiveRole, false) },
        { Close,           new LayoutButton(qtrc(c, "Close"),            Close,           RejectRole,      false) },
        { Cancel,          new LayoutButton(qtrc(c, "Cancel"),           Cancel,          RejectRole,      false) },
        { Discard,         new LayoutButton(qtrc(c, "Discard"),          Discard,         DestructiveRole, false) },
        { Help,            new LayoutButton(qtrc(c, "Help"),             Help,            HelpRole,        false) },
        { Apply,           new LayoutButton(qtrc(c, "Apply"),            Apply,           ApplyRole,       false) },
        { Reset,           new LayoutButton(qtrc(c, "Reset"),            Reset,           ResetRole,       false, true) },
        { RestoreDefaults,
          new LayoutButton(qtrc(c, "Restore defaults"), RestoreDefaults, ResetRole,       false, true, "RestoreDefaults") },
        { Continue,        new LayoutButton(qtrc(c, "Continue"),         Continue,        ContinueRole,    true) },
        { Next,            new LayoutButton(qtrc(c, "Next"),             Next,            ContinueRole,    false) },
        { Back,            new LayoutButton(qtrc(c, "Back"),             Back,            BackRole,        false) },
        { Select,          new LayoutButton(qtrc(c, "Select"),           Select,          AcceptRole,      true) },
        { Clear,           new LayoutButton(qtrc(c, "Clear"),            Clear,           DestructiveRole, false, true) },
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

    QList<int> m_buttonsEnum; //Enum version of m_buttons for temp use
    QList<LayoutButton*> m_customButtons;

    QList<LayoutButton*> m_buttons;
    ButtonLayout m_buttonLayout = ButtonLayout::UnknownLayout;
};
}

#endif // MU_UICOMPONENTS_BUTTONBOXMODEL_H
