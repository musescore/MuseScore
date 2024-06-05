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
#include "htmlparser.h"

namespace mu::engraving {
muse::String HtmlParser::parse(const muse::String& in_html)
{
    if (in_html.isEmpty()) {
        return in_html;
    }

    std::string html = in_html.toStdString();

    //! NOTE Get body
    auto body_b = html.find("<body");
    auto body_e = html.find_last_of("</body>");
    if (body_b == std::string::npos || body_e == std::string::npos) {
        return in_html;
    }
    std::string body = html.substr(body_b, body_e - body_b);

    std::vector<std::string> blocks;

    //! NOTE Split blocks
    std::string::size_type p_b = 0;
    std::string::size_type p_e = 0;
    while (true) {
        p_b = body.find("<p", p_e);
        p_e = body.find("/p>", p_b);
        if (p_b == std::string::npos || p_e == std::string::npos) {
            break;
        }

        std::string block = body.substr(p_b, p_e - p_b);
        blocks.push_back(block);
    }

    if (blocks.empty()) {
        blocks.push_back(body);
    }

    //! NOTE Format blocks
    auto extractText = [](const std::string& block) {
        std::string text;
        bool isTag = false;
        for (const char& c : block) {
            if (c == '<') {
                isTag = true;
            } else if (c == '>') {
                isTag = false;
            } else if (!isTag) {
                text += c;
            }
        }
        return text;
    };

    auto extractFont = [](const std::string& block) {
        auto fontsize_b = block.find("font-size");
        if (fontsize_b != std::string::npos) {
            std::string fontSize;
            bool started = false;
            for (auto i = fontsize_b; i < block.size(); ++i) {
                const char& c = block.at(i);
                if (strchr(".0123456789", c) != nullptr) {
                    started = true;
                    fontSize += c;
                } else if (started) {
                    break;
                }
            }

            return std::string("<font size=\"") + fontSize + std::string("\"/>");
        }
        return std::string();
    };

    auto formatRichText = [extractText, extractFont](const std::string& block) {
        std::string text = extractText(block);
        std::string font = extractFont(block);
        if (!font.empty()) {
            text = font + text;
        }
        return text;
    };

    //! NOTE Format rich text from blocks
    std::string text;
    for (const std::string& block : blocks) {
        if (!text.empty()) {
            text += "\n";
        }

        text += formatRichText(block);
    }

    auto replaceSym = [](std::string& str, int cc, const char* sym) {
        std::string code;
        code.resize(3);
        code[2] = static_cast<char>(cc);
        code[1] = static_cast<char>(cc >> 8);
        code[0] = static_cast<char>(cc >> 16);

        auto pos = str.find(code);
        if (pos != std::string::npos) {
            str.replace(pos, 3, sym);
        }
    };

    //! NOTE replace utf8 code /*utf16 code*/ on sym
    replaceSym(text, 0xee848e /*0xe10e*/, "<sym>accidentalNatural</sym>");        //natural
    replaceSym(text, 0xee848c /*0xe10c*/, "<sym>accidentalSharp</sym>");          // sharp
    replaceSym(text, 0xee848d /*0xe10d*/, "<sym>accidentalFlat</sym>");           // flat
    replaceSym(text, 0xee8484 /*0xe104*/, "<sym>metNoteHalfUp</sym>");            // note2_Sym
    replaceSym(text, 0xee8485 /*0xe105*/, "<sym>metNoteQuarterUp</sym>");         // note4_Sym
    replaceSym(text, 0xee8486 /*0xe106*/, "<sym>metNote8thUp</sym>");             // note8_Sym
    replaceSym(text, 0xee8487 /*0xe107*/, "<sym>metNote16thUp</sym>");            // note16_Sym
    replaceSym(text, 0xee8488 /*0xe108*/, "<sym>metNote32ndUp</sym>");            // note32_Sym
    replaceSym(text, 0xee8489 /*0xe109*/, "<sym>metNote64thUp</sym>");            // note64_Sym
    replaceSym(text, 0xee848a /*0xe10a*/, "<sym>metAugmentationDot</sym>");       // dot
    replaceSym(text, 0xee848b /*0xe10b*/, "<sym>metAugmentationDot</sym><sym>space</sym><sym>metAugmentationDot</sym>");          // dotdot
    replaceSym(text, 0xee85a7 /*0xe167*/, "<sym>segno</sym>");                    // segno
    replaceSym(text, 0xee85a8 /*0xe168*/, "<sym>coda</sym>");                     // coda
    replaceSym(text, 0xee85a9 /*0xe169*/, "<sym>codaSquare</sym>");               // varcoda

    return muse::String::fromStdString(text);
}
}
