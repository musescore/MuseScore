#include "hb-fuzzer.hh"

#include <cassert>
#include <cstdio>

int main (int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    hb_blob_t *blob = hb_blob_create_from_file_or_fail (argv[i]);
    assert (blob);

    unsigned len = 0;
    const char *font_data = hb_blob_get_data (blob, &len);
    printf ("%s (%u bytes)\n", argv[i], len);

    LLVMFuzzerTestOneInput ((const uint8_t *) font_data, len);

    hb_blob_destroy (blob);
  }
  return 0;
}
