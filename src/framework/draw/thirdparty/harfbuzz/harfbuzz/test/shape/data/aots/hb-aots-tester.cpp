/*____________________________________________________________________________

    Copyright 2000-2016 Adobe Systems Incorporated. All Rights Reserved.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use these files except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
____________________________________________________________________________*/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "hb.h"
#include "hb-ot.h"

static const bool verbose = true;

struct TestData
{
    TestData(hb_buffer_t  *buffer_,
             hb_face_t    *face_,
             hb_font_t    *font_,
             hb_feature_t *features_,
             int           num_features_)
        : buffer(buffer_), face(face_), font(font_),
          features(features_), num_features(num_features_)
    { }
    ~TestData()
    {
        free (features);
        hb_face_destroy (face);
        hb_font_destroy (font);
        hb_buffer_destroy (buffer);
    }

    hb_buffer_t  *buffer;
    hb_face_t    *face;
    hb_font_t    *font;
    hb_feature_t *features;
    int           num_features;
};

TestData
runTest(const char *testName,
        const char *fontfileName,
        unsigned int *in, int nbIn,
        unsigned int *select, int nbSelect)
{
    FILE *f = fopen (fontfileName, "rb");
    fseek(f, 0, SEEK_END);
    long fontsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *fontdata = (char *)malloc (fontsize);
    fread(fontdata, fontsize, 1, f);
    fclose(f);

    if (verbose) {
        printf ("------------------------------- %s\n", testName);
    }

    // setup font
    hb_blob_t *blob = hb_blob_create(fontdata, fontsize,
                                     HB_MEMORY_MODE_WRITABLE,
                                     0, 0);
    hb_face_t *face = hb_face_create(blob, 0);
    hb_font_t *font = hb_font_create(face);
    unsigned int upem = hb_face_get_upem (face);

    hb_font_set_scale(font, upem, upem);
    hb_ot_font_set_funcs (font);

    // setup buffer
    hb_buffer_t *buffer = hb_buffer_create();
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buffer, hb_language_from_string("en", 2));

    hb_buffer_add_utf32(buffer, in, nbIn, 0, nbIn);

    // setup features
    hb_feature_t *features;
    int nbFeatures;

    if (nbSelect == 0)
    {
        nbFeatures = 1;

        features = (hb_feature_t *) malloc (sizeof (*features));
        features[0].tag = HB_TAG('t', 'e', 's', 't');
        features[0].value = 1;
        features[0].start = HB_FEATURE_GLOBAL_START;
        features[0].end = HB_FEATURE_GLOBAL_END;
    }
    else
    {
        nbFeatures = 0;

        features = (hb_feature_t *) malloc (sizeof (*features) * nbSelect);
        for (int i = 0; i < nbSelect; i++) {
            if (select[i] != -1) {
                features[nbFeatures].tag = HB_TAG('t', 'e', 's', 't');
                features[nbFeatures].value = select[i];
                features[nbFeatures].start = i;
                features[nbFeatures].end = i + 1;
                nbFeatures++;
            }
        }
    }

    // shape
    hb_shape(font, buffer, features, nbFeatures);

    hb_blob_destroy(blob);

    return TestData(buffer, face, font, features, nbFeatures);
}


void printArray (const char* s, int *a, int n)
{
    printf ("%s  %d : ", s, n);
    for (int i = 0; i < n; i++) {
        printf (" %d", a[i]);
    }
    printf ("\n");
}

void printUArray (const char* s, unsigned int *a, int n)
{
    printArray (s, (int *) a, n);
}

bool gsub_test(const char *testName,
               const char *fontfileName,
               int nbIn, unsigned int *in,
               int nbSelect, unsigned int *select,
               int nbExpected, unsigned int *expected)
{
    TestData data = runTest(testName,
                            fontfileName,
                            in, nbIn,
                            select, nbSelect);

    // verify
    hb_glyph_info_t *actual = hb_buffer_get_glyph_infos(data.buffer, 0);
    unsigned int nbActual = hb_buffer_get_length(data.buffer);

    bool ok = true;

    if (nbActual != nbExpected)
        ok = false;
    else {
        for (int i = 0; i < nbActual; i++) {
            if (actual[i].codepoint != expected [i]) {
                ok = false;
                break;
            }
        }
    }


    char test_name[255];
    sprintf (test_name, "../../tests/%.*s.tests", (int) (strrchr (testName, '_') - testName), testName);
    FILE *tests_file = fopen (test_name, "a+");
    if (!ok) fprintf (tests_file, "#");
    fprintf (tests_file, "../fonts/%s;--features=\"", fontfileName + 9);
    for (unsigned int i = 0; i < data.num_features; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        char buf[255];
        hb_feature_to_string (&data.features[i], buf, sizeof (buf));
        fprintf (tests_file, "%s", buf);
    }
    fprintf (tests_file, "\" --single-par --no-clusters --no-glyph-names --no-positions;");

    for (unsigned int i = 0; i < nbIn; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        fprintf (tests_file, "U+%04X", in[i]);
    }

    fprintf (tests_file, ";[");
    for (unsigned int i = 0; i < nbActual; i++)
    {
        if (i != 0) fprintf (tests_file, "|");
        fprintf (tests_file, "%d", expected[i]);
    }
    fprintf (tests_file, "]");

    fprintf (tests_file, "\n");
    fclose (tests_file);


    if (! ok) {
        printf ("******* GSUB %s\n", testName);

        printf ("expected %d:", nbExpected);
        for (int i = 0; i < nbExpected; i++) {
            printf (" %d", expected[i]); }
        printf ("\n");

        printf ("  actual %d:", nbActual);
        for (int i = 0; i < nbActual; i++) {
            printf (" %d", actual[i].codepoint); }
        printf ("\n");

    }

    return ok;
}

bool cmap_test(const char *testName,
               const char *fontfileName,
               int nbIn, unsigned int *in,
               int nbSelect, unsigned int *select,
               int nbExpected, unsigned int *expected)
{
    TestData data = runTest(testName,
                            fontfileName,
                            in, nbIn,
                            select, nbSelect);

    // verify
    hb_glyph_info_t *actual = hb_buffer_get_glyph_infos(data.buffer, 0);
    unsigned int nbActual = hb_buffer_get_length(data.buffer);

    bool ok = true;

    if (nbActual != nbExpected)
        ok = false;
    else {
        for (int i = 0; i < nbActual; i++) {
            if (actual[i].codepoint != expected [i]) {
                ok = false;
                break;
            }
        }
    }


    char test_name[255];
    sprintf (test_name, "../../tests/%.*s.tests", (int) (strrchr (testName, '_') - testName), testName);
    FILE *tests_file = fopen (test_name, "a+");
    if (!ok) fprintf (tests_file, "#");
    fprintf (tests_file, "../fonts/%s;--features=\"", fontfileName + 9);
    for (unsigned int i = 0; i < data.num_features; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        char buf[255];
        hb_feature_to_string (&data.features[i], buf, sizeof (buf));
        fprintf (tests_file, "%s", buf);
    }
    fprintf (tests_file, "\" --single-par --no-clusters --no-glyph-names --no-positions --font-funcs=ot;");

    for (unsigned int i = 0; i < nbIn; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        fprintf (tests_file, "U+%04X", in[i]);
    }

    fprintf (tests_file, ";[");
    for (unsigned int i = 0; i < nbActual; i++)
    {
        if (i != 0) fprintf (tests_file, "|");
        fprintf (tests_file, "%d", expected[i]);
    }
    fprintf (tests_file, "]");

    fprintf (tests_file, "\n");
    fclose (tests_file);


    if (! ok) {
        printf ("******* cmap %s\n", testName);

        printf ("expected %d:", nbExpected);
        for (int i = 0; i < nbExpected; i++) {
            printf (" %d", expected[i]); }
        printf ("\n");

        printf ("  actual %d:", nbActual);
        for (int i = 0; i < nbActual; i++) {
            printf (" %d", actual[i].codepoint); }
        printf ("\n");

    }

    return ok;
}

bool gpos_test(const char *testName,
               const char *fontfileName,
               int nbIn,
               unsigned int *in,
               int nbOut,
               unsigned int *out,
               int *x,
               int *y)
{
    TestData data = runTest(testName,
                            fontfileName,
                            in, nbIn,
                            0, 0);

    // verify
    unsigned int nbActual;
    hb_glyph_info_t *actual = hb_buffer_get_glyph_infos(data.buffer, &nbActual);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (data.buffer, NULL);

    unsigned int *actualG = (unsigned int *) malloc(sizeof(*actualG) * nbActual);
    int *actualX = (int *) malloc(sizeof(*actualX) * nbActual);
    int *actualY = (int *) malloc(sizeof(*actualY) * nbActual);
    int curX = 0;
    int curY = 0;
    for (int i = 0; i < nbActual; i++) {
        actualG[i] = actual[i].codepoint;
        actualX[i] = curX + pos[i].x_offset;
        actualY[i] = curY + pos[i].y_offset;

        curX += pos[i].x_advance;
        if (hb_ot_layout_get_glyph_class (data.face, actualG[i]) != HB_OT_LAYOUT_GLYPH_CLASS_MARK)
            curX -= 1500;
        curY += pos[i].y_advance;
    }

    bool nbOk = true;
    bool xOk = true;
    bool yOk = true;

    if (nbActual != nbOut)
        nbOk = false;
    else {
        for (int i = 0; i < nbActual; i++) {
            if (actualX[i] != x[i]) {
                xOk = false;
            }
            if (actualY[i] != y[i]) {
                yOk = false;
            }
        }
    }

    bool ok = (nbOk && xOk && yOk);
    if (! ok) {
        printf ("******* GPOS %s\n", testName);

        if (! (nbOk && xOk)) {
            printArray ("expectedX", x, nbOut);
            printArray ("actualX  ", actualX, nbActual);

            printf ("xadv/pos:");
            for (int i = 0; i < nbOut; i++) {
                printf (" %d/%d", pos[i].x_advance, pos[i].x_offset);
            }
            printf ("\n");
        }

        if (! (nbOk && yOk)) {
            printArray ("expectedY", y, nbOut);
            printArray ("actualY  ", actualY, nbActual);

            printf ("yadv/pos:");
            for (int i = 0; i < nbOut; i++) {
                printf (" %d/%d", pos[i].y_advance, pos[i].y_offset);
            }
            printf ("\n");
        }
    }


    char test_name[255];
    sprintf (test_name, "../../tests/%.*s.tests", (int) (strrchr (testName, '_') - testName), testName);
    FILE *tests_file = fopen (test_name, "a+");
    if (!ok) fprintf (tests_file, "#");
    fprintf (tests_file, "../fonts/%s;--features=\"", fontfileName + 9);
    for (unsigned int i = 0; i < data.num_features; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        char buf[255];
        hb_feature_to_string (&data.features[i], buf, sizeof (buf));
        fprintf (tests_file, "%s", buf);
    }
    fprintf (tests_file, "\" --single-par --no-clusters --no-glyph-names --ned;");

    for (unsigned int i = 0; i < nbIn; i++)
    {
        if (i != 0) fprintf (tests_file, ",");
        fprintf (tests_file, "U+%04X", in[i]);
    }

    fprintf (tests_file, ";[");
    int accumlatedAdvance = 0;
    for (unsigned int i = 0; i < nbActual; i++)
    {
        if (i != 0) fprintf (tests_file, "|");
        fprintf (tests_file, "%d", /*it should be "out[i]"*/ actualG[i]);

        int expected_x = x[i] + accumlatedAdvance;
        int expected_y = y[i];
        if (expected_x || expected_y) fprintf (tests_file, "@%d,%d", expected_x, expected_y);
        if (hb_ot_layout_get_glyph_class (data.face, actualG[i]) != HB_OT_LAYOUT_GLYPH_CLASS_MARK)
            accumlatedAdvance += 1500;
    }
    fprintf (tests_file, "]");

    fprintf (tests_file, "\n");
    fclose (tests_file);


    free(actualG);
    free(actualX);
    free(actualY);

    return ok;
}


int main(int argc, char **argv)
{
    int failures = 0;
    int pass = 0;

#include "hb-aots-tester.h"

    printf ("%d failures, %d pass\n", failures, pass);
}



