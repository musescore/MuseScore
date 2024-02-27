#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hb-subset.h"

enum operation_t
{
  subset_codepoints,
  subset_glyphs
};

#define SUBSET_FONT_BASE_PATH "test/subset/data/fonts/"

struct test_input_t
{
  const char *font_path;
  const unsigned max_subset_size;
} default_tests[] =
{
  {SUBSET_FONT_BASE_PATH "Roboto-Regular.ttf", 4000},
  {SUBSET_FONT_BASE_PATH "Amiri-Regular.ttf", 4000},
  {SUBSET_FONT_BASE_PATH "NotoNastaliqUrdu-Regular.ttf", 1000},
  {SUBSET_FONT_BASE_PATH "NotoSansDevanagari-Regular.ttf", 1000},
  {SUBSET_FONT_BASE_PATH "Mplus1p-Regular.ttf", 10000},
  {SUBSET_FONT_BASE_PATH "SourceHanSans-Regular_subset.otf", 10000},
  {SUBSET_FONT_BASE_PATH "SourceSansPro-Regular.otf", 2000},
};


static test_input_t *tests = default_tests;
static unsigned num_tests = sizeof (default_tests) / sizeof (default_tests[0]);


// https://en.cppreference.com/w/cpp/thread/condition_variable/wait
static std::condition_variable cv;
static std::mutex cv_m;
static bool ready = false;

static unsigned num_repetitions = 1;
static unsigned num_threads = 3;

static void AddCodepoints(const hb_set_t* codepoints_in_font,
                          unsigned subset_size,
                          hb_subset_input_t* input)
{
  auto *unicodes = hb_subset_input_unicode_set (input);
  hb_codepoint_t cp = HB_SET_VALUE_INVALID;
  for (unsigned i = 0; i < subset_size; i++) {
    if (!hb_set_next (codepoints_in_font, &cp)) return;
    hb_set_add (unicodes, cp);
  }
}

static void AddGlyphs(unsigned num_glyphs_in_font,
               unsigned subset_size,
               hb_subset_input_t* input)
{
  auto *glyphs = hb_subset_input_glyph_set (input);
  for (unsigned i = 0; i < subset_size && i < num_glyphs_in_font; i++) {
    hb_set_add (glyphs, i);
  }
}

static void subset (operation_t operation,
                    const test_input_t &test_input,
                    hb_face_t *face)
{
  // Wait till all threads are ready.
  {
    std::unique_lock<std::mutex> lk (cv_m);
    cv.wait(lk, [] {return ready;});
  }

  unsigned subset_size = test_input.max_subset_size;

  hb_subset_input_t* input = hb_subset_input_create_or_fail ();
  assert (input);

  switch (operation)
  {
    case subset_codepoints:
    {
      hb_set_t* all_codepoints = hb_set_create ();
      hb_face_collect_unicodes (face, all_codepoints);
      AddCodepoints(all_codepoints, subset_size, input);
      hb_set_destroy (all_codepoints);
    }
    break;

    case subset_glyphs:
    {
      unsigned num_glyphs = hb_face_get_glyph_count (face);
      AddGlyphs(num_glyphs, subset_size, input);
    }
    break;
  }

  for (unsigned i = 0; i < num_repetitions; i++)
  {
    hb_face_t* subset = hb_subset_or_fail (face, input);
    assert (subset);
    hb_face_destroy (subset);
  }

  hb_subset_input_destroy (input);
}

static void test_operation (operation_t operation,
                            const char *operation_name,
                            const test_input_t &test_input)
{
  char name[1024] = "subset";
  const char *p;
  strcat (name, "/");
  p = strrchr (test_input.font_path, '/');
  strcat (name, p ? p + 1 : test_input.font_path);
  strcat (name, "/");
  strcat (name, operation_name);

  printf ("Testing %s\n", name);

  hb_face_t *face;
  {
    hb_blob_t *blob = hb_blob_create_from_file_or_fail (test_input.font_path);
    assert (blob);
    face = hb_face_create (blob, 0);
    hb_blob_destroy (blob);
  }

  std::vector<std::thread> threads;
  for (unsigned i = 0; i < num_threads; i++)
    threads.push_back (std::thread (subset, operation, test_input, face));

  {
    std::unique_lock<std::mutex> lk (cv_m);
    ready = true;
  }
  cv.notify_all();

  for (unsigned i = 0; i < num_threads; i++)
    threads[i].join ();

  hb_face_destroy (face);
}

int main(int argc, char** argv)
{
  if (argc > 1)
    num_threads = atoi (argv[1]);
  if (argc > 2)
    num_repetitions = atoi (argv[2]);

  if (argc > 4)
  {
    num_tests = argc - 3;
    tests = (test_input_t *) calloc (num_tests, sizeof (test_input_t));
    for (unsigned i = 0; i < num_tests; i++)
    {
      tests[i].font_path = argv[3 + i];
    }
  }

  printf ("Num threads %u; num repetitions %u\n", num_threads, num_repetitions);
  for (unsigned i = 0; i < num_tests; i++)
  {
    auto& test_input = tests[i];
    test_operation (subset_codepoints, "codepoints", test_input);
    test_operation (subset_glyphs, "glyphs", test_input);
  }

  if (tests != default_tests)
    free (tests);
}
