#pragma once

#include "UNIV.h"
#include "EXTZONES.h"

#include <cinttypes>

namespace extemp {
namespace EXTLLVM {
  EXPORT void llvm_destroy_zone_after_delay(llvm_zone_t *Zone, uint64_t Delay);
}
}
