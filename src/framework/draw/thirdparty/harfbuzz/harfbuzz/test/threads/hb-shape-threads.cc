#include <cassert>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hb.h"
#include "hb-ot.h"
#ifdef HAVE_FREETYPE
#include "hb-ft.h"
#endif

#define SUBSET_FONT_BASE_PATH "test/subset/data/fonts/"

struct test_input_t
{
  const char *font_path;
  const char *text_path;
  bool is_variable;
} default_tests[] =
{

  {"perf/fonts/NotoNastaliqUrdu-Regular.ttf",
   "perf/texts/fa-thelittleprince.txt",
   false},

  {"perf/fonts/Amiri-Regular.ttf",
   "perf/texts/fa-thelittleprince.txt",
   false},

  {"perf/fonts/Roboto-Regular.ttf",
   "perf/texts/en-thelittleprince.txt",
   false},

  {"perf/fonts/Roboto-Regular.ttf",
   "perf/texts/en-words.txt",
   false},

  {SUBSET_FONT_BASE_PATH "SourceSerifVariable-Roman.ttf",
   "perf/texts/en-thelittleprince.txt",
   true},
};


static test_input_t *tests = default_tests;
static unsigned num_tests = sizeof (default_tests) / sizeof (default_tests[0]);

enum backend_t { HARFBUZZ, FREETYPE };

// https://en.cppreference.com/w/cpp/thread/condition_variable/wait
static std::condition_variable cv;
static std::mutex cv_m;
static bool ready = false;

static unsigned num_repetitions = 1;
static unsigned num_threads = 3;

static void shape (const test_input_t &input,
		   hb_font_t *font)
{
  // Wait till all threads are ready.
  {
    std::unique_lock<std::mutex> lk (cv_m);
    cv.wait(lk, [] {return ready;});
  }

  const char *lang_str = strrchr (input.text_path, '/');
  lang_str = lang_str ? lang_str + 1 : input.text_path;
  hb_language_t language = hb_language_from_string (lang_str, -1);

  hb_blob_t *text_blob = hb_blob_create_from_file_or_fail (input.text_path);
  assert (text_blob);
  unsigned orig_text_length;
  const char *orig_text = hb_blob_get_data (text_blob, &orig_text_length);

  hb_buffer_t *buf = hb_buffer_create ();
  hb_buffer_set_flags (buf, HB_BUFFER_FLAG_VERIFY);
  for (unsigned i = 0; i < num_repetitions; i++)
  {
    unsigned text_length = orig_text_length;
    const char *text = orig_text;

    const char *end;
    while ((end = (const char *) memchr (text, '\n', text_length)))
    {
      hb_buffer_clear_contents (buf);
      hb_buffer_add_utf8 (buf, text, text_length, 0, end - text);
      hb_buffer_guess_segment_properties (buf);
      hb_buffer_set_language (buf, language);
      hb_shape (font, buf, nullptr, 0);

      unsigned skip = end - text + 1;
      text_length -= skip;
      text += skip;
    }
  }
  hb_buffer_destroy (buf);

  hb_blob_destroy (text_blob);
}

static void test_backend (backend_t backend,
			  const char *backend_name,
			  bool variable,
			  const test_input_t &test_input)
{
  char name[1024] = "shape";
  const char *p;
  strcat (name, "/");
  p = strrchr (test_input.font_path, '/');
  strcat (name, p ? p + 1 : test_input.font_path);
  strcat (name, "/");
  p = strrchr (test_input.text_path, '/');
  strcat (name, p ? p + 1 : test_input.text_path);
  strcat (name, variable ? "/var" : "");
  strcat (name, "/");
  strcat (name, backend_name);

  printf ("Testing %s\n", name);

  hb_font_t *font;
  {
    hb_blob_t *blob = hb_blob_create_from_file_or_fail (test_input.font_path);
    assert (blob);
    hb_face_t *face = hb_face_create (blob, 0);
    hb_blob_destroy (blob);
    font = hb_font_create (face);
    hb_face_destroy (face);
  }

  if (variable)
  {
    hb_variation_t wght = {HB_TAG ('w','g','h','t'), 500};
    hb_font_set_variations (font, &wght, 1);
  }

  switch (backend)
  {
    case HARFBUZZ:
      hb_ot_font_set_funcs (font);
      break;

    case FREETYPE:
#ifdef HAVE_FREETYPE
      hb_ft_font_set_funcs (font);
#endif
      break;
  }

  std::vector<std::thread> threads;
  for (unsigned i = 0; i < num_threads; i++)
    threads.push_back (std::thread (shape, test_input, font));

  {
    std::unique_lock<std::mutex> lk (cv_m);
    ready = true;
  }
  cv.notify_all();

  for (unsigned i = 0; i < num_threads; i++)
    threads[i].join ();

  hb_font_destroy (font);
}

int main(int argc, char** argv)
{
  if (argc > 1)
    num_threads = atoi (argv[1]);
  if (argc > 2)
    num_repetitions = atoi (argv[2]);

  /* Dummy call to alleviate _guess_segment_properties thread safety-ness
   * https://github.com/harfbuzz/harfbuzz/issues/1191 */
  hb_language_get_default ();

  if (argc > 4)
  {
    num_tests = (argc - 3) / 2;
    tests = (test_input_t *) calloc (num_tests, sizeof (test_input_t));
    for (unsigned i = 0; i < num_tests; i++)
    {
      tests[i].is_variable = true;
      tests[i].font_path = argv[3 + i * 2];
      tests[i].text_path = argv[4 + i * 2];
    }
  }

  printf ("Num threads %u; num repetitions %u\n", num_threads, num_repetitions);
  for (unsigned i = 0; i < num_tests; i++)
  {
    auto& test_input = tests[i];
    for (int variable = 0; variable < int (test_input.is_variable) + 1; variable++)
    {
      bool is_var = (bool) variable;

      test_backend (HARFBUZZ, "hb", is_var, test_input);
#ifdef HAVE_FREETYPE
      test_backend (FREETYPE, "ft", is_var, test_input);
#endif
    }
  }

  if (tests != default_tests)
    free (tests);
}
