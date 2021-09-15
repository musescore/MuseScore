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
#ifndef MU_FRAMEWORK_IINTERACTIVE_H
#define MU_FRAMEWORK_IINTERACTIVE_H

#include <QString>

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "val.h"
#include "retval.h"
#include "uri.h"

namespace mu::framework {
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
        DontSave,
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
        bool accent = false;
        ButtonData(int btn, const std::string& text)
            : btn(btn), text(text) {}
        ButtonData(int btn, const std::string& text, bool accent)
            : btn(btn), text(text), accent(accent) {}
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

    struct Result
    {
        Result() = default;
        Result(const int& button)
            : m_button(button) {}
        Result(const int& button, bool showAgain)
            : m_button(button), m_showAgain(showAgain) {}

        Button standardButton() const { return static_cast<Button>(m_button); }
        int button() const { return m_button; }

        bool showAgain() const { return m_showAgain; }

    private:
        int m_button = int(Button::NoButton);
        bool m_showAgain = true;
    };

    enum Option {
        NoOptions = 0x0,
        WithIcon = 0x1,
        WithShowAgain = 0x2
    };
    Q_DECLARE_FLAGS(Options, Option)

    virtual Result question(const std::string& title, const std::string& text, const Buttons& buttons, const Button& def = Button::NoButton,
                            const Options& options = {}) const = 0;

    virtual Result question(const std::string& title, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                            const Options& options = {}) const = 0;

    virtual ButtonData buttonData(Button b) const = 0;

    // info
    virtual Result info(const std::string& title, const std::string& text, const ButtonDatas& buttons = {},
                        int defBtn = int(Button::NoButton), const Options& options = {}) const = 0;

    // warning
    virtual Result warning(const std::string& title, const std::string& text, const Buttons& buttons = {},
                           const Button& def = Button::NoButton, const Options& options = {}) const = 0;

    virtual Result warning(const std::string& title, const Text& text, const ButtonDatas& buttons = {}, int defBtn = int(Button::NoButton),
                           const Options& options = {}) const = 0;

    // error
    virtual Result error(const std::string& title, const std::string& text, const Buttons& buttons = {},
                         const Button& def = Button::NoButton, const Options& options = {}) const = 0;

    virtual Result error(const std::string& title, const Text& text, const ButtonDatas& buttons = {}, int defBtn = int(Button::NoButton),
                         const Options& options = {}) const = 0;

    // files
    virtual io::path selectOpeningFile(const QString& title, const io::path& dir, const QString& filter) = 0;
    virtual io::path selectSavingFile(const QString& title, const io::path& dir, const QString& filter, bool confirmOverwrite = true) = 0;

    // dirs
    virtual io::path selectDirectory(const QString& title, const io::path& dir) = 0;

    // custom
    virtual RetVal<Val> open(const std::string& uri) const = 0;
    virtual RetVal<Val> open(const UriQuery& uri) const = 0;
    virtual RetVal<bool> isOpened(const std::string& uri) const = 0;
    virtual RetVal<bool> isOpened(const Uri& uri) const = 0;

    virtual void close(const std::string& uri) = 0;
    virtual void close(const Uri& uri) = 0;

    virtual ValCh<Uri> currentUri() const = 0;
    virtual std::vector<Uri> stack() const = 0;

    virtual Ret openUrl(const std::string& url) const = 0;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(IInteractive::Options)
}

#endif // MU_FRAMEWORK_IINTERACTIVE_H
