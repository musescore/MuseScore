#include "hb-fuzzer.hh"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hb.h"

// Only allow ~5,000 set values between the two input sets.
// Arbitrarily long input sets do not trigger any meaningful
// differences in behaviour so there's no benefit from allowing
// the fuzzer to create super large sets.
#define MAX_INPUT_SIZE 20000

enum set_operation_t : uint8_t
{
  INTERSECT = 0,
  UNION = 1,
  SUBTRACT = 2,
  SYMMETRIC_DIFFERENCE = 3
};

struct instructions_t
{
  set_operation_t operation;
  uint32_t first_set_size;
};

static hb_set_t *create_set (const uint32_t *value_array, int count)
{
  hb_set_t *set = hb_set_create ();
  for (int i = 0; i < count; i++)
    hb_set_add (set, value_array[i]);
  return set;
}


extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, size_t size)
{
  alloc_state = _fuzzing_alloc_state (data, size);

  if (size < sizeof (instructions_t))
    return 0;

  if (size > MAX_INPUT_SIZE)
    return 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  const instructions_t &instructions = reinterpret_cast<const instructions_t &> (data);
#pragma GCC diagnostic pop
  data += sizeof (instructions_t);
  size -= sizeof (instructions_t);

  const uint32_t *values = reinterpret_cast<const uint32_t *> (data);
  size = size / sizeof (uint32_t);

  if (size < instructions.first_set_size)
    return 0;

  hb_set_t *set_a = create_set (values, instructions.first_set_size);

  values += instructions.first_set_size;
  size -= instructions.first_set_size;
  hb_set_t *set_b = create_set (values, size);

  switch (instructions.operation)
  {
  case INTERSECT:
    hb_set_intersect (set_a, set_b);
    break;
  case UNION:
    hb_set_union (set_a, set_b);
    break;
  case SUBTRACT:
    hb_set_subtract (set_a, set_b);
    break;
  case SYMMETRIC_DIFFERENCE:
    hb_set_symmetric_difference (set_a, set_b);
    break;
  default:
    break;
  }

  hb_set_destroy (set_a);
  hb_set_destroy (set_b);

  return 0;
}
