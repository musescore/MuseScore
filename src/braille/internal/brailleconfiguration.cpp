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

#include "brailleconfiguration.h"

#include "settings.h"

using namespace mu::braille;
using namespace muse;

namespace mu::engraving {
static const std::string module_name("braille");

static const Settings::Key BRAILLE_STATUS(module_name, "score/braille/status");
static const Settings::Key BRAILLE_TABLE(module_name, "score/braille/table");
static const Settings::Key BRAILLE_INTERVAL_DIRECTION(module_name, "score/braille/intervalDirection");

void BrailleConfiguration::init()
{
    settings()->setDefaultValue(BRAILLE_STATUS, Val(false));
    settings()->valueChanged(BRAILLE_STATUS).onReceive(this, [this](const Val&) {
        m_braillePanelEnabledChanged.notify();
    });
    settings()->setDefaultValue(BRAILLE_TABLE, Val("Unified English uncontracted braille [en-ueb-g1.ctb]"));
    settings()->valueChanged(BRAILLE_TABLE).onReceive(this, [this](const Val&) {
        m_brailleTableChanged.notify();
    });
    settings()->setDefaultValue(BRAILLE_INTERVAL_DIRECTION, Val(BrailleIntervalDirection::Auto));
    settings()->valueChanged(BRAILLE_INTERVAL_DIRECTION).onReceive(this, [this](const Val&) {
        m_intervalDirectionChanged.notify();
    });
}

muse::async::Notification BrailleConfiguration::braillePanelEnabledChanged() const
{
    return m_braillePanelEnabledChanged;
}

bool BrailleConfiguration::braillePanelEnabled() const
{
    return settings()->value(BRAILLE_STATUS).toBool();
}

void BrailleConfiguration::setBraillePanelEnabled(const bool enabled)
{
    settings()->setSharedValue(BRAILLE_STATUS, Val(enabled));
}

muse::async::Notification BrailleConfiguration::intervalDirectionChanged() const
{
    return m_intervalDirectionChanged;
}

BrailleIntervalDirection BrailleConfiguration::intervalDirection() const
{
    return settings()->value(BRAILLE_INTERVAL_DIRECTION).toEnum<BrailleIntervalDirection>();
}

void BrailleConfiguration::setIntervalDirection(const BrailleIntervalDirection direction)
{
    settings()->setSharedValue(BRAILLE_INTERVAL_DIRECTION, Val(direction));
}

muse::async::Notification BrailleConfiguration::brailleTableChanged() const
{
    return m_brailleTableChanged;
}

QString BrailleConfiguration::brailleTable() const
{
    return settings()->value(BRAILLE_TABLE).toQString();
}

void BrailleConfiguration::setBrailleTable(const QString& table)
{
    settings()->setSharedValue(BRAILLE_TABLE, Val(table));
}

QStringList BrailleConfiguration::brailleTableList() const
{
    return {
        "Afrikaans uncontracted braille [afr-za-g1.ctb]",
        "Afrikaans contracted braille [afr-za-g2.ctb]",
        "Arabic uncontracted braille [ar.tbl]",
        "Arabic contracted braille [ar-ar-g2.ctb]",
        "Arabic computer braille [ar-ar-comp8.utb]",
        "Assamese braille [as.tbl]",
        "Awadhi braille [awa.tbl]",
        "Bashkir braille [ba.utb]",
        "Belarusian braille [bel.utb]",
        "Belarusian computer braille [bel-comp.utb]",
        "Bulgarian computer braille [bg.tbl]",
        "Bulgarian braille [bg.utb]",
        "Bihari braille [bh.tbl]",
        "Bengali braille [bn.tbl]",
        "Tibetan computer braille [bo.tbl]",
        "Braj braille [bra.tbl]",
        "Catalan braille [ca.tbl]",
        "Cherokee braille [chr-us-g1.ctb]",
        "Coptic computer braille [cop-eg-comp8.utb]",
        "Kurdish braille [ckb.tbl]",
        "Czech braille [cs.tbl]",
        "Czech computer braille [cs-comp8.utb]",
        "Welsh uncontracted braille [cy-cy-g1.utb]",
        "Welsh contracted braille [cy.tbl]",
        "Danish computer braille (1993 standard) [da-dk-g08_1993.ctb]",
        "Danish 6-dot uncontracted braille (1993 standard) [da-dk-g16-lit_1993.ctb]",
        "Danish 6-dot uncontracted braille (1993 standard) [da-dk-g16_1993.ctb]",
        "Danish 8-dot uncontracted braille (1993 standard) [da-dk-g18_1993.ctb]",
        "Danish 6-dot contracted braille (1993 standard) [da-dk-g26-lit_1993.ctb]",
        "Danish 6-dot contracted braille (1993 standard) [da-dk-g26_1993.ctb]",
        "Danish 6-dot partially contracted braille (1993 standard) [da-dk-g26l-lit_1993.ctb]",
        "Danish 6-dot partially contracted braille (1993 standard) [da-dk-g26l_1993.ctb]",
        "Danish 8-dot contracted braille (1993 standard) [da-dk-g28_1993.ctb]",
        "Danish 8-dot partially contracted braille (1993 standard) [da-dk-g28l_1993.ctb]",
        "Danish computer braille (2022 standard) [da-dk-g08.ctb]",
        "Danish 6-dot uncontracted braille (2022 standard) [da-dk-g16.ctb]",
        "Danish 8-dot uncontracted braille (2022 standard) [da-dk-g18.ctb]",
        "Danish 6-dot contracted braille (2022 standard) [da-dk-g26.ctb]",
        "Danish 8-dot contracted braille (2022 standard) [da-dk-g28.ctb]",
        "German 8-dot computer braille [de-de-comp8.ctb]",
        "German 6-dot computer braille [de-comp6.utb]",
        "German uncontracted braille [de-g0.utb]",
        "German uncontracted braille with indication of capitals [de-g0-detailed.utb]",
        "German partially contracted braille [de-g1.ctb]",
        "German partially contracted braille with indication of capitals [de-g1-detailed.ctb]",
        "German contracted braille [de-g2.ctb]",
        "German contracted braille with indication of capitals [de-g2-detailed.ctb]",
        "Dravidian computer braille [dra.tbl]",
        "Greek braille [el.ctb]",
        "Greek internationalized braille as used by English speakers [grc-international-en.utb]",
        "Persian computer braille [fa-ir-comp8.ctb]",
        "Persian braille [fa-ir-g1.utb]",
        "Unified English uncontracted braille [en-ueb-g1.ctb]",
        "Unified English contracted braille [en-ueb-g2.ctb]",
        "English computer braille as used in Canada [en_CA.tbl]",
        "English uncontracted braille as used in the U.K. [en-gb-g1.utb]",
        "English contracted braille as used in the U.K. [en_GB.tbl]",
        "English computer braille as used in the U.K. [en-gb-comp8.ctb]",
        "North American Braille Computer Code [en-nabcc.utb]",
        "English uncontracted braille as used in the U.S. [en-us-g1.ctb]",
        "English 6-dot computer braille as used in the U.S. [en-us-comp6.ctb]",
        "English 8-dot computer braille as used in the U.S. [en_US-comp8-ext.tbl]",
        "English contracted braille as used in the U.S. [en_US.tbl]",
        "Esperanto braille [eo.tbl]",
        "Esperanto x-system braille [eo-g1-x-system.ctb]",
        "Spanish uncontracted braille [es.tbl]",
        "Spanish contracted braille [es-g2.ctb]",
        "Spanish computer braille [Es-Es-G0.utb]",
        "Estonian computer braille [et.tbl]",
        "Finnish braille [fi.utb]",
        "Finnish computer braille [fi-fi-8dot.ctb]",
        "French uncontracted braille [fr-bfu-comp6.utb]",
        "French computer braille [fr-bfu-comp8.utb]",
        "French contracted braille [fr-bfu-g2.ctb]",
        "Irish uncontracted braille [ga-g1.utb]",
        "Irish contracted braille [ga-g2.ctb]",
        "Scottish Gaelic computer braille [gd.tbl]",
        "Ethiopic braille [gez.tbl]",
        "Gondi braille [gon.tbl]",
        "Gujarati braille [gu.tbl]",
        "Hawaiian braille [haw-us-g1.ctb]",
        "Hebrew computer braille [he-IL-comp8.utb]",
        "Israeli braille [he-IL.utb]",
        "Hindi braille [hi.tbl]",
        "Croatian braille [hr-g1.tbl]",
        "Croatian computer braille [hr-comp8.tbl]",
        "Hungarian computer braille [hu-hu-comp8.ctb]",
        "Hungarian partially contracted braille [hu.tbl]",
        "Hungarian contracted braille [hu-hu-g2.ctb]",
        "Armenian computer braille [hy.tbl]",
        "Icelandic braille [is.tbl]",
        "Italian braille [it.tbl]",
        "Italian computer braille [it-it-comp8.utb]",
        "Inuktitut braille [iu-ca-g1.ctb]",
        "Kantenji [ja-kantenji.utb]",
        "Georgian braille [ka.utb]",
        "Khasi braille [kha.tbl]",
        "Kazakh braille [kk.utb]",
        "Khmer braille [km-g1.utb]",
        "Northern Kurdish braille [kmr.tbl]",
        "Kannada braille [kn.tbl]",
        "Korean contracted braille (2006 standard) [ko-2006-g2.ctb]",
        "Korean uncontracted braille (2006 standard) [ko-2006-g1.ctb]",
        "Korean contracted braille [ko-g2.ctb]",
        "Korean uncontracted braille [ko-g1.ctb]",
        "Konkani braille [kok.tbl]",
        "Kurukh braille [kru.tbl]",
        "Luganda braille [lg-ug-g1.utb]",
        "Lithuanian 8-dot braille [lt.tbl]",
        "Lithuanian 6-dot braille [lt-6dot.tbl]",
        "Latvian braille [lv.tbl]",
        "Maori braille [mao-nz-g1.ctb]",
        "Malayalam braille [ml.tbl]",
        "Mongolian uncontracted braille [mn-MN-g1.utb]",
        "Mongolian contracted braille [mn-MN-g2.ctb]",
        "Manipuri braille [mni.tbl]",
        "Marathi braille [mr.tbl]",
        "Malay braille [ms-my-g2.ctb]",
        "Maltese computer braille [mt.tbl]",
        "Munda braille [mun.tbl]",
        "Marwari braille [mwr.tbl]",
        "Burmese uncontracted braille [my-g1.utb]",
        "Burmese contracted braille [my-g2.ctb]",
        "Norwegian computer braille [no-no-comp8.ctb]",
        "Norwegian 6-dot uncontracted braille [no-no-g0.utb]",
        "Norwegian 8-dot uncontracted braille [no-no-8dot.utb]",
        "Norwegian 8-dot uncontracted braille with 6-dot fallback [no-no-8dot-fallback-6dot-g0.utb]",
        "Norwegian grade 1 contracted braille [no-no-g1.ctb]",
        "Norwegian grade 2 contracted braille [no-no-g2.ctb]",
        "Norwegian grade 3 contracted braille [no-no-g3.ctb]",
        "Nepali braille [ne.tbl]",
        "Dutch braille [nl.tbl]",
        "Dutch computer braille [nl-comp8.utb]",
        "Sepedi uncontracted braille [nso-za-g1.utb]",
        "Sepedi contracted braille [nso-za-g2.ctb]",
        "Chichewa braille [ny-mw.utb]",
        "Oriya braille [or.tbl]",
        "Punjabi braille [pa.tbl]",
        "Pali braille [pi.tbl]",
        "Polish computer braille [pl-pl-comp8.ctb]",
        "Polish braille [pl.tbl]",
        "Portuguese contracted braille [pt.tbl]",
        "Portuguese uncontracted braille [pt-pt-g1.utb]",
        "Portuguese computer braille [pt-pt-comp8.ctb]",
        "Romanian computer braille [ro.tbl]",
        "Russian computer braille [ru.ctb]",
        "Russian braille [ru-litbrl.ctb]",
        "Russian braille with indication of capitals [ru-litbrl-detailed.utb]",
        "Russian braille for program sources [ru-compbrl.ctb]",
        "Russian contracted braille [ru-ru-g1.ctb]",
        "Kinyarwanda braille [rw-rw-g1.utb]",
        "Sanskrit braille [sa.tbl]",
        "Yakut braille [sah.utb]",
        "Sindhi braille [sd.tbl]",
        "Slovak braille [sk-g1.ctb]",
        "Slovenian braille [sl.tbl]",
        "Slovenian computer braille [sl-si-comp8.ctb]",
        "Sesotho uncontracted braille [sot-za-g1.ctb]",
        "Sesotho contracted braille [sot-za-g2.ctb]",
        "Serbian braille [sr.tbl]",
        "Swedish computer braille (1996 standard) [sv-1996.ctb]",
        "Swedish computer braille (1989 standard) [sv-1989.ctb]",
        "Swedish uncontracted braille [sv-g0.utb]",
        "Swedish partially contracted braille [sv-g1.ctb]",
        "Swedish contracted braille [sv-g2.ctb]",
        "Swahili grade 1.2 contracted braille [sw-ke-g1-2.ctb]",
        "Swahili grade 1.3 contracted braille [sw-ke-g1-3.ctb]",
        "Swahili grade 1.4 contracted braille [sw-ke-g1-4.ctb]",
        "Swahili grade 1.5 contracted braille [sw-ke-g1-5.ctb]",
        "Swahili uncontracted braille [sw-ke-g1.utb]",
        "Swahili grade 2 contracted braille [sw-ke-g2.ctb]",
        "Tamil braille [ta-ta-g1.ctb]",
        "Tamil computer braille [ta.tbl]",
        "Telugu braille [te.tbl]",
        "Turkish computer braille [tr.tbl]",
        "Turkish braille [tr-g2.tbl]",
        "Setswana uncontracted braille [tsn-za-g1.ctb]",
        "Setswana contracted braille [tsn-za-g2.ctb]",
        "Tatar braille [tt.utb]",
        "Ukrainian braille [uk.utb]",
        "Ukrainian computer braille [uk-comp.utb]",
        "Uzbek braille [uz-g1.utb]",
        "Urdu uncontracted braille [ur-pk-g1.utb]",
        "Urdu contracted braille [ur-pk-g2.ctb]",
        "Tshivenda uncontracted braille [ve-za-g1.utb]",
        "Tshivenda contracted braille [ve-za-g2.ctb]",
        "Vietnamese computer braille [vi.ctb]",
        "Vietnamese uncontracted braille [vi-vn-g0.utb]",
        "Vietnamese partially contracted braille [vi-vn-g1.ctb]",
        "Vietnamese contracted braille [vi-vn-g2.ctb]",
        "Southern Vietnamese braille [vi-saigon-g1.ctb]",
        "isiXhosa uncontracted braille [xh-za-g1.utb]",
        "isiXhosa contracted braille [xh-za-g2.ctb]",
        "Chinese current braille without tones, for simplified Chinese characters [zh_CHN.tbl]",
        "Cantonese braille [zh_HK.tbl]",
        "Taiwanese bopomofo braille [zh-tw.ctb]",
        "Chinese common braille, for simplified Chinese characters [zhcn-cbs.ctb]",
        "Chinese current braille with tones [zhcn-g1.ctb]",
        "Chinese double-phonic braille [zhcn-g2.ctb]",
        "isiZulu uncontracted braille [zu-za-g1.utb]",
        "isiZulu contracted braille [zu-za-g2.ctb]",
    };
}
}
