#pragma once

#include "../msgpack/msgpack.h" // kors

namespace app::msgpack {
using Packer = kors::msgpack::Packer;
using UnPacker = kors::msgpack::UnPacker;
using Cursor = kors::msgpack::Cursor;
}
