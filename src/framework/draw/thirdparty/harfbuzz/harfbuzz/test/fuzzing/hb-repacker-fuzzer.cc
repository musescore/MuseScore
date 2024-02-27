#include "hb-fuzzer.hh"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hb-subset-repacker.h"

typedef struct
{
  uint16_t parent;
  uint16_t child;
  uint16_t position;
  uint8_t width;
} link_t;

/* The fuzzer seed contains a serialized representation of a object graph which forms
 * the input graph to the repacker call. The binary format is:
 *
 * table tag: 4 bytes
 * number of objects: 2 bytes
 * objects[number of objects]:
 *   blob size: 2 bytes
 *   blob: blob size bytes
 * num of real links: 2 bytes
 * links[number of real links]: link_t struct
 *
 * TODO(garretrieger): add optional virtual links
 */

template <typename T>
bool read(const uint8_t** data, size_t* size, T* out)
{
  if (*size < sizeof (T)) return false;

  memcpy(out, *data, sizeof (T));

  *data += sizeof (T);
  *size -= sizeof (T);

  return true;
}

void cleanup (hb_object_t* objects, uint16_t num_objects)
{
  for (uint32_t i = 0; i < num_objects; i++)
  {
    free (objects[i].head);
    free (objects[i].real_links);
  }
}

void add_links_to_objects (hb_object_t* objects, uint16_t num_objects,
                           link_t* links, uint16_t num_links)
{
  unsigned* link_count = (unsigned*) calloc (num_objects, sizeof (unsigned));

  for (uint32_t i = 0; i < num_links; i++)
  {
    uint16_t parent_idx = links[i].parent;
    link_count[parent_idx]++;
  }

  for (uint32_t i = 0; i < num_objects; i++)
  {
    objects[i].num_real_links = link_count[i];
    objects[i].real_links = (hb_link_t*) calloc (link_count[i], sizeof (hb_link_t));
    objects[i].num_virtual_links = 0;
    objects[i].virtual_links = nullptr;
  }

  for (uint32_t i = 0; i < num_links; i++)
  {
    uint16_t parent_idx = links[i].parent;
    uint16_t child_idx = links[i].child + 1; // All indices are shifted by 1 by the null object.
    hb_link_t* link = &(objects[parent_idx].real_links[link_count[parent_idx] - 1]);

    link->width = links[i].width;
    link->position = links[i].position;
    link->objidx = child_idx;
    link_count[parent_idx]--;
  }

  free (link_count);
}

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, size_t size)
{
  // TODO(garretrieger): move graph validity checks into repacker graph creation.
  alloc_state = _fuzzing_alloc_state (data, size);

  uint16_t num_objects = 0;
  hb_object_t* objects = nullptr;

  uint16_t num_real_links = 0;
  link_t* links = nullptr;

  hb_tag_t table_tag;
  if (!read<hb_tag_t> (&data, &size, &table_tag)) goto end;
  if (!read<uint16_t> (&data, &size, &num_objects)) goto end;

  objects = (hb_object_t*) calloc (num_objects, sizeof (hb_object_t));
  for (uint32_t i = 0; i < num_objects; i++)
  {
    uint16_t blob_size;
    if (!read<uint16_t> (&data, &size, &blob_size)) goto end;
    if (size < blob_size) goto end;

    char* copy = (char*) calloc (1, blob_size);
    memcpy (copy, data, blob_size);
    objects[i].head = (char*) copy;
    objects[i].tail = (char*) (copy + blob_size);

    size -= blob_size;
    data += blob_size;
  }

  if (!read<uint16_t> (&data, &size, &num_real_links)) goto end;
  links = (link_t*) calloc (num_real_links, sizeof (link_t));
  for (uint32_t i = 0; i < num_real_links; i++)
  {
    if (!read<link_t> (&data, &size, &links[i])) goto end;

    if (links[i].parent >= num_objects)
      goto end;
  }

  add_links_to_objects (objects, num_objects,
                        links, num_real_links);

  hb_blob_destroy (hb_subset_repack_or_fail (table_tag,
                                             objects,
                                             num_objects));

end:
  if (objects)
  {
    cleanup (objects, num_objects);
    free (objects);
  }
  free (links);

  return 0;
}
