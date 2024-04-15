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
#ifndef MU_BRAILLE_LOUIS_H
#define MU_BRAILLE_LOUIS_H

#include <string>

extern std::string table_ascii_to_unicode;
extern std::string table_unicode_to_ascii;
extern std::string table_for_literature;
extern std::string table_for_general;
extern std::string tables_dir;

std::string get_louis_version();
std::string braille_translate(const char* table_name, std::string txt);
int check_tables(const char* tables);
char* setTablesDir(const char* tablesdir);
char* getTablesDir();

void initTables(std::string dir);
void updateTableForLyrics(std::string table);

std::string braille_long_translate(const char* table_name, std::string txt);
std::string braille_multi_line_translate(const char* table_name, std::string txt);
int get_braille_text_length(const char* table_name, std::string txt);

#endif // MU_BRAILLE_LOUIS_H
