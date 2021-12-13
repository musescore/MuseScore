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

#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <QString>

#include "braille/thirdparty/liblouis/liblouis/internal.h"
#include "braille/thirdparty/liblouis/liblouis/liblouis.h"

#define WIDECHARS_ARE_UCS4

#define FUNC u32_to_u8
#define SRC_UNIT uint32_t
#define DST_UNIT uint8_t

/* Type representing a Unicode character.  */
typedef uint32_t ucs4_t;

int
u8_uctomb(uint8_t* s, ucs4_t uc, int n)
{
    if (uc < 0x80) {
        if (n > 0) {
            s[0] = uc;
            return 1;
        }
        /* else return -2, below.  */
    } else {
        int count;

        if (uc < 0x800) {
            count = 2;
        } else if (uc < 0x10000) {
            if (uc < 0xd800 || uc >= 0xe000) {
                count = 3;
            } else {
                return -1;
            }
        } else if (uc < 0x110000) {
            count = 4;
        } else {
            return -1;
        }

        if (n >= count) {
            switch (count) { /* note: code falls through cases! */
            case 4: s[3] = 0x80 | (uc & 0x3f);
                uc = uc >> 6;
                uc |= 0x10000;
            // FALLTHROUGH
            case 3: s[2] = 0x80 | (uc & 0x3f);
                uc = uc >> 6;
                uc |= 0x800;
            // FALLTHROUGH
            case 2: s[1] = 0x80 | (uc & 0x3f);
                uc = uc >> 6;
                uc |= 0xc0;
                /*case 1:*/ s[0] = uc;
            }
            return count;
        }
    }
    return -2;
}

DST_UNIT*
FUNC(const SRC_UNIT* s, size_t n, DST_UNIT* resultbuf, size_t* lengthp)
{
    const SRC_UNIT* s_end = s + n;
    /* Output string accumulator.  */
    DST_UNIT* result;
    size_t allocated;
    size_t length;

    if (resultbuf != NULL) {
        result = resultbuf;
        allocated = *lengthp;
    } else {
        result = NULL;
        allocated = 0;
    }
    length = 0;
    /* Invariants:
       result is either == resultbuf or == NULL or malloc-allocated.
       If length > 0, then result != NULL.  */

    while (s < s_end)
    {
        ucs4_t uc;
        int count;

        /* Fetch a Unicode character from the input string.  */
        uc = *s++;
        /* No need to call the safe variant u32_mbtouc, because
           u8_uctomb will verify uc anyway.  */

        /* Store it in the output string.  */
        count = u8_uctomb(result + length, uc, static_cast<int>(allocated - length));
        if (count == -1) {
            if (!(result == resultbuf || result == NULL)) {
                free(result);
            }
            errno = EILSEQ;
            return NULL;
        }
        if (count == -2) {
            DST_UNIT* memory;

            allocated = (allocated > 0 ? 2 * allocated : 12);
            if (length + 6 > allocated) {
                allocated = length + 6;
            }
            if (result == resultbuf || result == NULL) {
                memory = (DST_UNIT*)malloc(allocated * sizeof(DST_UNIT));
            } else {
                memory
                    =(DST_UNIT*)realloc(result, allocated * sizeof(DST_UNIT));
            }

            if (memory == NULL) {
                if (!(result == resultbuf || result == NULL)) {
                    free(result);
                }
                errno = ENOMEM;
                return NULL;
            }
            if (result == resultbuf && length > 0) {
                memcpy((char*)memory, (char*)result,
                       length * sizeof(DST_UNIT));
            }
            result = memory;
            count = u8_uctomb(result + length, uc, static_cast<int>(allocated - length));
            if (count < 0) {
                abort();
            }
        }
        length += count;
    }

    if (length == 0) {
        if (result == NULL) {
            /* Return a non-NULL value.  NULL means error.  */
            result = (DST_UNIT*)malloc(1);
            if (result == NULL) {
                errno = ENOMEM;
                return NULL;
            }
        }
    } else if (result != resultbuf && length < allocated) {
        /* Shrink the allocated memory if possible.  */
        DST_UNIT* memory;

        memory = (DST_UNIT*)realloc(result, length * sizeof(DST_UNIT));
        if (memory != NULL) {
            result = memory;
        }
    }

    *lengthp = length;
    return result;
}

std::string table_unicode_to_ascii = "unicode-to-ascii.dis";
std::string table_ascii_to_unicode = "ascii-to-unicode.dis";
std::string table_for_literature = "unicode.dis,en-us-g2.ctb";
std::string table_for_general = "unicode.dis,en-us-symbols.mus";
std::string tables_dir = "";

void initTables(std::string dir)
{
    if (dir.empty()) {
        table_unicode_to_ascii = "unicode-to-ascii.dis";
        table_ascii_to_unicode = "ascii-to-unicode.dis";
        table_for_literature = "unicode.dis,en-us-g2.ctb";
        table_for_general = "unicode.dis,en-us-symbols.mus";
    } else {
        table_unicode_to_ascii = dir + "/unicode-to-ascii.dis";
        table_ascii_to_unicode = dir + "/ascii-to-unicode.dis";
        table_for_literature = dir + "/unicode.dis," + dir + "/en-us-g2.ctb";
        table_for_general = dir + "/unicode.dis," + dir + "/en-us-symbols.mus";
    }
    tables_dir = dir;
}

void updateTableForLyrics(std::string table)
{
    if (tables_dir.empty()) {
        table_for_literature = "unicode.dis," + table;
    } else {
        table_for_literature = tables_dir + "/unicode.dis," + tables_dir + "/" + table;
    }
}

std::string braille_translate(const char* table_name, std::string txt)
{
    uint8_t* outputbuf = nullptr;
    size_t outlen = 0;
    widechar inbuf[MAXSTRING];
    widechar transbuf[MAXSTRING];
    int inlen = 0;
    int translen = 0;

    inlen = _lou_extParseChars(txt.c_str(), inbuf);

    translen = MAXSTRING;
    lou_translateString(
        table_name, inbuf, &inlen, transbuf, &translen, NULL, NULL, 0);

#ifdef WIDECHARS_ARE_UCS4
    //outputbuf = (uint8_t *) malloc (translen * sizeof(widechar) * sizeof (uint8_t));
    outputbuf = u32_to_u8(transbuf, translen, NULL, &outlen);
#else
    outputbuf = u16_to_u8(transbuf, translen, NULL, &outlen);
#endif

    if (!outputbuf) {
        return std::string();
    }

    std::string ret = std::string(outputbuf, outputbuf + outlen);
    free(outputbuf);

    return ret;
}

int check_tables(const char* tables)
{
    if (lou_checkTable(tables) == 0) {
        return -1;
    } else {
        return 0;
    }
}

char* setTablesDir(const char* tablesdir)
{
    return lou_setDataPath(tablesdir);
}

char* getTablesDir()
{
    return lou_getDataPath();
}

std::vector<std::string> split_string(std::string txt, int width)
{
    std::vector<std::string> lines;

    QString str = QString::fromStdString(txt);

    int len = str.length();

    if (len <= width) {
        lines.push_back(txt);
        return lines;
    }

    while (len > width) {
        int idx = width - 1;
        for (; idx >= 0; idx--) {
            QString ch = str.left(idx).right(1);
            if (ch == "⠀") {
                break;
            }
        }
        if (idx == 0 || idx == -1) {
            idx = width;
        }

        QString line = str.left(idx);

        str = str.right(len - idx);
        len = str.length();
        lines.push_back(line.toStdString());
    }
    if (len > 0) {
        lines.push_back(str.toStdString());
    }
    return lines;
}

std::string braille_long_translate(const char* table_name, std::string txt)
{
    std::vector<std::string> lines = split_string(txt, 256);

    if (lines.size() == 0) {
        return "";
    }

    std::string buffer = braille_translate(table_name, lines.front());

    for (size_t i = 1; i < lines.size(); i++) {
        std::string text = lines[i] + " ";
        buffer.append(braille_translate(table_name, text));
    }
    return buffer;
}

std::string braille_multi_line_translate(const char* table_name, std::string txt)
{
    std::stringstream ss(txt);
    std::string line;

    std::string buffer = "";

    while (std::getline(ss, line)) {
        if (line == "[EOP]" || line == "[NP]") {
            buffer.append(line).append("\n");
        } else {
            std::string converted = braille_translate(table_name, line);
            buffer.append(converted).append("\n");
        }
    }

    return buffer;
}

int get_braille_text_length(const char* table_name, std::string txt)
{
    std::string buffer = braille_translate(table_name, txt);
    return QString::fromStdString(txt).length();
}
