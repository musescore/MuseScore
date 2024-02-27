/*
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod
 */

#include "hb-test.h"

#include <hb-ot.h>

/* Unit tests for hb-ot-tag.h */


/* https://docs.microsoft.com/en-us/typography/opentype/spec/scripttags */

static void
test_simple_tags (const char *s, hb_script_t script)
{
  hb_script_t tag;
  unsigned int count = 2;
  hb_tag_t t[2];

  g_test_message ("Testing script %c%c%c%c: tag %s", HB_UNTAG (hb_script_to_iso15924_tag (script)), s);
  tag = hb_tag_from_string (s, -1);

  hb_ot_tags_from_script_and_language (script,
				       HB_LANGUAGE_INVALID,
				       &count, t, NULL, NULL);

  if (count)
    g_assert_cmphex (t[0], ==, tag);
  else
    g_assert_cmphex (HB_TAG_CHAR4 ("DFLT"), ==, tag);

  g_assert_cmphex (hb_ot_tag_to_script (tag), ==, script);
}

static void
test_script_tags_from_language (const char *s, const char *lang_s, hb_script_t script)
{
  hb_script_t tag;
  unsigned int count = 1;
  hb_tag_t t;

  g_test_message ("Testing script %c%c%c%c: script tag %s, language tag %s", HB_UNTAG (hb_script_to_iso15924_tag (script)), s, lang_s);
  tag = hb_tag_from_string (s, -1);

  hb_ot_tags_from_script_and_language (script, hb_language_from_string (lang_s, -1), &count, &t, NULL, NULL);

  if (count != 0)
  {
    g_assert_cmpuint (count, ==, 1);
    g_assert_cmphex (t, ==, tag);
  }
}

static void
test_indic_tags (const char *s1, const char *s2, const char *s3, hb_script_t script)
{
  hb_script_t tag1, tag2, tag3;
  hb_tag_t t[3];
  unsigned int count = 3;

  g_test_message ("Testing script %c%c%c%c: USE tag %s, new tag %s, old tag %s", HB_UNTAG (hb_script_to_iso15924_tag (script)), s1, s2, s3);
  tag1 = hb_tag_from_string (s1, -1);
  tag2 = hb_tag_from_string (s2, -1);
  tag3 = hb_tag_from_string (s3, -1);

  hb_ot_tags_from_script_and_language (script,
				       HB_LANGUAGE_INVALID,
				       &count, t, NULL, NULL);

  g_assert_cmpuint (count, ==, 3);
  g_assert_cmphex (t[0], ==, tag1);
  g_assert_cmphex (t[1], ==, tag2);
  g_assert_cmphex (t[2], ==, tag3);

  g_assert_cmphex (hb_ot_tag_to_script (tag1), ==, script);
  g_assert_cmphex (hb_ot_tag_to_script (tag2), ==, script);
  g_assert_cmphex (hb_ot_tag_to_script (tag3), ==, script);
}

static void
test_ot_tag_script_degenerate (void)
{
  hb_tag_t t[2];
  unsigned int count = 2;

  g_assert_cmphex (HB_TAG_CHAR4 ("DFLT"), ==, HB_OT_TAG_DEFAULT_SCRIPT);

  /* HIRAGANA and KATAKANA both map to 'kana' */
  test_simple_tags ("kana", HB_SCRIPT_KATAKANA);

  hb_ot_tags_from_script_and_language (HB_SCRIPT_HIRAGANA,
				       HB_LANGUAGE_INVALID,
				       &count, t, NULL, NULL);

  g_assert_cmpuint (count, ==, 1);
  g_assert_cmphex (t[0], ==, HB_TAG_CHAR4 ("kana"));

  test_simple_tags ("DFLT", HB_SCRIPT_INVALID);

  /* Spaces are replaced */
  g_assert_cmphex (hb_ot_tag_to_script (HB_TAG_CHAR4 ("be  ")), ==, hb_script_from_string ("Beee", -1));
}

static void
test_ot_tag_script_simple (void)
{
  /* Arbitrary non-existent script */
  test_simple_tags ("wwyz", hb_script_from_string ("wWyZ", -1));

  /* These we don't really care about */
  test_simple_tags ("zyyy", HB_SCRIPT_COMMON);
  test_simple_tags ("zinh", HB_SCRIPT_INHERITED);
  test_simple_tags ("zzzz", HB_SCRIPT_UNKNOWN);

  test_simple_tags ("arab", HB_SCRIPT_ARABIC);
  test_simple_tags ("copt", HB_SCRIPT_COPTIC);
  test_simple_tags ("kana", HB_SCRIPT_KATAKANA);
  test_simple_tags ("latn", HB_SCRIPT_LATIN);

  test_simple_tags ("math", HB_SCRIPT_MATH);

  /* These are trickier since their OT script tags have space. */
  test_simple_tags ("lao ", HB_SCRIPT_LAO);
  test_simple_tags ("yi  ", HB_SCRIPT_YI);
  /* Unicode-5.0 additions */
  test_simple_tags ("nko ", HB_SCRIPT_NKO);
  /* Unicode-5.1 additions */
  test_simple_tags ("vai ", HB_SCRIPT_VAI);

  /* https://docs.microsoft.com/en-us/typography/opentype/spec/scripttags */

  /* Unicode-5.2 additions */
  test_simple_tags ("mtei", HB_SCRIPT_MEETEI_MAYEK);
  /* Unicode-6.0 additions */
  test_simple_tags ("mand", HB_SCRIPT_MANDAIC);
}

static void
test_ot_tag_script_from_language (void)
{
  test_script_tags_from_language (NULL, NULL, HB_SCRIPT_INVALID);
  test_script_tags_from_language (NULL, "en", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("copt", "en", HB_SCRIPT_COPTIC);
  test_script_tags_from_language (NULL, "x-hbsc", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("copt", "x-hbsc", HB_SCRIPT_COPTIC);
  test_script_tags_from_language (NULL, "x-hbsc-", HB_SCRIPT_INVALID);
  test_script_tags_from_language (NULL, "x-hbsc-1", HB_SCRIPT_INVALID);
  test_script_tags_from_language (NULL, "x-hbsc-1a", HB_SCRIPT_INVALID);
  test_script_tags_from_language (NULL, "x-hbsc-1a2b3c4x", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("2lon", "x-hbsc-326c6f6e67", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("abc ", "x-hbscabc", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("deva", "x-hbscdeva", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev2", "x-hbscdev2", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev3", "x-hbscdev3", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev3", "x-hbsc-64657633", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("copt", "x-hbotpap0-hbsccopt", HB_SCRIPT_INVALID);
  test_script_tags_from_language (NULL, "en-x-hbsc", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("copt", "en-x-hbsc", HB_SCRIPT_COPTIC);
  test_script_tags_from_language ("abc ", "en-x-hbscabc", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("deva", "en-x-hbscdeva", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev2", "en-x-hbscdev2", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev3", "en-x-hbscdev3", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("dev3", "en-x-hbsc-64657633", HB_SCRIPT_INVALID);
  test_script_tags_from_language ("copt", "en-x-hbotpap0-hbsccopt", HB_SCRIPT_INVALID);
}

static void
test_ot_tag_script_indic (void)
{
  test_indic_tags ("bng3", "bng2", "beng", HB_SCRIPT_BENGALI);
  test_indic_tags ("dev3", "dev2", "deva", HB_SCRIPT_DEVANAGARI);
  test_indic_tags ("gjr3", "gjr2", "gujr", HB_SCRIPT_GUJARATI);
  test_indic_tags ("gur3", "gur2", "guru", HB_SCRIPT_GURMUKHI);
  test_indic_tags ("knd3", "knd2", "knda", HB_SCRIPT_KANNADA);
  test_indic_tags ("mlm3", "mlm2", "mlym", HB_SCRIPT_MALAYALAM);
  test_indic_tags ("ory3", "ory2", "orya", HB_SCRIPT_ORIYA);
  test_indic_tags ("tml3", "tml2", "taml", HB_SCRIPT_TAMIL);
  test_indic_tags ("tel3", "tel2", "telu", HB_SCRIPT_TELUGU);
}



/* https://docs.microsoft.com/en-us/typography/opentype/spec/languagetags */

static void
test_language_two_way (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);
  hb_tag_t tag2;
  unsigned int count = 1;

  g_test_message ("Testing language %s <-> tag %s", lang_s, tag_s);

  hb_ot_tags_from_script_and_language (HB_SCRIPT_INVALID,
				       lang,
				       NULL, NULL, &count, &tag2);

  if (count)
    g_assert_cmphex (tag, ==, tag2);
  else
    g_assert_cmphex (tag, ==, HB_TAG_CHAR4 ("dflt"));
  g_assert (lang == hb_ot_tag_to_language (tag));
}

static void
test_tag_from_language (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);
  hb_tag_t tag2;
  unsigned int count = 1;

  g_test_message ("Testing language %s -> tag %s", lang_s, tag_s);

  hb_ot_tags_from_script_and_language (HB_SCRIPT_INVALID,
				       lang,
				       NULL, NULL, &count, &tag2);

  if (count)
    g_assert_cmphex (tag, ==, tag2);
  else
    g_assert_cmphex (tag, ==, HB_TAG_CHAR4 ("dflt"));
}

static void
test_tag_to_language (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);

  g_test_message ("Testing tag %s -> language %s", tag_s, lang_s);

  g_assert (lang == hb_ot_tag_to_language (tag));
}

static void
test_tags_to_script_and_language (const char *script_tag_s,
				  const char *lang_tag_s,
				  const char *script_s,
				  const char *lang_s)
{
  hb_script_t actual_script[1];
  hb_language_t actual_lang[1];
  hb_tag_t script_tag = hb_tag_from_string (script_tag_s, -1);
  hb_tag_t lang_tag = hb_tag_from_string (lang_tag_s, -1);
  hb_ot_tags_to_script_and_language (script_tag, lang_tag, actual_script, actual_lang);
  g_assert_cmphex (*actual_script, ==, hb_tag_from_string (script_s, -1));
  g_assert_cmpstr (hb_language_to_string (*actual_lang), ==, lang_s);
}

static void
test_ot_tags_to_script_and_language (void)
{
  test_tags_to_script_and_language ("DFLT", "ENG", "", "en-x-hbsc-44464c54");
  test_tags_to_script_and_language ("latn", "ENG", "Latn", "en");
  test_tags_to_script_and_language ("deva", "MAR", "Deva", "mr-x-hbsc-64657661");
  test_tags_to_script_and_language ("dev2", "MAR", "Deva", "mr-x-hbsc-64657632");
  test_tags_to_script_and_language ("dev3", "MAR", "Deva", "mr");
  test_tags_to_script_and_language ("qaa", "QTZ0", "Qaaa", "x-hbot-51545a30-hbsc-71616120");
}

static void
test_ot_tag_language (void)
{
  g_assert_cmphex (HB_TAG_CHAR4 ("dflt"), ==, HB_OT_TAG_DEFAULT_LANGUAGE);
  test_language_two_way ("dflt", NULL);

  test_language_two_way ("ALT", "alt");

  test_language_two_way ("ARA", "ar");

  test_language_two_way ("AZE", "az");
  test_tag_from_language ("AZE", "az-ir");
  test_tag_from_language ("AZE", "az-az");

  test_language_two_way ("ENG", "en");
  test_tag_from_language ("ENG", "en_US");

  test_language_two_way ("CJA", "cja-x-hbot-434a4120"); /* Western Cham */
  test_language_two_way ("CJM", "cjm-x-hbot-434a4d20"); /* Eastern Cham */
  test_tag_from_language ("CJM", "cjm");
  test_language_two_way ("EVN", "eve");

  test_language_two_way ("HAL", "cfm"); /* BCP47 and current ISO639-3 code for Halam/Falam Chin */
  test_tag_from_language ("HAL", "flm"); /* Retired ISO639-3 code for Halam/Falam Chin */

  test_language_two_way ("HYE0", "hy");
  test_language_two_way ("HYE", "hyw");

  test_tag_from_language ("QIN", "bgr"); /* Bawm Chin */
  test_tag_from_language ("QIN", "cbl"); /* Bualkhaw Chin */
  test_tag_from_language ("QIN", "cka"); /* Khumi Awa Chin */
  test_tag_from_language ("QIN", "cmr"); /* Mro-Khimi Chin */
  test_tag_from_language ("QIN", "cnb"); /* Chinbon Chin */
  test_tag_from_language ("QIN", "cnh"); /* Hakha Chin */
  test_tag_from_language ("QIN", "cnk"); /* Khumi Chin */
  test_tag_from_language ("QIN", "cnw"); /* Ngawn Chin */
  test_tag_from_language ("QIN", "csh"); /* Asho Chin */
  test_tag_from_language ("QIN", "csy"); /* Siyin Chin */
  test_tag_from_language ("QIN", "ctd"); /* Tedim Chin */
  test_tag_from_language ("QIN", "czt"); /* Zotung Chin */
  test_tag_from_language ("QIN", "dao"); /* Daai Chin */
  test_tag_from_language ("QIN", "hlt"); /* Matu Chin */
  test_tag_from_language ("QIN", "mrh"); /* Mara Chin */
  test_tag_from_language ("QIN", "pck"); /* Paite Chin */
  test_tag_from_language ("QIN", "sez"); /* Senthang Chin */
  test_tag_from_language ("QIN", "tcp"); /* Tawr Chin */
  test_tag_from_language ("QIN", "tcz"); /* Thado Chin */
  test_tag_from_language ("QIN", "yos"); /* Yos, deprecated by IANA in favor of Zou [zom] */
  test_tag_from_language ("QIN", "zom"); /* Zou */
  test_tag_to_language ("QIN", "bgr");   /* no single BCP47 tag for Chin; picking Bawm Chin */

  test_language_two_way ("FAR", "fa");
  test_tag_from_language ("FAR", "fa_IR");

  test_language_two_way ("MNK", "man"); /* Mandingo [macrolanguage] */

  test_language_two_way ("SWA", "aii"); /* Swadaya Aramaic */

  test_language_two_way ("SYR", "syr"); /* Syriac [macrolanguage] */
  test_tag_from_language ("SYR", "amw"); /* Western Neo-Aramaic */
  test_tag_from_language ("SYR", "cld"); /* Chaldean Neo-Aramaic */
  test_tag_from_language ("SYR", "syc"); /* Classical Syriac */

  test_language_two_way ("TUA", "tru"); /* Turoyo Aramaic */

  test_tag_from_language ("ZHS", "zh"); /* Chinese */
  test_tag_from_language ("ZHS", "zh-cn"); /* Chinese (China) */
  test_tag_from_language ("ZHS", "zh-sg"); /* Chinese (Singapore) */
  test_tag_from_language ("ZHTM", "zh-mo"); /* Chinese (Macao) */
  test_tag_from_language ("ZHTM", "zh-hant-mo"); /* Chinese (Macao) */
  test_tag_from_language ("ZHS", "zh-hans-mo"); /* Chinese (Simplified, Macao) */
  test_language_two_way ("ZHH", "zh-HK"); /* Chinese (Hong Kong) */
  test_tag_from_language ("ZHH", "zH-HanT-hK"); /* Chinese (Hong Kong) */
  test_tag_from_language ("ZHS", "zH-HanS-hK"); /* Chinese (Simplified, Hong Kong) */
  test_tag_from_language ("ZHT", "zh-tw"); /* Chinese (Taiwan) */
  test_language_two_way ("ZHS", "zh-Hans"); /* Chinese (Simplified) */
  test_language_two_way ("ZHT", "zh-Hant"); /* Chinese (Traditional) */
  test_tag_from_language ("ZHS", "zh-xx"); /* Chinese (Other) */

  test_tag_from_language ("ZHS", "zh-Hans-TW");

  test_tag_from_language ("ZHH", "yue");
  test_tag_from_language ("ZHH", "yue-Hant");
  test_tag_from_language ("ZHS", "yue-Hans");

  test_language_two_way ("ABC", "abc-x-hbot-41424320");
  test_language_two_way ("ABCD", "x-hbot-41424344");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbotabc");
  test_tag_from_language ("ABCD", "asdf-asdf-wer-x-hbotabcd");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbot-41424320-zxc");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbot-41424320");
  test_tag_from_language ("ABCD", "asdf-asdf-wer-x-hbot-41424344");

  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot");
  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot-zxc");
  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot-zxc-414243");
  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot-414243");
  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot-4142432");

  test_tag_from_language ("dflt", "xy");
  test_tag_from_language ("XYZ", "xyz"); /* Unknown ISO 639-3 */
  test_tag_from_language ("XYZ", "xyz-qw"); /* Unknown ISO 639-3 */

  /*
   * Invalid input. The precise answer does not matter, as long as it
   * does not crash or get into an infinite loop.
   */
  test_tag_from_language ("IPPH", "-fonipa");

  /*
   * Tags that contain "-fonipa" as a substring but which do not contain
   * the subtag "fonipa".
   */
  test_tag_from_language ("ENG", "en-fonipax");
  test_tag_from_language ("ENG", "en-x-fonipa");
  test_tag_from_language ("ENG", "en-a-fonipa");
  test_tag_from_language ("ENG", "en-a-qwe-b-fonipa");

  /* International Phonetic Alphabet */
  test_tag_from_language ("IPPH", "en-fonipa");
  test_tag_from_language ("IPPH", "en-fonipax-fonipa");
  test_tag_from_language ("IPPH", "rm-CH-fonipa-sursilv-x-foobar");
  test_language_two_way ("IPPH", "und-fonipa");
  test_tag_from_language ("IPPH", "zh-fonipa");

  /* North American Phonetic Alphabet (Americanist Phonetic Notation) */
  test_tag_from_language ("APPH", "en-fonnapa");
  test_tag_from_language ("APPH", "chr-fonnapa");
  test_language_two_way ("APPH", "und-fonnapa");

  /* Khutsuri Georgian */
  test_tag_from_language ("KGE", "ka-Geok");
  test_language_two_way ("KGE", "und-Geok");

  /* Irish Traditional */
  test_language_two_way ("IRT", "ga-Latg");

  /* Moldavian */
  test_language_two_way ("MOL", "ro-MD");

  /* Polytonic Greek */
  test_language_two_way ("PGR", "el-polyton");
  test_tag_from_language ("PGR", "el-CY-polyton");

  /* Estrangela Syriac */
  test_tag_from_language ("SYRE", "aii-Syre");
  test_tag_from_language ("SYRE", "de-Syre");
  test_tag_from_language ("SYRE", "syr-Syre");
  test_language_two_way ("SYRE", "und-Syre");

  /* Western Syriac */
  test_tag_from_language ("SYRJ", "aii-Syrj");
  test_tag_from_language ("SYRJ", "de-Syrj");
  test_tag_from_language ("SYRJ", "syr-Syrj");
  test_language_two_way ("SYRJ", "und-Syrj");

  /* Eastern Syriac */
  test_tag_from_language ("SYRN", "aii-Syrn");
  test_tag_from_language ("SYRN", "de-Syrn");
  test_tag_from_language ("SYRN", "syr-Syrn");
  test_language_two_way ("SYRN", "und-Syrn");

  /* Test that x-hbot overrides the base language */
  test_tag_from_language ("ABC", "fa-x-hbotabc-hbot-41686121-zxc");
  test_tag_from_language ("ABC", "fa-ir-x-hbotabc-hbot-41686121-zxc");
  test_tag_from_language ("ABC", "zh-x-hbotabc-hbot-41686121-zxc");
  test_tag_from_language ("ABC", "zh-cn-x-hbotabc-hbot-41686121-zxc");
  test_tag_from_language ("ABC", "zh-xy-x-hbotabc-hbot-41686121-zxc");
  test_tag_from_language ("ABC", "xyz-xy-x-hbotabc-hbot-41686121-zxc");

  test_tag_from_language ("Aha!", "fa-x-hbot-41686121-hbotabc-zxc");
  test_tag_from_language ("Aha!", "fa-ir-x-hbot-41686121-hbotabc-zxc");
  test_tag_from_language ("Aha!", "zh-x-hbot-41686121-hbotabc-zxc");
  test_tag_from_language ("Aha!", "zh-cn-x-hbot-41686121-hbotabc-zxc");
  test_tag_from_language ("Aha!", "zh-xy-x-hbot-41686121-hbotabc-zxc");
  test_tag_from_language ("Aha!", "xyz-xy-x-hbot-41686121-hbotabc-zxc");

  /* Invalid x-hbot */
  test_tag_from_language ("dflt", "x-hbot");
  test_tag_from_language ("dflt", "x-hbot-");
  test_tag_from_language ("dflt", "x-hbot-1");
  test_tag_from_language ("dflt", "x-hbot-1a");
  test_tag_from_language ("dflt", "x-hbot-1a2b3c4x");
  test_tag_from_language ("2lon", "x-hbot-326c6f6e67");

  /* Unnormalized BCP 47 tags */
  test_tag_from_language ("ARA", "ar-aao");
  test_tag_from_language ("JBO", "art-lojban");
  test_tag_from_language ("KOK", "kok-gom");
  test_tag_from_language ("LTZ", "i-lux");
  test_tag_from_language ("MNG", "drh");
  test_tag_from_language ("MOR", "ar-ary");
  test_tag_from_language ("MOR", "ar-ary-DZ");
  test_tag_from_language ("NOR", "no-bok");
  test_tag_from_language ("NYN", "no-nyn");
  test_tag_from_language ("ZHS", "i-hak");
  test_tag_from_language ("ZHS", "zh-guoyu");
  test_tag_from_language ("ZHS", "zh-min");
  test_tag_from_language ("ZHS", "zh-min-nan");
  test_tag_from_language ("ZHS", "zh-xiang");

  /* BCP 47 tags that look similar to unrelated language system tags */
  test_tag_from_language ("SQI", "als");
  test_tag_from_language ("dflt", "far");

  /* A UN M.49 region code, not an extended language subtag */
  test_tag_from_language ("ARA", "ar-001");

  /* An invalid tag */
  test_tag_from_language ("TRK", "tr@foo=bar");
}

static void
test_tags (hb_script_t  script,
	   const char  *lang_s,
	   unsigned int script_count,
	   unsigned int language_count,
	   unsigned int expected_script_count,
	   unsigned int expected_language_count,
	   ...)
{
  va_list expected_tags;
  unsigned int i;
  hb_tag_t *script_tags = malloc (script_count * sizeof (hb_tag_t));
  hb_tag_t *language_tags = malloc (language_count * sizeof (hb_tag_t));
  hb_language_t lang;
  g_assert (script_tags);
  g_assert (language_tags);
  lang = hb_language_from_string (lang_s, -1);
  va_start (expected_tags, expected_language_count);

  hb_ot_tags_from_script_and_language (script, lang, &script_count, script_tags, &language_count, language_tags);

  g_assert_cmpuint (script_count, ==, expected_script_count);
  g_assert_cmpuint (language_count, ==, expected_language_count);

  for (i = 0; i < script_count + language_count; i++)
  {
    hb_tag_t expected_tag = hb_tag_from_string (va_arg (expected_tags, const char *), -1);
    hb_tag_t actual_tag = i < script_count ? script_tags[i] : language_tags[i - script_count];
    g_assert_cmphex (actual_tag, ==, expected_tag);
  }

  free (script_tags);
  free (language_tags);
  va_end (expected_tags);
}

static void
test_ot_tag_full (void)
{
  test_tags (HB_SCRIPT_INVALID, "en", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 0, 1, "ENG");
  test_tags (HB_SCRIPT_INVALID, "en-x-hbscdflt", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 1, 1, "DFLT", "ENG");
  test_tags (HB_SCRIPT_LATIN, "en", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 1, 1, "latn", "ENG");
  test_tags (HB_SCRIPT_LATIN, "en", 0, 0, 0, 0);
  test_tags (HB_SCRIPT_INVALID, "und-fonnapa", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 0, 1, "APPH");
  test_tags (HB_SCRIPT_INVALID, "en-fonnapa", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 0, 1, "APPH");
  test_tags (HB_SCRIPT_INVALID, "x-hbot1234-hbsc5678", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 1, 1, "5678", "1234");
  test_tags (HB_SCRIPT_INVALID, "x-hbsc5678-hbot1234", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 1, 1, "5678", "1234");
  test_tags (HB_SCRIPT_MALAYALAM, "ml", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 3, 2, "mlm3", "mlm2", "mlym", "MAL", "MLR");
  test_tags (HB_SCRIPT_MALAYALAM, "ml", 1, 1, 1, 1, "mlm3", "MAL");
  test_tags (HB_SCRIPT_MYANMAR, "und", HB_OT_MAX_TAGS_PER_SCRIPT, 0, 2, 0, "mym2", "mymr");
  test_tags (HB_SCRIPT_INVALID, "xyz", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 0, 1, "XYZ");
  test_tags (HB_SCRIPT_INVALID, "xy", HB_OT_MAX_TAGS_PER_SCRIPT, HB_OT_MAX_TAGS_PER_LANGUAGE, 0, 0);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_ot_tag_script_degenerate);
  hb_test_add (test_ot_tag_script_simple);
  hb_test_add (test_ot_tag_script_from_language);
  hb_test_add (test_ot_tag_script_indic);

  hb_test_add (test_ot_tags_to_script_and_language);

  hb_test_add (test_ot_tag_language);

  hb_test_add (test_ot_tag_full);

  return hb_test_run();
}
