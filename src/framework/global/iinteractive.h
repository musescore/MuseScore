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
#ifndef MU_FRAMEWORK_IINTERACTIVE_H
#define MU_FRAMEWORK_IINTERACTIVE_H

#include <QString>

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "val.h"
#include "retval.h"
#include "uri.h"

namespace mu {
namespace framework {
class IInteractive : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInteractive)

public:
    virtual ~IInteractive() = default;

    // question
    enum class Button {
        NoButton,
        Ok,
        Save,
        SaveAll,
        Open,
        Yes,
        YesToAll,
        No,
        NoToAll,
        Abort,
        Retry,
        Ignore,
        Close,
        Cancel,
        Discard,
        Help,
        Apply,
        Reset,
        RestoreDefaults,

        CustomButton
    };
    using Buttons = std::vector<Button>;

    struct ButtonData {
        int btn = int(Button::CustomButton);
        std::string text;
        ButtonData(int b, const std::string& t)
            : btn(b), text(t) {}
    };
    using ButtonDatas = std::vector<ButtonData>;

    enum class TextFormat {
        PlainText = 0,
        RichText
    };

    struct Text {
        std::string text;
        TextFormat format = TextFormat::PlainText;
        Text() = default;
        Text(const char* t)
            : text(t), format(TextFormat::PlainText) {}
        Text(const std::string& t, const TextFormat& f = TextFormat::PlainText)
            : text(t), format(f) {}
    };

    virtual Button question(const std::string& title, const std::string& text, const Buttons& buttons,
                            const Button& def = Button::NoButton) const = 0;

    virtual int /*button*/ question(const std::string& title, const Text& text, const ButtonDatas& buttons,
                                    int defBtn = int(Button::NoButton)) const = 0;

    virtual ButtonData buttonData(Button b) const = 0;

    // message
    enum class Type {
        Info,
        Warning,
        Critical
    };
    virtual void message(Type type, const std::string& title, const std::string& text) const = 0;

    // files
    virtual io::path selectOpeningFile(const QString& title, const io::path& dir, const QString& filter) = 0;
    virtual io::path selectSavingFile(const QString& title, const io::path& dir, const QString& filter) = 0;

    // dirs
    virtual io::path selectDirectory(const QString& title, const io::path& dir) = 0;

    // custom
    virtual RetVal<Val> open(const std::string& uri) const = 0;
    virtual RetVal<Val> open(const UriQuery& uri) const = 0;
    virtual RetVal<bool> isOpened(const std::string& uri) const = 0;

    virtual void close(const std::string& uri) = 0;

    virtual ValCh<Uri> currentUri() const = 0;

    virtual Ret openUrl(const std::string& url) const = 0;
};
}
}

#endif // MU_FRAMEWORK_IINTERACTIVE_H
