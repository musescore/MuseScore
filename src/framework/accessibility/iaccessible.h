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
#ifndef MU_ACCESSIBILITY_IACCESSIBLE_H
#define MU_ACCESSIBILITY_IACCESSIBLE_H

#include <QString>
#include <QRect>
#include "async/notification.h"

class QWindow;

namespace mu::accessibility {
class IAccessible
{
public:

    //! NOTE Must be sync with QAccessible::Role
    enum Role {
        NoRole         = 0x00000000,
        TitleBar       = 0x00000001,
        MenuBar        = 0x00000002,
        ScrollBar      = 0x00000003,
        Grip           = 0x00000004,
        Sound          = 0x00000005,
        Cursor         = 0x00000006,
        Caret          = 0x00000007,
        AlertMessage   = 0x00000008,
        Window         = 0x00000009,
        Client         = 0x0000000A,
        PopupMenu      = 0x0000000B,
        MenuItem       = 0x0000000C,
        ToolTip        = 0x0000000D,
        Application    = 0x0000000E,
        Document       = 0x0000000F,
        Pane           = 0x00000010,
        Chart          = 0x00000011,
        Dialog         = 0x00000012,
        Border         = 0x00000013,
        Grouping       = 0x00000014,
        Separator      = 0x00000015,
        ToolBar        = 0x00000016,
        StatusBar      = 0x00000017,
        Table          = 0x00000018,
        ColumnHeader   = 0x00000019,
        RowHeader      = 0x0000001A,
        Column         = 0x0000001B,
        Row            = 0x0000001C,
        Cell           = 0x0000001D,
        Link           = 0x0000001E,
        HelpBalloon    = 0x0000001F,
        Assistant      = 0x00000020,
        List           = 0x00000021,
        ListItem       = 0x00000022,
        Tree           = 0x00000023,
        TreeItem       = 0x00000024,
        PageTab        = 0x00000025,
        PropertyPage   = 0x00000026,
        Indicator      = 0x00000027,
        Graphic        = 0x00000028,
        StaticText     = 0x00000029,
        EditableText   = 0x0000002A,  // Editable, selectable, etc.
        Button         = 0x0000002B,
#ifndef Q_QDOC
        PushButton     = Button, // deprecated
#endif
        CheckBox       = 0x0000002C,
        RadioButton    = 0x0000002D,
        ComboBox       = 0x0000002E,
        // DropList       = 0x0000002F,
        ProgressBar    = 0x00000030,
        Dial           = 0x00000031,
        HotkeyField    = 0x00000032,
        Slider         = 0x00000033,
        SpinBox        = 0x00000034,
        Canvas         = 0x00000035, // MSAA: ROLE_SYSTEM_DIAGRAM - The object represents a graphical image that is used to diagram data.
        Animation      = 0x00000036,
        Equation       = 0x00000037,
        ButtonDropDown = 0x00000038, // The object represents a button that expands a grid.
        ButtonMenu     = 0x00000039,
        ButtonDropGrid = 0x0000003A,
        Whitespace     = 0x0000003B, // The object represents blank space between other objects.
        PageTabList    = 0x0000003C,
        Clock          = 0x0000003D,
        Splitter       = 0x0000003E,
        // Reserved space in case MSAA roles needs to be added

        // Additional Qt roles where enum value does not map directly to MSAA:
        LayeredPane    = 0x00000080,
        Terminal       = 0x00000081,
        Desktop        = 0x00000082,
        Paragraph      = 0x00000083,
        WebDocument    = 0x00000084,
        Section        = 0x00000085,
        Notification   = 0x00000086,

        // IAccessible2 roles
        // IA2_ROLE_CANVAS = 0x401, // An object that can be drawn into and to manage events from the objects drawn into it
        // IA2_ROLE_CAPTION = 0x402,
        // IA2_ROLE_CHECK_MENU_ITEM = 0x403,
        ColorChooser = 0x404,
        // IA2_ROLE_DATE_EDITOR = 0x405,
        // IA2_ROLE_DESKTOP_ICON = 0x406,
        // IA2_ROLE_DESKTOP_PANE = 0x407,
        // IA2_ROLE_DIRECTORY_PANE = 0x408,
        // IA2_ROLE_EDITBAR = 0x409,
        // IA2_ROLE_EMBEDDED_OBJECT = 0x40A,
        // IA2_ROLE_ENDNOTE = 0x40B,
        // IA2_ROLE_FILE_CHOOSER = 0x40C,
        // IA2_ROLE_FONT_CHOOSER = 0x40D,
        Footer      = 0x40E,
        // IA2_ROLE_FOOTNOTE = 0x40F,
        Form        = 0x410,
        // some platforms (windows and at-spi) use Frame for regular windows
        // because window was taken for tool/dock windows by MSAA
        // Frame = 0x411,
        // IA2_ROLE_GLASS_PANE = 0x412,
        // IA2_ROLE_HEADER = 0x413,
        Heading  = 0x414,
        // IA2_ROLE_ICON = 0x415,
        // IA2_ROLE_IMAGE_MAP = 0x416,
        // IA2_ROLE_INPUT_METHOD_WINDOW = 0x417,
        // IA2_ROLE_INTERNAL_FRAME = 0x418,
        // IA2_ROLE_LABEL = 0x419,
        // IA2_ROLE_LAYERED_PANE = 0x41A,
        Note = 0x41B,
        // IA2_ROLE_OPTION_PANE = 0x41C,
        // IA2_ROLE_PAGE = 0x41D,
        // IA2_ROLE_PARAGRAPH = 0x42E,
        // IA2_ROLE_RADIO_MENU_ITEM = 0x41F,
        // IA2_ROLE_REDUNDANT_OBJECT = 0x420,
        // IA2_ROLE_ROOT_PANE = 0x421,
        // IA2_ROLE_RULER = 0x422,
        // IA2_ROLE_SCROLL_PANE = 0x423,
        // IA2_ROLE_SECTION = 0x424,
        // IA2_ROLE_SHAPE = 0x425,
        // IA2_ROLE_SPLIT_PANE = 0x426,
        // IA2_ROLE_TEAR_OFF_MENU = 0x427,
        // IA2_ROLE_TERMINAL = 0x428,
        // IA2_ROLE_TEXT_FRAME = 0x429,
        // IA2_ROLE_TOGGLE_BUTTON = 0x42A,
        // IA2_ROLE_VIEW_PORT = 0x42B,
        ComplementaryContent = 0x42C,

        UserRole       = 0x0000ffff
    };

    enum class State {
        Undefined = 0,
        Enabled,
        Active,
        Focused,
        Selected
    };

    virtual const IAccessible* accessibleParent() const = 0;
    virtual async::Notification accessibleParentChanged() const = 0;
    virtual size_t accessibleChildCount() const = 0;
    virtual const IAccessible* accessibleChild(size_t i) const = 0;

    virtual Role accessibleRole() const = 0;
    virtual QString accessibleName() const = 0;
    virtual bool accessibleState(State st) const = 0;
    virtual QRect accessibleRect() const = 0;
    virtual QWindow* accessibleWindow() const = 0;
};
}

#endif // MU_ACCESSIBILITY_IACCESSIBLE_H
