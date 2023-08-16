/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_MSBASICPRESETSCATEGORIES_H
#define MU_PLAYBACK_MSBASICPRESETSCATEGORIES_H

#include <vector>

#include "types/string.h"

#include "midi/miditypes.h"

namespace mu::playback {
struct MsBasicItem {
    String title;
    std::vector<MsBasicItem> subItems;
    midi::Program preset = {};

    MsBasicItem(String&& title, std::vector<MsBasicItem>&& subItems)
        : title(title), subItems(subItems) {}

    MsBasicItem(midi::Program&& preset, String&& titleOverride = {})
        : title(titleOverride), preset(preset) {}
};

// Based on https://docs.google.com/spreadsheets/d/1SwqZb8lq5rfv5regPSA10drWjUAoi65EuMoYtG-4k5s/edit#gid=1502434630
static const std::vector<MsBasicItem> MS_BASIC_PRESET_CATEGORIES {
    MsBasicItem {
        /*title=*/ u"Pipe",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  72) },
            MsBasicItem { midi::Program(17, 72) },
            MsBasicItem { midi::Program(0,  73) },
            MsBasicItem { midi::Program(17, 73) },
            MsBasicItem { midi::Program(0,  74) },
            MsBasicItem { midi::Program(17, 74) },
            MsBasicItem { midi::Program(0,  75) },
            MsBasicItem { midi::Program(17, 75) },
            MsBasicItem { midi::Program(0,  76) },
            MsBasicItem { midi::Program(17, 76) },
            MsBasicItem { midi::Program(0,  77) },
            MsBasicItem { midi::Program(17, 77) },
            MsBasicItem { midi::Program(0,  78) },
            MsBasicItem { midi::Program(17, 78) },
            MsBasicItem { midi::Program(0,  79) },
            MsBasicItem { midi::Program(17, 79) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Reed",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  64) },
            MsBasicItem { midi::Program(17, 64) },
            MsBasicItem { midi::Program(0,  65) },
            MsBasicItem { midi::Program(17, 65) },
            MsBasicItem { midi::Program(0,  66) },
            MsBasicItem { midi::Program(17, 66) },
            MsBasicItem { midi::Program(0,  67) },
            MsBasicItem { midi::Program(17, 67) },
            MsBasicItem { midi::Program(0,  68) },
            MsBasicItem { midi::Program(17, 68) },
            MsBasicItem { midi::Program(0,  69) },
            MsBasicItem { midi::Program(17, 69) },
            MsBasicItem { midi::Program(0,  70) },
            MsBasicItem { midi::Program(17, 70) },
            MsBasicItem { midi::Program(0,  71) },
            MsBasicItem { midi::Program(17, 71) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Brass",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  56) },
            MsBasicItem { midi::Program(17, 56) },
            MsBasicItem { midi::Program(0,  57) },
            MsBasicItem { midi::Program(17, 57) },
            MsBasicItem { midi::Program(0,  58) },
            MsBasicItem { midi::Program(17, 58) },
            MsBasicItem { midi::Program(0,  59) },
            MsBasicItem { midi::Program(17, 59) },
            MsBasicItem { midi::Program(0,  60) },
            MsBasicItem { midi::Program(17, 60) },
            MsBasicItem { midi::Program(0,  61) },
            MsBasicItem { midi::Program(17, 61) },
            MsBasicItem { midi::Program(8,  61) },
            MsBasicItem { midi::Program(18, 61) },
            MsBasicItem { midi::Program(0,  62) },
            MsBasicItem { midi::Program(17, 62) },
            MsBasicItem { midi::Program(8,  62) },
            MsBasicItem { midi::Program(18, 62) },
            MsBasicItem { midi::Program(0,  63) },
            MsBasicItem { midi::Program(17, 63) },
            MsBasicItem { midi::Program(8,  63) },
            MsBasicItem { midi::Program(18, 63) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Vocal",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  52) },
            MsBasicItem { midi::Program(17, 52) },
            MsBasicItem { midi::Program(0,  53) },
            MsBasicItem { midi::Program(17, 53) },
            MsBasicItem { midi::Program(0,  54) },
            MsBasicItem { midi::Program(17, 54) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Piano",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  0) },
            MsBasicItem { midi::Program(8,  0) },
            MsBasicItem { midi::Program(0,  1) },
            MsBasicItem { midi::Program(0,  2) },
            MsBasicItem { midi::Program(0,  3) },
            MsBasicItem { midi::Program(0,  4) },
            MsBasicItem { midi::Program(8,  4) },
            MsBasicItem { midi::Program(0,  5) },
            MsBasicItem { midi::Program(8,  5) },
            MsBasicItem { midi::Program(0,  6) },
            MsBasicItem { midi::Program(8,  6) },
            MsBasicItem { midi::Program(0,  7) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Organ",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  16) },
            MsBasicItem { midi::Program(17, 16) },
            MsBasicItem { midi::Program(8,  16) },
            MsBasicItem { midi::Program(18, 16) },
            MsBasicItem { midi::Program(0,  17) },
            MsBasicItem { midi::Program(17, 17) },
            MsBasicItem { midi::Program(8,  17) },
            MsBasicItem { midi::Program(18, 17) },
            MsBasicItem { midi::Program(0,  18) },
            MsBasicItem { midi::Program(17, 18) },
            MsBasicItem { midi::Program(0,  19) },
            MsBasicItem { midi::Program(17, 19) },
            MsBasicItem { midi::Program(8,  19) },
            MsBasicItem { midi::Program(18, 19) },
            MsBasicItem { midi::Program(0,  20) },
            MsBasicItem { midi::Program(17, 20) },
            MsBasicItem { midi::Program(0,  21) },
            MsBasicItem { midi::Program(17, 21) },
            MsBasicItem { midi::Program(8,  21) },
            MsBasicItem { midi::Program(18, 21) },
            MsBasicItem { midi::Program(0,  22) },
            MsBasicItem { midi::Program(17, 22) },
            MsBasicItem { midi::Program(0,  23) },
            MsBasicItem { midi::Program(17, 23) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Guitar",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  24) },
            MsBasicItem { midi::Program(8,  24) },
            MsBasicItem { midi::Program(0,  25) },
            MsBasicItem { midi::Program(8,  25) },
            MsBasicItem { midi::Program(16, 25) },
            MsBasicItem { midi::Program(0,  26) },
            MsBasicItem { midi::Program(8,  26) },
            MsBasicItem { midi::Program(0,  27) },
            MsBasicItem { midi::Program(0,  28) },
            MsBasicItem { midi::Program(8,  28) },
            MsBasicItem { midi::Program(0,  29) },
            MsBasicItem { midi::Program(0,  30) },
            MsBasicItem { midi::Program(8,  30) },
            MsBasicItem { midi::Program(0,  31) },
            MsBasicItem { midi::Program(8,  31) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Bass",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  32) },
            MsBasicItem { midi::Program(0,  33) },
            MsBasicItem { midi::Program(0,  34) },
            MsBasicItem { midi::Program(0,  35) },
            MsBasicItem { midi::Program(0,  36) },
            MsBasicItem { midi::Program(0,  37) },
            MsBasicItem { midi::Program(0,  38) },
            MsBasicItem { midi::Program(8,  38) },
            MsBasicItem { midi::Program(0,  39) },
            MsBasicItem { midi::Program(8,  39) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Strings",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  40) },
            MsBasicItem { midi::Program(17, 40) },
            MsBasicItem { midi::Program(8,  40) },
            MsBasicItem { midi::Program(18, 40) },
            MsBasicItem { midi::Program(0,  41) },
            MsBasicItem { midi::Program(17, 41) },
            MsBasicItem { midi::Program(0,  42) },
            MsBasicItem { midi::Program(17, 42) },
            MsBasicItem { midi::Program(0,  43) },
            MsBasicItem { midi::Program(17, 43) },
            MsBasicItem { midi::Program(0,  44) },
            MsBasicItem { midi::Program(17, 44) },
            MsBasicItem { midi::Program(20, 44) },
            MsBasicItem { midi::Program(21, 44) },
            MsBasicItem { midi::Program(25, 44) },
            MsBasicItem { midi::Program(26, 44) },
            MsBasicItem { midi::Program(30, 44) },
            MsBasicItem { midi::Program(31, 44) },
            MsBasicItem { midi::Program(40, 44) },
            MsBasicItem { midi::Program(41, 44) },
            MsBasicItem { midi::Program(50, 44) },
            MsBasicItem { midi::Program(51, 44) },
            MsBasicItem { midi::Program(0,  45) },
            MsBasicItem { midi::Program(20, 45) },
            MsBasicItem { midi::Program(25, 45) },
            MsBasicItem { midi::Program(30, 45) },
            MsBasicItem { midi::Program(40, 45) },
            MsBasicItem { midi::Program(50, 45) },
            MsBasicItem { midi::Program(0,  46) },
            MsBasicItem { midi::Program(0,  47) },
            MsBasicItem { midi::Program(0,  48) },
            MsBasicItem { midi::Program(17, 48) },
            MsBasicItem { midi::Program(8,  48) },
            MsBasicItem { midi::Program(20, 48) },
            MsBasicItem { midi::Program(21, 48) },
            MsBasicItem { midi::Program(25, 48) },
            MsBasicItem { midi::Program(26, 48) },
            MsBasicItem { midi::Program(30, 48) },
            MsBasicItem { midi::Program(31, 48) },
            MsBasicItem { midi::Program(40, 48) },
            MsBasicItem { midi::Program(41, 48) },
            MsBasicItem { midi::Program(50, 48) },
            MsBasicItem { midi::Program(51, 48) },
            MsBasicItem { midi::Program(0,  49) },
            MsBasicItem { midi::Program(17, 49) },
            MsBasicItem { midi::Program(20, 49) },
            MsBasicItem { midi::Program(21, 49) },
            MsBasicItem { midi::Program(25, 49) },
            MsBasicItem { midi::Program(26, 49) },
            MsBasicItem { midi::Program(30, 49) },
            MsBasicItem { midi::Program(31, 49) },
            MsBasicItem { midi::Program(40, 49) },
            MsBasicItem { midi::Program(41, 49) },
            MsBasicItem { midi::Program(50, 49) },
            MsBasicItem { midi::Program(51, 49) },
            MsBasicItem { midi::Program(0,  50) },
            MsBasicItem { midi::Program(17, 50) },
            MsBasicItem { midi::Program(8,  50) },
            MsBasicItem { midi::Program(18, 50) },
            MsBasicItem { midi::Program(0,  51) },
            MsBasicItem { midi::Program(17, 51) },
        }
    },

    MsBasicItem {
        /*title=*/ u"World",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  104) },
            MsBasicItem { midi::Program(0,  105) },
            MsBasicItem { midi::Program(0,  106) },
            MsBasicItem { midi::Program(0,  107) },
            MsBasicItem { midi::Program(8,  107) },
            MsBasicItem { midi::Program(0,  108) },
            MsBasicItem { midi::Program(0,  109) },
            MsBasicItem { midi::Program(0,  110) },
            MsBasicItem { midi::Program(17, 110) },
            MsBasicItem { midi::Program(0,  111) },
            MsBasicItem { midi::Program(17, 111) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Synth",
        /*subItems=*/ {
            MsBasicItem {
                /*title=*/ u"Lead",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(0,  80) },
                    MsBasicItem { midi::Program(17, 80) },
                    MsBasicItem { midi::Program(8,  80) },
                    MsBasicItem { midi::Program(18, 80) },
                    MsBasicItem { midi::Program(0,  81) },
                    MsBasicItem { midi::Program(17, 81) },
                    MsBasicItem { midi::Program(0,  82) },
                    MsBasicItem { midi::Program(17, 82) },
                    MsBasicItem { midi::Program(0,  83) },
                    MsBasicItem { midi::Program(17, 83) },
                    MsBasicItem { midi::Program(0,  84) },
                    MsBasicItem { midi::Program(17, 84) },
                    MsBasicItem { midi::Program(0,  85) },
                    MsBasicItem { midi::Program(17, 85) },
                    MsBasicItem { midi::Program(0,  86) },
                    MsBasicItem { midi::Program(17, 86) },
                    MsBasicItem { midi::Program(0,  87) },
                    MsBasicItem { midi::Program(17, 87) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Pad",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(0,  88) },
                    MsBasicItem { midi::Program(0,  89) },
                    MsBasicItem { midi::Program(17, 89) },
                    MsBasicItem { midi::Program(0,  90) },
                    MsBasicItem { midi::Program(17, 90) },
                    MsBasicItem { midi::Program(0,  91) },
                    MsBasicItem { midi::Program(17, 91) },
                    MsBasicItem { midi::Program(0,  92) },
                    MsBasicItem { midi::Program(17, 92) },
                    MsBasicItem { midi::Program(0,  93) },
                    MsBasicItem { midi::Program(17, 93) },
                    MsBasicItem { midi::Program(0,  94) },
                    MsBasicItem { midi::Program(17, 94) },
                    MsBasicItem { midi::Program(0,  95) },
                    MsBasicItem { midi::Program(17, 95) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Effects",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(0,  96) },
                    MsBasicItem { midi::Program(0,  97) },
                    MsBasicItem { midi::Program(17, 97) },
                    MsBasicItem { midi::Program(0,  98) },
                    MsBasicItem { midi::Program(0,  99) },
                    MsBasicItem { midi::Program(17, 99) },
                    MsBasicItem { midi::Program(0,  100) },
                    MsBasicItem { midi::Program(0,  101) },
                    MsBasicItem { midi::Program(17, 101) },
                    MsBasicItem { midi::Program(0,  102) },
                    MsBasicItem { midi::Program(17, 102) },
                    MsBasicItem { midi::Program(0,  103) },
                    MsBasicItem { midi::Program(17, 103) },
                }
            },
        }
    },

    MsBasicItem {
        /*title=*/ u"Sound effects",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  120) },
            MsBasicItem { midi::Program(0,  121) },
            MsBasicItem { midi::Program(0,  122) },
            MsBasicItem { midi::Program(0,  123) },
            MsBasicItem { midi::Program(0,  124) },
            MsBasicItem { midi::Program(0,  125) },
            MsBasicItem { midi::Program(0,  126) },
            MsBasicItem { midi::Program(0,  127) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Percussion (chromatic)",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  8) },
            MsBasicItem { midi::Program(0,  9) },
            MsBasicItem { midi::Program(0,  10) },
            MsBasicItem { midi::Program(0,  11) },
            MsBasicItem { midi::Program(0,  12) },
            MsBasicItem { midi::Program(0,  13) },
            MsBasicItem { midi::Program(0,  14) },
            MsBasicItem { midi::Program(8,  14) },
            MsBasicItem { midi::Program(0,  15) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Percussion (mixed)",
        /*subItems=*/ {
            MsBasicItem { midi::Program(0,  55) },
            MsBasicItem { midi::Program(0,  112) },
            MsBasicItem { midi::Program(0,  113) },
            MsBasicItem { midi::Program(0,  114) },
            MsBasicItem { midi::Program(0,  115) },
            MsBasicItem { midi::Program(1,  115) },
            MsBasicItem { midi::Program(8,  115) },
            MsBasicItem { midi::Program(0,  116) },
            MsBasicItem { midi::Program(8,  116) },
            MsBasicItem { midi::Program(0,  117) },
            MsBasicItem { midi::Program(8,  117) },
            MsBasicItem { midi::Program(0,  118) },
            MsBasicItem { midi::Program(8,  118) },
            MsBasicItem { midi::Program(0,  119) },
        }
    },

    MsBasicItem {
        /*title=*/ u"Drumsets",
        /*subItems=*/ {
            MsBasicItem {
                /*title=*/ u"Standard kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 0) },
                    MsBasicItem { midi::Program(128, 1) },
                    MsBasicItem { midi::Program(128, 2) },
                    MsBasicItem { midi::Program(128, 3) },
                    MsBasicItem { midi::Program(128, 4) },
                    MsBasicItem { midi::Program(128, 5) },
                    MsBasicItem { midi::Program(128, 6) },
                    MsBasicItem { midi::Program(128, 7) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Room kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 8) },
                    MsBasicItem { midi::Program(128, 9) },
                    MsBasicItem { midi::Program(128, 10) },
                    MsBasicItem { midi::Program(128, 11) },
                    MsBasicItem { midi::Program(128, 12) },
                    MsBasicItem { midi::Program(128, 13) },
                    MsBasicItem { midi::Program(128, 14) },
                    MsBasicItem { midi::Program(128, 15) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Power kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 16) },
                    MsBasicItem { midi::Program(128, 17) },
                    MsBasicItem { midi::Program(128, 18) },
                    MsBasicItem { midi::Program(128, 19) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Electronic kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 24) },
                }
            },

            MsBasicItem {
                /*title=*/ u"TR-808 kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 25) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Jazz kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 32) },
                    MsBasicItem { midi::Program(128, 33) },
                    MsBasicItem { midi::Program(128, 34) },
                    MsBasicItem { midi::Program(128, 35) },
                    MsBasicItem { midi::Program(128, 36) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Brush kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 40) },
                    MsBasicItem { midi::Program(128, 41) },
                    MsBasicItem { midi::Program(128, 42) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Orchestra kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 48) },
                }
            },

            MsBasicItem {
                /*title=*/ u"Marching kits",
                /*subItems=*/ {
                    MsBasicItem { midi::Program(128, 56) },
                    MsBasicItem { midi::Program(128, 57) },
                    MsBasicItem { midi::Program(128, 58) },
                    MsBasicItem { midi::Program(128, 59) },
                    MsBasicItem { midi::Program(128, 95) },
                    MsBasicItem { midi::Program(128, 96) },
                }
            },
        }
    },
};
}

#endif // MU_PLAYBACK_MSBASICPRESETSCATEGORIES_H
