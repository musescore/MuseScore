/*
 * Copyright © 2018  Ebrahim Byagowi
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
 */

#include <pthread.h>

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include "hb-test.h"

static const char *font_path = "fonts/Inconsolata-Regular.abc.ttf";
static const char *text = "abc";

static int num_threads = 30;
static int num_iters = 200;

static hb_font_t *font;
static hb_buffer_t *ref_buffer;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void
fill_the_buffer (hb_buffer_t *buffer)
{
  hb_buffer_add_utf8 (buffer, text, -1, 0, -1);
  hb_buffer_guess_segment_properties (buffer);
  hb_shape (font, buffer, NULL, 0);
}

static void
validity_check (hb_buffer_t *buffer) {
  if (hb_buffer_diff (ref_buffer, buffer, (hb_codepoint_t) -1, 0))
  {
    fprintf (stderr, "One of the buffers was different from the reference.\n");
    char out[255];

    hb_buffer_serialize_glyphs (buffer, 0, hb_buffer_get_length (ref_buffer),
				out, sizeof (out), NULL,
				font, HB_BUFFER_SERIALIZE_FORMAT_TEXT,
				HB_BUFFER_SERIALIZE_FLAG_NO_GLYPH_NAMES);
    fprintf (stderr, "Actual:   %s\n", out);

    hb_buffer_serialize_glyphs (ref_buffer, 0, hb_buffer_get_length (ref_buffer),
				out, sizeof (out), NULL,
				font, HB_BUFFER_SERIALIZE_FORMAT_TEXT,
				HB_BUFFER_SERIALIZE_FLAG_NO_GLYPH_NAMES);
    fprintf (stderr, "Expected: %s\n", out);

    exit (1);
  }
}

static void *
thread_func (void *data)
{
  hb_buffer_t *buffer = (hb_buffer_t *) data;

  pthread_mutex_lock (&mutex);
  pthread_mutex_unlock (&mutex);

  int i;
  for (i = 0; i < num_iters; i++)
  {
    hb_buffer_clear_contents (buffer);
    fill_the_buffer (buffer);
    validity_check (buffer);
  }

  return 0;
}

static void
test_body (void)
{
  int i;
  pthread_t *threads = calloc (num_threads, sizeof (pthread_t));
  hb_buffer_t **buffers = calloc (num_threads, sizeof (hb_buffer_t *));

  pthread_mutex_lock (&mutex);

  for (i = 0; i < num_threads; i++)
  {
    hb_buffer_t *buffer = hb_buffer_create ();
    buffers[i] = buffer;
    pthread_create (&threads[i], NULL, thread_func, buffer);
  }

  /* Let them loose! */
  pthread_mutex_unlock (&mutex);

  for (i = 0; i < num_threads; i++)
  {
    pthread_join (threads[i], NULL);
    hb_buffer_destroy (buffers[i]);
  }

  free (buffers);
  free (threads);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  char *path = argc > 1 && *argv[1] ? argv[1] : (char *) font_path;
  if (argc > 2)
    num_threads = atoi (argv[2]);
  if (argc > 3)
    num_iters = atoi (argv[3]);
  if (argc > 4)
    text = argv[4];

  /* Dummy call to alleviate _guess_segment_properties thread safety-ness
   * https://github.com/harfbuzz/harfbuzz/issues/1191 */
  hb_language_get_default ();

  hb_face_t *face = hb_test_open_font_file (path);
  font = hb_font_create (face);

  /* Fill the reference */
  ref_buffer = hb_buffer_create ();
  fill_the_buffer (ref_buffer);

  /* Unnecessary, since version 2 it is ot-font by default */
  hb_ot_font_set_funcs (font);
  test_body ();

  /* Test hb-ft in multithread */
  hb_ft_font_set_funcs (font);
  test_body ();

  hb_buffer_destroy (ref_buffer);

  hb_font_destroy (font);
  hb_face_destroy (face);

  return 0;
}
