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
#include <vector>

#include "braillecode.h"

#include "log.h"

namespace mu::engraving {
std::vector<braille_code*> master_braille_code_list;

braille_code::braille_code(std::string t, std::string c)
{
    tag = t;
    code = c;
    braille = translate2Braille(code);
    num_cells = static_cast<int>(braille.length());
    master_braille_code_list.push_back(this);
}

braille_code::~braille_code()
{
    tag.clear();
    code.clear();
    braille.clear();
}

void braille_code::print()
{
    LOGD() << " Braille code " << tag << " " << code << " " << braille << " " << num_cells;
}

braille_code Braille_CapIndicator = braille_code("CapIndicator", "6");
braille_code Braille_NumIndicator = braille_code("NumIndicator", "3456");
braille_code Braille_TxtIndicator = braille_code("TextIndicator", "345");

//[Letter]
braille_code Braille_a = braille_code("a", "1");
braille_code Braille_b = braille_code("b", "12");
braille_code Braille_c = braille_code("c", "14");
braille_code Braille_d = braille_code("d", "145");
braille_code Braille_e = braille_code("e", "15");
braille_code Braille_f = braille_code("f", "124");
braille_code Braille_g = braille_code("g", "1245");
braille_code Braille_h = braille_code("h", "125");
braille_code Braille_i = braille_code("i", "24");
braille_code Braille_j = braille_code("j", "245");
braille_code Braille_k = braille_code("k", "13");
braille_code Braille_l = braille_code("l", "123");
braille_code Braille_m = braille_code("m", "134");
braille_code Braille_n = braille_code("n", "1345");
braille_code Braille_o = braille_code("o", "135");
braille_code Braille_p = braille_code("p", "1234");
braille_code Braille_q = braille_code("q", "12345");
braille_code Braille_r = braille_code("r", "1235");
braille_code Braille_s = braille_code("s", "234");
braille_code Braille_t = braille_code("t", "2345");
braille_code Braille_u = braille_code("u", "136");
braille_code Braille_v = braille_code("v", "1236");
braille_code Braille_w = braille_code("w", "2456");
braille_code Braille_x = braille_code("x", "1346");
braille_code Braille_y = braille_code("y", "13456");
braille_code Braille_z = braille_code("z", "1356");

braille_code* Braille_Letters[] = {
    &Braille_a, &Braille_b, &Braille_c, &Braille_d, &Braille_e,
    &Braille_f, &Braille_g, &Braille_h, &Braille_i, &Braille_j,
    &Braille_k, &Braille_l, &Braille_m, &Braille_n, &Braille_o,
    &Braille_p, &Braille_q, &Braille_r, &Braille_s, &Braille_t,
    &Braille_u, &Braille_v, &Braille_w, &Braille_x, &Braille_y,
    &Braille_z };

//upper and lower numbers
braille_code Braille_Upper0 = braille_code("Upper0", "245");
braille_code Braille_Upper1 = braille_code("Upper1", "1");
braille_code Braille_Upper2 = braille_code("Upper2", "12");
braille_code Braille_Upper3 = braille_code("Upper3", "14");
braille_code Braille_Upper4 = braille_code("Upper4", "145");
braille_code Braille_Upper5 = braille_code("Upper5", "15");
braille_code Braille_Upper6 = braille_code("Upper6", "124");
braille_code Braille_Upper7 = braille_code("Upper7", "1245");
braille_code Braille_Upper8 = braille_code("Upper8", "125");
braille_code Braille_Upper9 = braille_code("Upper9", "24");

braille_code* Braille_UpperNumbers[] = {
    &Braille_Upper0, &Braille_Upper1, &Braille_Upper2, &Braille_Upper3,
    &Braille_Upper4, &Braille_Upper5, &Braille_Upper6, &Braille_Upper7,
    &Braille_Upper8, &Braille_Upper9, };

//#lower numbers
braille_code Braille_Lower0 = braille_code("Lower0", "356");
braille_code Braille_Lower1 = braille_code("Lower1", "2");
braille_code Braille_Lower2 = braille_code("Lower2", "23");
braille_code Braille_Lower3 = braille_code("Lower3", "25");
braille_code Braille_Lower4 = braille_code("Lower4", "256");
braille_code Braille_Lower5 = braille_code("Lower5", "26");
braille_code Braille_Lower6 = braille_code("Lower6", "235");
braille_code Braille_Lower7 = braille_code("Lower7", "2356");
braille_code Braille_Lower8 = braille_code("Lower8", "236");
braille_code Braille_Lower9 = braille_code("Lower9", "35");

braille_code* Braille_LowerNumbers[] ={
    &Braille_Lower0, &Braille_Lower1, &Braille_Lower2, &Braille_Lower3,
    &Braille_Lower4, &Braille_Lower5, &Braille_Lower6, &Braille_Lower7,
    &Braille_Lower8, &Braille_Lower9 };

//[NoteShape]
//#Notes: Braille note shapes are same such as whole=16th=256, half=32nd=512 etc.
//#to deal with complicate back-translation and reading issues, each note should be defined, even it could easily handle by coding.
braille_code Braille_aWhole = braille_code("aWhole", "2346");
braille_code Braille_aHalf = braille_code("aHalf", "234");
braille_code Braille_aQuarter = braille_code("aQuarter", "246");
braille_code Braille_a8th = braille_code("a8th", "24");
braille_code Braille_bWhole = braille_code("bWhole", "23456");
braille_code Braille_bHalf = braille_code("bHalf", "2345");
braille_code Braille_bQuarter = braille_code("bQuarter", "2456");
braille_code Braille_b8th = braille_code("b8th", "245");
braille_code Braille_cWhole = braille_code("cWhole", "13456");
braille_code Braille_cHalf = braille_code("cHalf", "1345");
braille_code Braille_cQuarter = braille_code("cQuarter", "1456");
braille_code Braille_c8th = braille_code("c8th", "145");
braille_code Braille_dWhole = braille_code("dWhole", "1356");
braille_code Braille_dHalf = braille_code("dHalf", "135");
braille_code Braille_dQuarter = braille_code("dQuarter", "156");
braille_code Braille_d8th = braille_code("d8th", "15");
braille_code Braille_eWhole = braille_code("eWhole", "12346");
braille_code Braille_eHalf = braille_code("eHalf", "1234");
braille_code Braille_eQuarter = braille_code("eQuarter", "1246");
braille_code Braille_e8th = braille_code("e8th", "124");
braille_code Braille_fWhole = braille_code("fWhole", "123456");
braille_code Braille_fHalf = braille_code("fHalf", "12345");
braille_code Braille_fQuarter = braille_code("fQuarter", "12456");
braille_code Braille_f8th = braille_code("f8th", "1245");
braille_code Braille_gWhole = braille_code("gWhole", "12356");
braille_code Braille_gHalf = braille_code("gHalf", "1235");
braille_code Braille_gQuarter = braille_code("gQuarter", "1256");
braille_code Braille_g8th = braille_code("g8th", "125");
//#16th to 128th notes
braille_code Braille_a16th = braille_code("a16th", "2346");
braille_code Braille_a32nd = braille_code("a32nd", "234");
braille_code Braille_a64th = braille_code("a64th", "246");
braille_code Braille_a128th = braille_code("a128th", "24");
braille_code Braille_b16th = braille_code("b16th", "23456");
braille_code Braille_b32nd = braille_code("b32nd", "2345");
braille_code Braille_b64th = braille_code("b64th", "2456");
braille_code Braille_b128th = braille_code("b128th", "245");
braille_code Braille_c16th = braille_code("c16th", "13456");
braille_code Braille_c32nd = braille_code("c32nd", "1345");
braille_code Braille_c64th = braille_code("c64th", "1456");
braille_code Braille_c128th = braille_code("c128th", "145");
braille_code Braille_d16th = braille_code("d16th", "1356");
braille_code Braille_d32nd = braille_code("d32nd", "135");
braille_code Braille_d64th = braille_code("d64th", "156");
braille_code Braille_d128th = braille_code("d128th", "15");
braille_code Braille_e16th = braille_code("e16th", "12346");
braille_code Braille_e32nd = braille_code("e32nd", "1234");
braille_code Braille_e64th = braille_code("e64th", "1246");
braille_code Braille_e128th = braille_code("e128th", "124");
braille_code Braille_f16th = braille_code("f16th", "123456");
braille_code Braille_f32nd = braille_code("f32nd", "12345");
braille_code Braille_f64th = braille_code("f64th", "12456");
braille_code Braille_f128th = braille_code("f128th", "1245");
braille_code Braille_g16th = braille_code("g16th", "12356");
braille_code Braille_g32nd = braille_code("g32nd", "1235");
braille_code Braille_g64th = braille_code("g64th", "1256");
braille_code Braille_g128th = braille_code("g128th", "125");
//#256th to 2048th notes
braille_code Braille_a256th = braille_code("a256th", "2346");
braille_code Braille_a512th = braille_code("a512th", "234");
braille_code Braille_a1024th = braille_code("a1024th", "246");
braille_code Braille_a2048th = braille_code("a2048th", "24");
braille_code Braille_b256th = braille_code("b256th", "23456");
braille_code Braille_b512th = braille_code("b512th", "2345");
braille_code Braille_b1024th = braille_code("b1024th", "2456");
braille_code Braille_b2048th = braille_code("b2048th", "245");
braille_code Braille_c256th = braille_code("c256th", "13456");
braille_code Braille_c512th = braille_code("c512th", "1345");
braille_code Braille_c1024th = braille_code("c1024th", "1456");
braille_code Braille_c2048th = braille_code("c2048th", "145");
braille_code Braille_d256th = braille_code("d256th", "1356");
braille_code Braille_d512th = braille_code("d512th", "135");
braille_code Braille_d1024th = braille_code("d1024th", "156");
braille_code Braille_d2048th = braille_code("d2048th", "15");
braille_code Braille_e256th = braille_code("e256th", "12346");
braille_code Braille_e512th = braille_code("e512th", "1234");
braille_code Braille_e1024th = braille_code("e1024th", "1246");
braille_code Braille_e2048th = braille_code("e2048th", "124");
braille_code Braille_f256th = braille_code("f256th", "123456");
braille_code Braille_f512th = braille_code("f512th", "12345");
braille_code Braille_f1024th = braille_code("f1024th", "12456");
braille_code Braille_f2048th = braille_code("f2048th", "1245");
braille_code Braille_g256th = braille_code("g256th", "12356");
braille_code Braille_g512th = braille_code("g512th", "1235");
braille_code Braille_g1024th = braille_code("g1024th", "1256");
braille_code Braille_g2048th = braille_code("g2048th", "125");
//#special notes: breve (double whole notes)
//#Advised to use the definitions of Haipeng as first option for breve notes right below this section.
braille_code Braille_aBreve = braille_code("aBreve", "2346-13");
braille_code Braille_bBreve = braille_code("bBreve", "23456-13");
braille_code Braille_cBreve = braille_code("cBreve", "13456-13");
braille_code Braille_dBreve = braille_code("dBreve", "1356-13");
braille_code Braille_eBreve = braille_code("eBreve", "12346-13");
braille_code Braille_fBreve = braille_code("fBreve", "123456-13");
braille_code Braille_gBreve = braille_code("gBreve", "12356-13");
//##########Added by Haipeng
//#Another kind of breve, mainly used other than the above one which should be used mainly in plain chant
braille_code Braille_aBreveAlt = braille_code("aBreveAlt", "2346-45-14-2346");
braille_code Braille_bBreveAlt = braille_code("bBreveAlt", "23456-45-14-23456");
braille_code Braille_cBreveAlt = braille_code("cBreveAlt", "13456-45-14-13456");
braille_code Braille_dBreveAlt = braille_code("dBreveAlt", "1356-45-14-1356");
braille_code Braille_eBreveAlt = braille_code("eBreveAlt", "12346-45-14-12346");
braille_code Braille_fBreveAlt = braille_code("fBreveAlt", "123456-45-14-123456");
braille_code Braille_gBreveAlt = braille_code("gBreveAlt", "12356-45-14-12356");
//##Other longer notes in ancient music, including Longa and Maxima (quadruple whole note)
braille_code Braille_aLonga = braille_code("aLonga", "2346-45-14-45-14-2346");
braille_code Braille_bLonga = braille_code("bLonga", "23456-45-14-45-14-23456");
braille_code Braille_cLonga = braille_code("cLonga", "13456-45-14-45-14-13456");
braille_code Braille_dLonga = braille_code("dLonga", "1356-45-14-45-14-1356");
braille_code Braille_eLonga = braille_code("eLonga", "12346-45-14-45-14-12346");
braille_code Braille_fLonga = braille_code("fLonga", "123456-45-14-45-14-123456");
braille_code Braille_gLonga = braille_code("gLonga", "12356-45-14-45-14-12356");
braille_code Braille_aMaxima = braille_code("aMaxima", "2346-45-14-45-14-45-14-2346");
braille_code Braille_bMaxima = braille_code("bMaxima", "23456-45-14-45-14-45-14-23456");
braille_code Braille_cMaxima = braille_code("cMaxima", "13456-45-14-45-14-45-14-13456");
braille_code Braille_dMaxima = braille_code("dMaxima", "1356-45-14-45-14-45-14-1356");
braille_code Braille_eMaxima = braille_code("eMaxima", "12346-45-14-45-14-45-14-12346");
braille_code Braille_fMaxima = braille_code("fMaxima", "123456-45-14-45-14-45-14-123456");
braille_code Braille_gMaxima = braille_code("gMaxima", "12356-45-14-45-14-45-14-12356");

braille_code* Braille_aNotes[] = {
    &Braille_aMaxima, &Braille_aLonga, &Braille_aBreve,
    &Braille_aWhole, &Braille_aHalf, &Braille_aQuarter,
    &Braille_a8th, &Braille_a16th, &Braille_a32nd,
    &Braille_a64th, &Braille_a128th, &Braille_a256th,
    &Braille_a512th, &Braille_a128th, &Braille_a2048th,
    &Braille_aBreveAlt };
braille_code* Braille_bNotes[] = {
    &Braille_bMaxima, &Braille_bLonga, &Braille_bBreve,
    &Braille_bWhole, &Braille_bHalf, &Braille_bQuarter,
    &Braille_b8th, &Braille_b16th, &Braille_b32nd,
    &Braille_b64th, &Braille_b128th, &Braille_b256th,
    &Braille_b512th, &Braille_b128th, &Braille_b2048th,
    &Braille_bBreveAlt };
braille_code* Braille_cNotes[] = {
    &Braille_cMaxima, &Braille_cLonga, &Braille_cBreve,
    &Braille_cWhole, &Braille_cHalf, &Braille_cQuarter,
    &Braille_c8th, &Braille_c16th, &Braille_c32nd,
    &Braille_c64th, &Braille_c128th, &Braille_c256th,
    &Braille_c512th, &Braille_c128th, &Braille_c2048th,
    &Braille_cBreveAlt };
braille_code* Braille_dNotes[] = {
    &Braille_dMaxima, &Braille_dLonga, &Braille_dBreve,
    &Braille_dWhole, &Braille_dHalf, &Braille_dQuarter,
    &Braille_d8th, &Braille_d16th, &Braille_d32nd,
    &Braille_d64th, &Braille_d128th, &Braille_d256th,
    &Braille_d512th, &Braille_d128th, &Braille_d2048th,
    &Braille_dBreveAlt };
braille_code* Braille_eNotes[] = {
    &Braille_eMaxima, &Braille_eLonga, &Braille_eBreve,
    &Braille_eWhole, &Braille_eHalf, &Braille_eQuarter,
    &Braille_e8th, &Braille_e16th, &Braille_e32nd,
    &Braille_e64th, &Braille_e128th, &Braille_e256th,
    &Braille_e512th, &Braille_e128th, &Braille_e2048th,
    &Braille_eBreveAlt };
braille_code* Braille_fNotes[] = {
    &Braille_fMaxima, &Braille_fLonga, &Braille_fBreve,
    &Braille_fWhole, &Braille_fHalf, &Braille_fQuarter,
    &Braille_f8th, &Braille_f16th, &Braille_f32nd,
    &Braille_f64th, &Braille_f128th, &Braille_f256th,
    &Braille_f512th, &Braille_f128th, &Braille_f2048th,
    &Braille_fBreveAlt };
braille_code* Braille_gNotes[] = {
    &Braille_gMaxima, &Braille_gLonga, &Braille_gBreve,
    &Braille_gWhole, &Braille_gHalf, &Braille_gQuarter,
    &Braille_g8th, &Braille_g16th, &Braille_g32nd,
    &Braille_g64th, &Braille_g128th, &Braille_g256th,
    &Braille_g512th, &Braille_g128th, &Braille_g2048th,
    &Braille_gBreveAlt };

braille_code* Braille_wholeNotes[] = {
    &Braille_aWhole, &Braille_bWhole, &Braille_cWhole,
    &Braille_dWhole, &Braille_eWhole, &Braille_fWhole, &Braille_gWhole
};
braille_code* Braille_halfNotes[] = {
    &Braille_aHalf, &Braille_bHalf, &Braille_cHalf,
    &Braille_dHalf, &Braille_eHalf, &Braille_fHalf, &Braille_gHalf
};
braille_code* Braille_quarterNotes[] = {
    &Braille_aQuarter, &Braille_bQuarter, &Braille_cQuarter,
    &Braille_dQuarter, &Braille_eQuarter, &Braille_fQuarter, &Braille_gQuarter
};
braille_code* Braille_8thNotes[] = {
    &Braille_a8th, &Braille_b8th, &Braille_c8th,
    &Braille_d8th, &Braille_e8th, &Braille_f8th, &Braille_g8th
};
braille_code* Braille_16thNotes[] = {
    &Braille_a16th, &Braille_b16th, &Braille_c16th,
    &Braille_d16th, &Braille_e16th, &Braille_f16th, &Braille_g16th
};
braille_code* Braille_32ndNotes[] = {
    &Braille_a32nd, &Braille_b32nd, &Braille_c32nd,
    &Braille_d32nd, &Braille_e32nd, &Braille_f32nd, &Braille_g32nd
};
braille_code* Braille_64thNotes[] = {
    &Braille_a64th, &Braille_b64th, &Braille_c64th,
    &Braille_d64th, &Braille_e64th, &Braille_f64th, &Braille_g64th
};
braille_code* Braille_128thNotes[] = {
    &Braille_a128th, &Braille_b128th, &Braille_c128th,
    &Braille_d128th, &Braille_e128th, &Braille_f128th, &Braille_g128th
};
braille_code* Braille_256thNotes[] = {
    &Braille_a256th, &Braille_b256th, &Braille_c256th,
    &Braille_d256th, &Braille_e256th, &Braille_f256th, &Braille_g256th
};
braille_code* Braille_512thNotes[] = {
    &Braille_a512th, &Braille_b512th, &Braille_c512th,
    &Braille_d512th, &Braille_e512th, &Braille_f512th, &Braille_g512th
};
braille_code* Braille_1024thNotes[] = {
    &Braille_a1024th, &Braille_b1024th, &Braille_c1024th,
    &Braille_d1024th, &Braille_e1024th, &Braille_f1024th, &Braille_g1024th
};
braille_code* Braille_2048thNotes[] = {
    &Braille_a2048th, &Braille_b2048th, &Braille_c2048th,
    &Braille_d2048th, &Braille_e2048th, &Braille_f2048th, &Braille_g2048th
};

//####################

//#Braille rests' shapes are same as notes above, whole rest=16th rest=256th etc.
braille_code Braille_RestWhole = braille_code("RestWhole", "134");
braille_code Braille_RestHalf = braille_code("RestHalf", "136");
braille_code Braille_RestQuarter = braille_code("RestQuarter", "1236");
braille_code Braille_Rest8th = braille_code("Rest8th", "1346");
//#16th to 128th rests
braille_code Braille_Rest16th = braille_code("Rest16th", "134");
braille_code Braille_Rest32nd = braille_code("Rest32nd", "136");
braille_code Braille_Rest64th = braille_code("Rest64th", "1236");
braille_code Braille_Rest128th = braille_code("Rest128th", "1346");
//#256th to 2048th rests
braille_code Braille_Rest256th = braille_code("Rest256th", "134");
braille_code Braille_Rest512th = braille_code("Rest512th", "136");
braille_code Braille_Rest1024th = braille_code("Rest1024th", "1236");
braille_code Braille_Rest2048th = braille_code("Rest2048th", "1346");
//#breve rest, use Haipeng's breve rest definition below as first option.
braille_code Braille_RestBreve = braille_code("RestBreve", "134-13");
//##########Added by Haipeng##########
//#More widely used Breve rest, swap the above with Alt
braille_code Braille_RestBreveAlt = braille_code("RestBreveAlt", "134-45-14-134");
//#Other longer rests in ancient music
braille_code Braille_RestLonga = braille_code("RestLonga", "134-45-14-45-14-134");
braille_code Braille_RestMaxima = braille_code("RestMaxima", "134-45-14-45-14-45-14-134");

braille_code* Braille_Rests[] = {
    &Braille_RestMaxima, &Braille_RestLonga, &Braille_RestBreve,
    &Braille_RestWhole, &Braille_RestHalf, &Braille_RestQuarter,
    &Braille_Rest8th, &Braille_Rest16th, &Braille_Rest32nd,
    &Braille_Rest64th, &Braille_Rest128th, &Braille_Rest256th,
    &Braille_Rest512th, &Braille_Rest128th, &Braille_Rest2048th,
    &Braille_RestBreveAlt };
//####################

//[MusicPunctuation]
braille_code Braille_PlusSign = braille_code("PlusSign", "346");
braille_code Braille_MinusSign = braille_code("MinusSign", "36");
braille_code Braille_SlashSign = braille_code("SlashSign", "34");
//#note dot sign, to add value for the associated note and some other situations
braille_code Braille_Dot = braille_code("Dot", "3");
braille_code Braille_Parentheses = braille_code("Parentheses", "6-3");
braille_code Braille_OpenParentheses = braille_code("OpenParentheses", "6-3");
braille_code Braille_CloseParentheses = braille_code("CloseParentheses", "6-3");
braille_code Braille_SpecialParentheses = braille_code("SpecialParentheses", "2356");
braille_code Braille_PageIndicator = braille_code("PageIndicator", "5-25");
braille_code Braille_LineIndicator = braille_code("LineIndicator", "6-123");
braille_code Braille_Hyphen = braille_code("Hyphen", "5");
braille_code Braille_MusicComma = braille_code("MusicComma", "126-2");
braille_code Braille_MusicCommaEnd = braille_code("MusicCommaEnd", "126-2-3");
//##########Added by Haipeng##########
//#Prefix for cautionary accidentals and hidden rests
braille_code Braille_Cautionary = braille_code("Cautionary", "5");
//#Prefix for editorial elements such as dashed slurs, ties and hairpins
braille_code Braille_Editorial = braille_code("Editorial", "5-123");
//####################
braille_code Braille_EqualSign = braille_code("EqualSign", "2356");
braille_code Braille_UpperOpenBracket = braille_code("UpperOpenBracket", "56-2");
braille_code Braille_UpperCloseBracket = braille_code("UpperCloseBracket", "5-23");
braille_code Braille_UpperBrokenOpenBracket = braille_code("UpperBrokenOpenBracket", "56-2-2");
braille_code Braille_UpperBrokenCloseBracket = braille_code("UpperBrokenCloseBracket", "5-5-23");
braille_code Braille_UpperOpenEndedBracket = braille_code("UpperOpenEndedBracket", "56-2");
braille_code Braille_UpperCloseEndedBracket = braille_code("UpperCloseEndedBracket", "5-3");
braille_code Braille_LowerOpenBracket = braille_code("LowerOpenBracket", "56-3");
braille_code Braille_LowerCloseBracket = braille_code("LowerCloseBracket", "6-23");
braille_code Braille_LowerBrokenOpenBracket = braille_code("LowerBrokenOpenBracket", "56-3-3");
braille_code Braille_LowerBrokenCloseBracket = braille_code("LowerBrokenCloseBracket", "6-6-23");
braille_code Braille_LowerOpenEndedBracket = braille_code("LowerOpenEndedBracket", "56-3");
braille_code Braille_LowerCloseEndedBracket = braille_code("LowerCloseEndedBracket", "6-2");
braille_code Braille_OpenMusicCodeIndicator = braille_code("OpenMusicCodeIndicator", "6-3");
braille_code Braille_CloseMusicCodeIndicator = braille_code("CloseMusicCodeIndicator", "56-23");
braille_code Braille_AsteriskSign = braille_code("AsteriskSign", "345-26-35");
braille_code Braille_FootnoteSeparator = braille_code("FootnoteSeparator", "36-36-36-36-36");
//[Harmony]
braille_code Braille_Diminished = braille_code("Diminished", "256");
braille_code Braille_HalfDiminished = braille_code("", "256-3");
braille_code Braille_Triangle = braille_code("HalfDiminished", "356");
braille_code Braille_HalfTriangle = braille_code("HalfTriangle", "356-3");
//#No chord
braille_code Braille_NoHarmony = braille_code("NoHarmony", "6-6-1345-14");

//##########Newly added by Haipeng
//#A figure is prefixed by a number sign, so here only give the signs not available elsewhere
braille_code Braille_IsolatedSharp = braille_code("IsolatedSharp", "146-13");
braille_code Braille_IsolatedDoubleSharp = braille_code("IsolatedDoubleSharp", "146-146-13");
braille_code Braille_IsolatedFlat = braille_code("IsolatedFlat", "126-13");
braille_code Braille_IsolatedDoubleFlat = braille_code("IsolatedDoubleFlat", "126-126-13");
braille_code Braille_IsolatedNatural = braille_code("IsolatedNatural", "16-13");
braille_code Braille_Cross = braille_code("Cross", "56");

braille_code Braille_FiguredBassIndicator = braille_code("FiguredBassIndicator", "56-345");
braille_code Braille_FiguredBassSeparator = braille_code("FiguredBassSeparator", "36");
braille_code Braille_PlusAccidental = braille_code("PlusAccidental", "346");
braille_code Braille_AccidentalIsolator = braille_code("AccidentalIsolator", "13");
braille_code Braille_BackslashFigure = braille_code("BackslashFigure", "56");
braille_code Braille_SlashFigure = braille_code("SlashFigure", "34");
braille_code Braille_FigureExtension = braille_code("FigureExtension", "1");
//####################
//#note value indicators: to add before a note having same Braille shape but with different value like between  half and 32nd notes.
//#whole-8th notes range
braille_code Braille_FirstValueRange = braille_code("FirstValueRange", "45-126-2");
//#16th-128th notes range
braille_code Braille_SecondValueRange = braille_code("SecondValueRange", "6-126-2");
//#256th notes and and further range
braille_code Braille_ThirdValueRange = braille_code("ThirdValueRange", "56-126-2");

braille_code* Braille_ValueRanges[] = { &Braille_FirstValueRange, &Braille_SecondValueRange, &Braille_ThirdValueRange };
//#octave signs
//#based on Piano keyboard, octave 0 and 8 for lowest and highest notes out of the full octave
//#there are 7 full octaves from 1 to 7
braille_code Braille_Octave0 = braille_code("Octave0", "4-4");
braille_code Braille_Octave1 = braille_code("Octave1", "4");
braille_code Braille_Octave2 = braille_code("Octave2", "45");
braille_code Braille_Octave3 = braille_code("Octave3", "456");
braille_code Braille_Octave4 = braille_code("Octave4", "5");
braille_code Braille_Octave5 = braille_code("Octave5", "46");
braille_code Braille_Octave6 = braille_code("Octave6", "56");
braille_code Braille_Octave7 = braille_code("Octave7", "6");
braille_code Braille_Octave8 = braille_code("Octave8", "6-6");

braille_code* Braille_Octaves[] = {
    &Braille_Octave0, &Braille_Octave1, &Braille_Octave2,
    &Braille_Octave3, &Braille_Octave4, &Braille_Octave5,
    &Braille_Octave6, &Braille_Octave7, &Braille_Octave8 };
//#clef signs
braille_code Braille_ClefG = braille_code("ClefG", "345-34-123");
braille_code Braille_ClefF = braille_code("ClefF", "345-3456-123");
braille_code Braille_ClefC = braille_code("ClefC", "345-346-123");
//#Treble and bass clefs used in different hands
braille_code Braille_ClefGLeft = braille_code("ClefGLeft", "345-34-13");
braille_code Braille_ClefFRight = braille_code("ClefFRight", "345-3456-13");
//#special clefs
//#G/F/C clef on first line (French violin), second/third/fourth/fifth line etc
braille_code Braille_ClefGFirstLine = braille_code("ClefGFirstLine", "345-34-4-123");
braille_code Braille_ClefGThirdLine = braille_code("ClefGThirdLine", "345-34-456-123");
braille_code Braille_ClefGFourthLine = braille_code("ClefGFourthLine", "345-34-5-123");
braille_code Braille_ClefGFifthLine = braille_code("ClefGFifthLine", "345-34-46-123");
braille_code Braille_ClefFFirstLine = braille_code("ClefFFirstLine", "345-3456-4-123");
braille_code Braille_ClefFSecondLine = braille_code("ClefFSecondLine", "345-3456-45-123");
braille_code Braille_ClefFThirdLine = braille_code("ClefFThirdLine", "345-3456-456-123");
//braille_code Braille_ClefFFourthLine = braille_code("ClefFFourthLine", "345-3456-5-123");
braille_code Braille_ClefFFifthLine = braille_code("ClefFFifthLine", "345-3456-46-123");
braille_code Braille_ClefCFirstLine = braille_code("ClefCFirstLine", "345-346-4-123");
braille_code Braille_ClefCSecondLine = braille_code("ClefCSecondLine", "345-346-45-123");
//braille_code Braille_ClefCThirdLine = braille_code("ClefCThirdLine", "345-346-456-123");
braille_code Braille_ClefCFourthLine = braille_code("ClefCFourthLine", "345-346-5-123");
braille_code Braille_ClefCFifthLine = braille_code("ClefCFifthLine", "345-346-46-123");
//#hands
braille_code Braille_RightHand = braille_code("RightHand", "46-345");
braille_code Braille_LeftHand = braille_code("LeftHand", "456-345");
//##########Added by Haipeng##########
//#Right and left hands with reversed interval directions in hand-changing passage. Not used for general interval direction changes such as piano left hand reading downwards in orchestral scores where all intervals are down.
braille_code Braille_RightHandUp = braille_code("RightHandUp", "46-345-345");
braille_code Braille_LeftHandDown = braille_code("LeftHandDown", "456-345-345");
//####################
braille_code Braille_OrganPedal = braille_code("OrganPedal", "45-345");
//##########Added by Haipeng##########
//#Chord and figured bass prefix
braille_code Braille_ChordPrefix = braille_code("ChordPrefix", "25-345");
//#Accordion prefix, detected by instrument definition
//AccordionBass 6-345
//#Outline prefix, when producing piano accompaniment with melody outline
braille_code Braille_Outline = braille_code("Outline", "5-345");
//####################
//#accidental signs: natural, sharp and flat.
braille_code Braille_NaturalAccidental = braille_code("NaturalAccidental", "16");
//#sharps
braille_code Braille_SharpAccidental = braille_code("SharpAccidental", "146");
//#1/4 sharp
braille_code Braille_QuarterSharp = braille_code("QuarterSharp", "4-146");
//#3/4 sharp
braille_code Braille_ThreeQuarterSharp = braille_code("ThreeQuarterSharp", "456-146");
//#flats
braille_code Braille_FlatAccidental = braille_code("FlatAccidental", "126");
braille_code Braille_QuarterFlat = braille_code("QuarterFlat", "4-126");
braille_code Braille_ThreeQuarterFlat = braille_code("ThreeQuarterFlat", "456-126");

braille_code* Braille_Accidentals[] = {
    &Braille_NaturalAccidental,
    &Braille_SharpAccidental, &Braille_QuarterSharp, &Braille_ThreeQuarterSharp,
    &Braille_FlatAccidental, &Braille_QuarterFlat, &Braille_ThreeQuarterFlat
};
//#Time signatures
braille_code Braille_CommonTime = braille_code("CommonTime", "46-14");
braille_code Braille_CutTime = braille_code("CutTime", "456-14");
//# time signature by seconds
braille_code Braille_TimeInSecondSign = braille_code("TimeInSecondSign", "45");
braille_code Braille_TimeExtensionSign = braille_code("TimeExtensionSign", "36-36");
//#ties
braille_code Braille_NoteTie = braille_code("NoteTie", "4-14");
braille_code Braille_ChordTie = braille_code("ChordTie", "46-14");
braille_code Braille_ChordTieDoubling = braille_code("ChordTieDoubling", "46-14-14");
braille_code Braille_Arpeggio = braille_code("Arpeggio", "45-14");
braille_code Braille_TieLetRing = braille_code("TieLetRing", "56-14");
braille_code Braille_TieNoStart = braille_code("TieNoStart", "46-56-14");
//##########Added by Haipeng##########
braille_code Braille_TieCrossVoice = braille_code("TieCrossVoice", "456-4-14");
braille_code Braille_TieCrossVoiceFrom = braille_code("TieCrossVoiceFrom", "46-456-4-14");
braille_code Braille_TieCrossStaff = braille_code("TieCrossStaff", "5-4-14");
braille_code Braille_TieCrossStaffFrom = braille_code("TieCrossStaffFrom", "46-5-4-14");
braille_code Braille_ChordTieCrossVoice = braille_code("ChordTieCrossVoice", "456-46-14");
braille_code Braille_ChordTieCrossVoiceFrom = braille_code("ChordTieCrossVoiceFrom", "46-456-46-14");
braille_code Braille_ChordTieCrossStaff = braille_code("ChordTieCrossStaff", "5-46-14");
braille_code Braille_ChordTieCrossStaffFrom = braille_code("ChordTieCrossStaffFrom", "46-5-46-14");
//####################
//#slurs
braille_code Braille_NoteSlur = braille_code("NoteSlur", "14");
braille_code Braille_LongSlurOpenBracket = braille_code("LongSlurOpenBracket", "56-12");
braille_code Braille_LongSlurCloseBracket = braille_code("LongSlurCloseBracket", "45-23");
braille_code Braille_ConvergentSlur = braille_code("ConvergentSlur", "6-14");
braille_code Braille_SameNoteSlur = braille_code("SameNoteSlur", "56-14");

braille_code Braille_GraceSlur = braille_code("GraceSlur", "56-14");
braille_code Braille_GraceSlurDoubling = braille_code("GraceSlurDoubling", "56-14-14");
//##########Added by Haipeng##########
braille_code Braille_SlurCrossVoice = braille_code("SlurCrossVoice", "456-14");
braille_code Braille_SlurCrossVoiceFrom = braille_code("SlurCrossVoiceFrom", "46-456-14");
braille_code Braille_SlurCrossStaff = braille_code("slurCrossStaff", "5-14");
braille_code Braille_SlurCrossStaffFrom = braille_code("SlurCrossStaffFrom", "46-5-14");
//####################
//#Intervals, Braille signs to write for chords
//#Braille sign for second interval has same dots for 9, 16, 23 (+7)
braille_code Braille_Interval2 = braille_code("Interval2", "34");
braille_code Braille_Interval3 = braille_code("Interval3", "346");
braille_code Braille_Interval4 = braille_code("Interval4", "3456");
braille_code Braille_Interval5 = braille_code("Interval5", "35");
braille_code Braille_Interval6 = braille_code("Interval6", "356");
braille_code Braille_Interval7 = braille_code("Interval7", "25");
braille_code Braille_Interval8 = braille_code("Interval8", "36");

braille_code* Braille_Intervals[] = {
    &Braille_Interval2, &Braille_Interval3, &Braille_Interval4,
    &Braille_Interval5, &Braille_Interval6, &Braille_Interval7,
    &Braille_Interval8 };
//#tuplet: note grouping 2/3/5/6/XXX-notes grouping
braille_code Braille_Tuplet3 = braille_code("Tuplet3", "23");
braille_code Braille_TupletPrefix = braille_code("TupletPrefix", "456");
//#repeats' signs in print score
braille_code Braille_RepetitionForward = braille_code("RepetitionForward", "126-2356");
braille_code Braille_RepetitionBackward = braille_code("RepetitionBackward", "126-23");
braille_code Braille_Coda = braille_code("Coda", "346-123");
braille_code Braille_Segno = braille_code("Segno", "346");
// Fermata
braille_code Braille_InvertedType = braille_code("InvertedType", "5");
braille_code Braille_FermataDefault = braille_code("FermataDefault", "126-123");
braille_code Braille_FermataSquare = braille_code("FermataSquare", "56-126-123");
braille_code Braille_FermataAngled = braille_code("FermataAngled", "45-126-123");
braille_code Braille_FermataDoubleSquare = braille_code("FermataDoubleSquare", "56-56-126-123");
braille_code Braille_FermataDoubleAngled = braille_code("FermataDoubleAngled", "45-45-126-123");
braille_code Braille_FermataHalfCurve = braille_code("FermataHalfCurve", "45-126-123");
braille_code Braille_FermataDoubleDot = braille_code("FermataDoubleDot", "56-126-123");
//##Newly added by Haipeng##
braille_code Braille_FermataBarline = braille_code("FermataBarline", "456-126-123");
braille_code Braille_FermataNormalBarline = braille_code("FermataNormalBarline", "456");
//#Braille measure or partial repetition sign
braille_code Braille_NotesRepeat = braille_code("NotesRepeat", "2356");
//#Newly added by Haipeng##
//#This repeat should be used according to beamings in unmeasured passage. It should be used between beams, within a long beam, but can't cross beams in different places, thus break the musical meaning implied by the beaming.
braille_code Braille_StartingBeamRepeat = braille_code("StartingBeamRepeat", "16-2356");
//#barlines
braille_code Braille_SingleBarline = braille_code("SingleBarline", "0");
braille_code Braille_DashedBarline = braille_code("DashedBarline", "13");
braille_code Braille_SpecialBarline = braille_code("SpecialBarline", "123");
braille_code Braille_SectionalDouble = braille_code("SectionalDouble", "126-13-3");
braille_code Braille_FinalDouble = braille_code("FinalDouble", "126-13");
//#fingering
braille_code Braille_Finger0 = braille_code("Finger0", "13"); //#thumb
braille_code Braille_Finger1 = braille_code("Finger1", "1");     //#index
braille_code Braille_Finger2 = braille_code("Finger2", "12");    //#middle
braille_code Braille_Finger3 = braille_code("Finger3", "123");   //#ring
braille_code Braille_Finger4 = braille_code("Finger4", "2");     //#little
braille_code Braille_Finger5 = braille_code("Finger5", "13");    //#open string

braille_code Braille_FingerSlur = braille_code("FingerSlur", "14");    //Finger slur

braille_code* Braille_Fingers[] = { &Braille_Finger0, &Braille_Finger1, &Braille_Finger2,
                                    &Braille_Finger3, &Braille_Finger4, &Braille_Finger5 };
//##Comment by Haipeng: The names above are not strict, but I believe they can be correctly mapped to Musicxml.##
//#plucks, for string instruments
braille_code Braille_PluckP = braille_code("PluckP", "1234"); //#thumb
braille_code Braille_PluckI = braille_code("PluckI", "24");   //#index
braille_code Braille_PluckM = braille_code("PluckM", "134");  //#middle
braille_code Braille_PluckA = braille_code("PluckA", "1");    //#ring
braille_code Braille_PluckC = braille_code("PluckC", "1346");   //#little

braille_code* Braille_Plucks[] = { &Braille_PluckP, &Braille_PluckI, &Braille_PluckM,
                                   &Braille_PluckA, &Braille_PluckC };

braille_code Braille_Dot6PluckP = braille_code("Dot6PluckP", "6-1");
braille_code Braille_Dot6PluckI = braille_code("Dot6PluckI", "6-12");
braille_code Braille_Dot6PluckM = braille_code("Dot6PluckM", "6-123");
braille_code Braille_Dot6PluckA = braille_code("Dot6PluckA", "6-2");
braille_code Braille_Dot6PluckC = braille_code("Dot6PluckC", "6-13");
//#string
braille_code Braille_String1 = braille_code("String1", "146-1");
braille_code Braille_String2 = braille_code("String2", "146-12");
braille_code Braille_String3 = braille_code("String3", "146-123");
braille_code Braille_String4 = braille_code("String4", "146-2");
braille_code Braille_String5 = braille_code("String5", "146-13");
braille_code Braille_String6 = braille_code("String6", "146-23");
braille_code Braille_String7 = braille_code("String7", "146-3");

braille_code* Braille_Strings[] = {
    &Braille_String1, &Braille_String2, &Braille_String3,
    &Braille_String4, &Braille_String5, &Braille_String6,
    &Braille_String7 };

braille_code Braille_String1_Doubling = braille_code("String1Doubling", "146-1-1");
braille_code Braille_String2_Doubling = braille_code("String2Doubling", "146-12-12");
braille_code Braille_String3_Doubling = braille_code("String3Doubling", "146-123-123");
braille_code Braille_String4_Doubling = braille_code("String4Doubling", "146-2-2");
braille_code Braille_String5_Doubling = braille_code("String5Doubling", "146-13-13");
braille_code Braille_String6_Doubling = braille_code("String6Doubling", "146-23-23");
braille_code Braille_String7_Doubling = braille_code("String7Doubling", "146-3-3");

braille_code* Braille_Strings_Doubling[] = {
    &Braille_String1_Doubling, &Braille_String2_Doubling, &Braille_String3_Doubling,
    &Braille_String4_Doubling, &Braille_String5_Doubling, &Braille_String6_Doubling,
    &Braille_String7_Doubling };
//[Fret]
//#frets
braille_code Braille_Fret1 = braille_code("Fret1", "345-345");
braille_code Braille_Fret2 = braille_code("Fret2", "345-34");
braille_code Braille_Fret3 = braille_code("Fret3", "345-346");
braille_code Braille_Fret4 = braille_code("Fret4", "345-3456");
braille_code Braille_Fret5 = braille_code("Fret5", "345-35");
braille_code Braille_Fret6 = braille_code("Fret6", "345-356");
braille_code Braille_Fret7 = braille_code("Fret7", "6-345-25");
braille_code Braille_Fret8 = braille_code("Fret8", "345-36");
braille_code Braille_Fret9 = braille_code("Fret9", "345-36-34");
braille_code Braille_Fret10 = braille_code("Fret10", "345-36-346");
braille_code Braille_Fret11 = braille_code("Fret11", "345-36-3456");
braille_code Braille_Fret12 = braille_code("Fret12", "345-36-35");
braille_code Braille_Fret13 = braille_code("Fret13", "345-36-356");

braille_code* Braille_Frets[] = {
    &Braille_Fret1, &Braille_Fret2, &Braille_Fret3,
    &Braille_Fret4, &Braille_Fret5, &Braille_Fret6,
    &Braille_Fret7, &Braille_Fret8, &Braille_Fret9,
    &Braille_Fret10, &Braille_Fret11, &Braille_Fret12,
    &Braille_Fret13 };
//[VoiceAccord]
//#accord
braille_code Braille_FullMeasureAccord = braille_code("FullMeasureAccord", "126-345");
braille_code Braille_PartialMeasureAccord = braille_code("PartialMeasureAccord", "5-2");
braille_code Braille_MeasureSeparator = braille_code("MeasureSeparator", "46-13");
//[Pedal]
//#pedal signs
braille_code Braille_PedalDown = braille_code("PedalDown", "126-14");
braille_code Braille_PedalUp = braille_code("PedalUp", "16-14");
braille_code Braille_NotePedalUpDownAtNote = braille_code("NotePedalUpDownAtNote", "16-126-14");
braille_code Braille_HalfPedal = braille_code("HalfPedal", "5-126-14");
braille_code Braille_AfterNotePedalDown = braille_code("AfterNotePedalDown", "6-126-14");
braille_code Braille_AfterNotePedalUp = braille_code("AfterNotePedalUp", "5-16-14");
braille_code Braille_NoPedal = braille_code("NoPedal", "16-14");
//[Ornament]
//#articulations, ornaments
braille_code Braille_Spiccato = braille_code("Spiccato", "236");
braille_code Braille_Staccato = braille_code("Staccato", "236");
braille_code Braille_Staccatissimo = braille_code("Staccatissimo", "6-236");
braille_code Braille_DetachedLegato = braille_code("DetachedLegato", "5-236");
braille_code Braille_Tenuto = braille_code("Tenuto", "456-236");
braille_code Braille_Accent = braille_code("Accent", "46-236");
braille_code Braille_Stress = braille_code("Stress", "45-236");
braille_code Braille_Unstress = braille_code("Unstress", "4-236");
braille_code Braille_StrongAccent = braille_code("StrongAccent", "56-236");
//##########Added by Haipeng##########
braille_code Braille_SoftAccent = braille_code("SoftAccent", "16-3");
braille_code Braille_Scoop = braille_code("Scoop", "126-3-14");
braille_code Braille_Plop = braille_code("Plop", "126-12-14");
braille_code Braille_Doit = braille_code("Doit", "14-126-3");
braille_code Braille_Falloff = braille_code("Falloff", "14-126-12");
braille_code Braille_UpBow = braille_code("UpBow", "126-3");
braille_code Braille_DownBow = braille_code("DownBow", "126-12");
braille_code Braille_HarmonicNatural = braille_code("HarmonicNatural", "13");
braille_code Braille_HarmonicArtificial = braille_code("HarmonicArtificial", "16-123");
braille_code Braille_OpenString = braille_code("OpenString", "13");
braille_code Braille_Stopped = braille_code("Stopped", "126-12");
braille_code Braille_SnapPizzicato = braille_code("SnapPizzicato", "16-13");
braille_code Braille_HammerOn = braille_code("HammerOn", "126-3");
braille_code Braille_PullOff = braille_code("PullOff", "126-12");
//####################
braille_code Braille_ArpeggiateUp = braille_code("ArpeggiateUp", "345-13");
braille_code Braille_ArpeggiateDown = braille_code("ArpeggiateDown", "345-13-13");
braille_code Braille_Cue = braille_code("Cue", "6-26");
braille_code Braille_GraceShort = braille_code("GraceShort", "26");
braille_code Braille_GraceLong = braille_code("GraceLong", "5-26");
braille_code Braille_TrillMark = braille_code("TrillMark", "235");
braille_code Braille_Turn = braille_code("Turn", "6-256");
braille_code Braille_InvertedTurn = braille_code("InvertedTurn", "6-256-123");
braille_code Braille_DelayedTurn = braille_code("DelayedTurn", "256");
braille_code Braille_DelayedInvertedTurn = braille_code("DelayedInvertedTurn", "256-123");
braille_code Braille_MordentShort = braille_code("MordentShort", "5-235-123");
braille_code Braille_MordentLong = braille_code("MordentLong", "56-235-123");
braille_code Braille_InvertedMordentShort = braille_code("InvertedMordentShort", "5-235");
braille_code Braille_InvertedMordentLong = braille_code("InvertedMordentLong", "56-235");
//##Added by Haipeng##
braille_code Braille_Schleifer = braille_code("Schleifer", "4-26");
braille_code Braille_Shake = braille_code("Shake", "5-235-123");
braille_code Braille_BreathMark = braille_code("BreathMark", "345-2");
braille_code Braille_Caesura = braille_code("Caesura", "6-34");

braille_code Braille_TremoloSingle1 = braille_code("TremoloSingle1", "45-12");
braille_code Braille_TremoloSingle2 = braille_code("TremoloSingle2", "45-123");
braille_code Braille_TremoloSingle3 = braille_code("TremoloSingle3", "45-2");
braille_code Braille_TremoloSingle4 = braille_code("TremoloSingle4", "45-13");
braille_code Braille_TremoloSingle5 = braille_code("TremoloSingle5", "45-3");

braille_code* Braille_TremoloSingles[] = {
    &Braille_TremoloSingle1, &Braille_TremoloSingle2, &Braille_TremoloSingle3,
    &Braille_TremoloSingle4, &Braille_TremoloSingle5 };

braille_code Braille_TremoloDouble1 = braille_code("TremoloDouble1", "46-12");
braille_code Braille_TremoloDouble2 = braille_code("TremoloDouble2", "46-123");
braille_code Braille_TremoloDouble3 = braille_code("TremoloDouble3", "46-2");
braille_code Braille_TremoloDouble4 = braille_code("TremoloDouble4", "46-1");
braille_code Braille_TremoloDouble5 = braille_code("TremoloDouble5", "46-3");

braille_code* Braille_TremoloDoubles[] = {
    &Braille_TremoloDouble1, &Braille_TremoloDouble2, &Braille_TremoloDouble3,
    &Braille_TremoloDouble4, &Braille_TremoloDouble5 };

braille_code Braille_Glissando = braille_code("Glissando", "4-1");
braille_code Braille_LongGlissandoStart = braille_code("LongGlissandoStart", "4-1-3");
braille_code Braille_LongGlissandoStop = braille_code("LongGlissandoStop", "6-4-1");
braille_code Braille_Slide = braille_code("Slide", "4-1");
braille_code Braille_LongSlideStart = braille_code("LongSlideStart", "4-1-3");
braille_code Braille_LongSlideStop = braille_code("LongSlideStop", "6-4-1");
//##########Added by Haipeng##########
//[Noteheads]
braille_code Braille_blackHead = braille_code("blackHead", "26-1");
braille_code Braille_XShape = braille_code("XShape", "26-12");
braille_code Braille_Diamond = braille_code("Diamond", "26-123");
braille_code Braille_DiamondHarmonic = braille_code("DiamondHarmonic", "16-123");
braille_code Braille_Slashed = braille_code("Slashed", "26-13");
//#Comment: If the diamond is in a chord, then use DiamondHarmonic, since it's an artificial harmonic.
braille_code Braille_StemOnly = braille_code("StemOnly", "26-13");
braille_code Braille_Circled = braille_code("Circled", "26-2");
//[Stems]
braille_code Braille_StemPrefix = braille_code("StemPrefix", "456");
braille_code Braille_StemWhole = braille_code("StemWhole", "3");
braille_code Braille_StemHalf = braille_code("StemHalf", "13");
braille_code Braille_StemQuarter = braille_code("StemQuarter", "1");
braille_code Braille_Stem8th = braille_code("Stem8th", "12");
braille_code Braille_Stem16th = braille_code("Stem16th", "123");
braille_code Braille_Stem32nd = braille_code("Stem32nd", "2");
//[HarpPedalDiagram]
braille_code Braille_HarpPedalBegin = braille_code("HarpPedalBegin", "345-36");
braille_code Braille_HarpPedalEnd = braille_code("HarpPedalEnd", "3-345");
braille_code Braille_HarpPedalRaised = braille_code("HarpPedalRaised", "12");
braille_code Braille_HarpPedalCentered = braille_code("HarpPedalCentered", "2");
braille_code Braille_HarpPedalLowered = braille_code("HarpPedalLowered", "23");
braille_code Braille_HarpPedalDivider = braille_code("HarpPedalDivider", "123");
//[FetherBeams]
braille_code Braille_FanBeamAccelerando = braille_code("FanBeamAccelerando", "45-126-2-6-126-2");
braille_code Braille_FanBeamRitardando = braille_code("FanBeamRitardando", "6-126-2-45-126-2");
braille_code Braille_FanBeamSteady = braille_code("FanBeamSteady", "6-126-2-6-126-2");
braille_code Braille_FanBeamEnd = braille_code("FanBeamEnd", "56-13");
// Placement
braille_code Braille_PlacementBelow = braille_code("PlacementBelow", "6");
// Hand interval up/down
braille_code Braille_LeftHandIntervalDown = braille_code("LeftHandIntervalDown", "456-345-345");
braille_code Braille_RightHandIntervalUp = braille_code("RightHandIntervalUp", "46-345-345");
// Additional braille codes
braille_code Braille_NewLine = braille_code("NewLine", "\n");
// Slurs
braille_code Braille_SlurCrossVoiceDubling = braille_code("SlurCrossVoiceDubling", "456-14-14");
braille_code Braille_SlurCrossStaffDubling = braille_code("SlurCrossStaffDubling", "5-14-14");
// Wavy
braille_code Braille_OrnamentWavyStart = braille_code("OrnamentWavyStart", "3-3");
braille_code Braille_OrnamentWavyStop = braille_code("OrnamentWavyStop", "345-3");
braille_code Braille_WavyRepetitionLine = braille_code("WavyRepetitionLine", "26-2356");
// Dubling
braille_code Braille_CueDubling = braille_code("CueDubling", "6-26-26");
braille_code Braille_GraceLongDubling = braille_code("GraceLongDubling", "5-26-26");
// Noteheads dubling
braille_code Braille_blackHeadDubling = braille_code("blackHeadDubling", "26-1-1");
braille_code Braille_XShapeDubling = braille_code("XShapeDubling", "26-12-12");
braille_code Braille_DiamondDubling = braille_code("DiamondDubling", "26-123-123");
braille_code Braille_DiamondHarmonicDubling = braille_code("DiamondHarmonicDubling", "16-123-16-123");
braille_code Braille_SlashedDubling = braille_code("SlashedDubling", "26-13-13");
braille_code Braille_StemOnlyDubling = braille_code("StemOnlyDubling", "26-13-13");
braille_code Braille_CircledDubling = braille_code("CircledDubling", "26-2-2");
// Tremolo dubling
braille_code Braille_TremoloSingle1Dubling = braille_code("TremoloSingle1Dubling", "45-12-12");
braille_code Braille_TremoloSingle2Dubling = braille_code("TremoloSingle2Dubling", "45-123-123");
braille_code Braille_TremoloSingle3Dubling = braille_code("TremoloSingle3Dubling", "45-2-2");
braille_code Braille_TremoloSingle4Dubling = braille_code("TremoloSingle4Dubling", "45-13-13");
braille_code Braille_TremoloSingle5Dubling = braille_code("TremoloSingle5Dubling", "45-3-3");
braille_code Braille_TremoloDouble1Dubling = braille_code("TremoloDouble1Dubling", "46-12-12");
braille_code Braille_TremoloDouble2Dubling = braille_code("TremoloDouble2Dubling", "46-123-123");
braille_code Braille_TremoloDouble3Dubling = braille_code("TremoloDouble3Dubling", "46-2-2");
braille_code Braille_TremoloDouble4Dubling = braille_code("TremoloDouble4Dubling", "46-1-1");
braille_code Braille_TremoloDouble5Dubling = braille_code("TremoloDouble5Dubling", "46-3-3");
// Dash sign
braille_code Braille_DashSign = braille_code("DashSign", "36");
// Dashline
braille_code Braille_DashLineStart = braille_code("DashLineStart", "3-3");
braille_code Braille_DashLineStop = braille_code("DashLineStop", "345-3");
braille_code Braille_DashNestedLineStart = braille_code("DashNestedLineStart", "36-36");
braille_code Braille_DashNestedLineStop = braille_code("DashNestedLineStop", "345-36");
// Elision
braille_code Braille_Elision2 = braille_code("Elision2", "12");
braille_code Braille_Elision3 = braille_code("Elision3", "123");
// Empty
braille_code Braille_Empty = braille_code("Empty", "");
// line over line format
braille_code Braille_HarmonyLineIndicator = braille_code("HarmonyLineIndicator", "25-345");
braille_code Braille_LyricLineIndicator = braille_code("LyricLineIndicator", "56-23");
braille_code Braille_MusicLineIndicator = braille_code("MusicLineIndicator", "6-3");
braille_code Braille_PluckLineIndicator = braille_code("PluckLineIndicator", "0-0");
braille_code Braille_SoloMelodyLineIndicator = braille_code("SoloMelodyLineIndicator", "5-345");
// wedge stop
braille_code Braille_WedgeCStop = braille_code("WedgeCStop", "345-25");
braille_code Braille_WedgeDStop = braille_code("WedgeDStop", "345-256");
// Repeats
braille_code Braille_PartialRepeat = braille_code("PartialRepeat", "2356");
braille_code Braille_MeasureRepeat = braille_code("MeasureRepeat", "2356"); // ??? 2356?
// Check dot
braille_code Braille_CheckDot = braille_code("CheckDot", "");
// Check figure Bass Separator
braille_code Braille_CheckFiguredBassSeparator = braille_code("CheckFiguredBassSeparator", "");
// Thumb
braille_code Braille_ThumbPosition = braille_code("ThumbPosition", "16-13");
// Accordion
braille_code Braille_AccordionMiddle2 = braille_code("AccordionMiddle2", "34");
braille_code Braille_AccordionMiddle3 = braille_code("AccordionMiddle3", "346");
// String intrument
braille_code Braille_LeftHandPizzicato = braille_code("LeftHandPizzicato", "456-345");

std::string getBraillePattern(std::string dots)
{
    const char* dotc = dots.c_str();
    int d = atoi(dotc);

    switch (d) {
    // No dots
    case 0: return " ";

    // One dot
    case 1: return "⠁";
    case 2: return "⠂";
    case 3: return "⠄";
    case 4: return "⠈";
    case 5: return "⠐";
    case 6: return "⠠";

    // Two dots
    case 12: return "⠃";
    case 13: return "⠅";
    case 14: return "⠉";
    case 15: return "⠑";
    case 16: return "⠡";
    case 23: return "⠆";
    case 24: return "⠊";
    case 25: return "⠒";
    case 26: return "⠢";
    case 34: return "⠌";
    case 35: return "⠔";
    case 36: return "⠤";
    case 45: return "⠘";
    case 46: return "⠨";
    case 56: return "⠰";

    // Three dots
    case 123: return "⠇";
    case 124: return "⠋";
    case 125: return "⠓";
    case 126: return "⠣";
    case 134: return "⠍";
    case 135: return "⠕";
    case 136: return "⠥";
    case 145: return "⠙";
    case 146: return "⠩";
    case 156: return "⠱";
    case 234: return "⠎";
    case 235: return "⠖";
    case 236: return "⠦";
    case 245: return "⠚";
    case 246: return "⠪";
    case 256: return "⠲";
    case 345: return "⠜";
    case 346: return "⠬";
    case 356: return "⠴";
    case 456: return "⠸";

    // Four dots
    case 1234: return "⠏";
    case 1235: return "⠗";
    case 1236: return "⠧";
    case 1245: return "⠛";
    case 1246: return "⠫";
    case 1256: return "⠳";
    case 1345: return "⠝";
    case 1346: return "⠭";
    case 1356: return "⠵";
    case 1456: return "⠹";
    case 2345: return "⠞";
    case 2346: return "⠮";
    case 2356: return "⠶";
    case 2456: return "⠺";
    case 3456: return "⠼";

    // Five dots
    case 12345: return "⠟";
    case 12346: return "⠯";
    case 12356: return "⠷";
    case 12456: return "⠻";
    case 13456: return "⠽";
    case 23456: return "⠾";

    // Six dots
    case 123456: return "⠿";
    }

    return " ";
}

std::string translate2Braille(std::string codes)
{
    std::stringstream test(codes);
    std::string segment;
    std::vector<std::string> seglist;

    std::string txt = "";
    while (std::getline(test, segment, '-')) {
        txt.append(getBraillePattern(segment));
    }
    return txt;
}

std::string intToBrailleUpperNumbers(std::string txt, bool indicator)
{
    std::string braille = "";
    if (indicator) {
        braille.append(translate2Braille(Braille_NumIndicator.code));
    }

    for (size_t i=0; i < txt.length(); i++) {
        char c = txt.at(i);
        if (c - '0' <= 9) {
            braille.append(translate2Braille(Braille_UpperNumbers[c - '0']->code));
        }
    }
    return braille;
}

std::string intToBrailleLowerNumbers(std::string txt, bool indicator)
{
    std::string braille = "";
    if (indicator) {
        braille.append(Braille_NumIndicator.code);
    }

    for (size_t i=0; i < txt.length(); i++) {
        char c = txt.at(i);
        if (c - '0' <= 9) {
            braille.append(Braille_LowerNumbers[c - '0']->code);
        }
    }
    return braille;
}

std::vector<std::string> splitCodes(std::string code)
{
    std::stringstream test(code);
    std::string segment;
    std::vector<std::string> seglist;

    std::vector<std::string> lst;
    while (std::getline(test, segment, '-')) {
        lst.push_back(segment);
    }
    return lst;
}
}
