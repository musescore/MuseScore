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
#ifndef MU_FRAMEWORK_INTERACTIVE_H
#define MU_FRAMEWORK_INTERACTIVE_H

#include "iinteractive.h"

namespace mu {
namespace framework {
class Interactive : public IInteractive
{
public:

    Interactive() = default;

    Button question(const std::string& title, const std::string& text, const Buttons& buttons,
                    const Button& def = Button::NoButton) const override;

    int /*button*/ question(const std::string& title, const Text& text, const ButtonDatas& buttons,
                            int defBtn = int(Button::NoButton)) const override;

    ButtonData buttonData(Button b) const override;

    void message(Type type, const std::string& title, const std::string& text) const override;

    io::path selectOpeningFile(const std::string& title, const std::string& dir, const std::string& filter) override;
};
}
}

#endif // MU_FRAMEWORK_UIINTERACTIVE_H
