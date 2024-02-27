/*
 * Copyright © 2011  Google, Inc.
 * Copyright © 2022  Behdad Esfahbod
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

/* This file tests that all headers can be included from C++ files,
 * as well as test the C++ API. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <hb.h>
#include <hb-subset.h>
#include <hb-ot.h>
#include <hb-aat.h>

#ifdef HAVE_GLIB
#include <hb-glib.h>
#endif

#ifdef HAVE_ICU
#include <hb-icu.h>
#endif

#ifdef HAVE_FREETYPE
#include <hb-ft.h>
#endif

#ifdef HAVE_UNISCRIBE
#include <hb-uniscribe.h>
#endif

#ifdef HAVE_CORETEXT
#include <hb-coretext.h>
#endif


/* Test C++ API. */

#include "hb-cplusplus.hh"

#include <cassert>
#include <functional>
#include <utility>

int
main ()
{
  hb_buffer_t *b = hb_buffer_create ();
  hb::shared_ptr<hb_buffer_t> pb {b};

  /* Test copy-construction. */
  assert (bool (pb));
  hb::shared_ptr<hb_buffer_t> pb2 {pb};
  assert (bool (pb2));
  assert (bool (pb));

  /* Test move-construction. */
  assert (bool (pb2));
  hb::shared_ptr<hb_buffer_t> pb4 {std::move (pb2)};
  assert (!bool (pb2));
  assert (bool (pb4));

  /* Test copy-assignment. */
  hb::shared_ptr<hb_buffer_t> pb3;
  assert (!bool (pb3));
  pb3 = pb;
  assert (bool (pb3));
  assert (bool (pb));

  /* Test move-assignment. */
  assert (bool (pb));
  pb2 = std::move (pb);
  assert (!bool (pb));

  pb.reference ();
  pb.destroy ();

  pb3.reference ();
  pb3.destroy ();

  pb3.swap (pb4);

  hb_user_data_key_t key;
  pb.set_user_data (&key, b, nullptr, true);
  (void) pb.get_user_data (&key);

  hb::unique_ptr<hb_buffer_t> pb5 {pb3.reference ()};


  /* Test that shared_ptr / unique_ptr are std::hash'able, and that they
   * return the same hash (which is the underlying pointer's hash. */
  std::hash<hb_buffer_t *> hash {};
  std::hash<hb::shared_ptr<hb_buffer_t>> hash2 {};
  std::hash<hb::unique_ptr<hb_buffer_t>> hash3 {};

  assert (hash (b) == hash2 (pb4));
  assert (hash2 (pb4) == hash2 (pb2));
  assert (hash (b) == hash3 (pb5));

  return pb == pb.get_empty () || pb == pb2;
}
