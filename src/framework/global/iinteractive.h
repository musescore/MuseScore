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
#ifndef MUSE_GLOBAL_IINTERACTIVE_H
#define MUSE_GLOBAL_IINTERACTIVE_H

#include "modularity/imoduleinterface.h"
#include "io/path.h"
#include "types/val.h"
#include "types/retval.h"
#include "types/uri.h"
#include "types/flags.h"
#include "async/promise.h"
#include "progress.h"

namespace muse {
class IInteractive : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInteractive)

public:
    virtual ~IInteractive() = default;

    // question
    enum class Button {
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
    };
    using Buttons = std::vector<Button>;

    enum ButtonRole { // Keep updated with ButtonRole in buttonboxmodel.h
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

    struct ButtonData {
        int btn = int(Button::CustomButton);
        std::string text;
        bool accent = false;
        bool leftSide = false;
        ButtonRole role = ButtonRole::CustomRole;

        ButtonData(int btn, const std::string& text)
            : btn(btn), text(text) {}
        ButtonData(Button btn, const std::string& text)
            : btn(int(btn)), text(text) {}
        ButtonData(int btn, const std::string& text, bool accent, bool leftSide = false, ButtonRole role = ButtonRole::CustomRole)
            : btn(btn), text(text), accent(accent), leftSide(leftSide), role(role) {}
        ButtonData(Button btn, const std::string& text, bool accent, bool leftSide = false, ButtonRole role = ButtonRole::CustomRole)
            : btn(int(btn)), text(text), accent(accent), leftSide(leftSide), role(role) {}
    };
    using ButtonDatas = std::vector<ButtonData>;

    enum class TextFormat {
        Auto = 0,
        PlainText,
        RichText
    };

    struct Text {
        std::string text;
        TextFormat format = TextFormat::Auto;
        std::string detailedText;

        Text() = default;
        Text(const char* t)
            : text(t), format(TextFormat::Auto) {}
        Text(const std::string& t, const TextFormat& f = TextFormat::Auto)
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
        bool isButton(int b) const { return b == m_button; }
        bool isButton(Button b) const { return static_cast<int>(b) == m_button; }

        bool showAgain() const { return m_showAgain; }

    private:
        int m_button = int(Button::NoButton);
        bool m_showAgain = true;
    };

    enum Option {
        NoOptions = 0x0,
        WithIcon = 0x1,
        WithDontShowAgainCheckBox = 0x2
    };
    DECLARE_FLAGS(Options, Option)

    // buttons data
    virtual ButtonData buttonData(Button b) const = 0;
    inline ButtonDatas buttonDataList(const Buttons& buttons) const
    {
        ButtonDatas result;
        result.reserve(buttons.size());

        for (Button b : buttons) {
            result.push_back(buttonData(b));
        }

        return result;
    }

    // question
    virtual async::Promise<Result> question(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                            int defBtn = int(Button::NoButton), const Options& options = {},
                                            const std::string& dialogTitle = "") = 0;

    async::Promise<Result> question(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                    Button defBtn = Button::NoButton, const Options& options = {}, const std::string& dialogTitle = "")
    {
        return question(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    // info
    virtual async::Promise<Result> info(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                        int defBtn = int(Button::NoButton), const Options& options = {},
                                        const std::string& dialogTitle = "") = 0;

    async::Promise<Result> info(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                Button defBtn, const Options& options = {}, const std::string& dialogTitle = "")
    {
        return info(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    // warning
    virtual async::Promise<Result> warning(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                           int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                                           const std::string& dialogTitle = "") = 0;

    async::Promise<Result> warning(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                   Button defBtn = Button::NoButton, const Options& options = { WithIcon },
                                   const std::string& dialogTitle = "")
    {
        return warning(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    // error
    virtual async::Promise<Result> error(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                         int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                                         const std::string& dialogTitle = "") = 0;

    async::Promise<Result> error(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                 Button defBtn = Button::NoButton, const Options& options = { WithIcon },
                                 const std::string& dialogTitle = "")
    {
        return error(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    // progress
    virtual void showProgress(const std::string& title, Progress* progress) = 0;

    // files
    virtual async::Promise<io::path_t> selectOpeningFile(const std::string& title, const io::path_t& dir,
                                                         const std::vector<std::string>& filter) = 0;
    virtual io::path_t selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter) = 0;
    virtual io::path_t selectSavingFileSync(const std::string& title, const io::path_t& path, const std::vector<std::string>& filter,
                                            bool confirmOverwrite = true) = 0;

    // dirs
    virtual io::path_t selectDirectory(const std::string& title, const io::path_t& dir) = 0;
    virtual io::paths_t selectMultipleDirectories(const std::string& title, const io::path_t& dir,
                                                  const io::paths_t& selectedDirectories) = 0;

    // color
    virtual QColor selectColor(const QColor& color = Qt::white, const std::string& title = "") = 0;
    virtual bool isSelectColorOpened() const = 0;

    // custom
    virtual async::Promise<Val> open(const UriQuery& uri) = 0;
    async::Promise<Val> open(const std::string& uri) { return open(UriQuery(uri)); }
    async::Promise<Val> open(const Uri& uri) { return open(UriQuery(uri)); }
    virtual RetVal<bool> isOpened(const UriQuery& uri) const = 0;
    virtual RetVal<bool> isOpened(const Uri& uri) const = 0;
    virtual async::Channel<Uri> opened() const = 0;

    virtual void raise(const UriQuery& uri) = 0;

    virtual void close(const UriQuery& uri) = 0;
    virtual void close(const Uri& uri) = 0;
    virtual void closeAllDialogs() = 0;

    virtual ValCh<Uri> currentUri() const = 0;
    virtual RetVal<bool> isCurrentUriDialog() const = 0;
    virtual std::vector<Uri> stack() const = 0;

    virtual Ret openUrl(const std::string& url) const = 0;
    virtual Ret openUrl(const QUrl& url) const = 0;

    virtual Ret isAppExists(const std::string& appIdentifier) const = 0;
    virtual Ret canOpenApp(const Uri& uri) const = 0;
    virtual async::Promise<Ret> openApp(const Uri& uri) const = 0;

    /// Opens a file browser at the parent directory of filePath,
    /// and selects the file at filePath on OSs that support it
    virtual Ret revealInFileBrowser(const io::path_t& filePath) const = 0;

    //! =================================
    //! NOTE Please don't use this
    //! =================================
    virtual Result questionSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                int defBtn = int(Button::NoButton), const Options& options = {}, const std::string& dialogTitle = "") = 0;

    Result questionSync(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                        const Button& defBtn = Button::NoButton, const Options& options = {}, const std::string& dialogTitle = "")
    {
        return questionSync(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    virtual Result infoSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                            int defBtn = int(Button::NoButton), const Options& options = {}, const std::string& dialogTitle = "") = 0;

    Result infoSync(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                    const Button& defBtn = Button::NoButton, const Options& options = {}, const std::string& dialogTitle = "")
    {
        return infoSync(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    virtual Result warningSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                               int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                               const std::string& dialogTitle = "") = 0;

    Result warningSync(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                       const Button& defBtn = Button::NoButton, const Options& options = { WithIcon }, const std::string& dialogTitle = "")
    {
        return warningSync(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    virtual Result errorSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                             int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                             const std::string& dialogTitle = "") = 0;

    Result errorSync(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                     const Button& defBtn = Button::NoButton, const Options& options = { WithIcon }, const std::string& dialogTitle = "")
    {
        return errorSync(contentTitle, Text(text), buttonDataList(buttons), (int)defBtn, options, dialogTitle);
    }

    virtual RetVal<Val> openSync(const UriQuery& uri) = 0;
    RetVal<Val> openSync(const std::string& uri) { return openSync(UriQuery(uri)); }
    RetVal<Val> openSync(const Uri& uri) { return openSync(UriQuery(uri)); }
    //! ==============================
};
DECLARE_OPERATORS_FOR_FLAGS(IInteractive::Options)
}

#endif // MUSE_GLOBAL_IINTERACTIVE_H
