#include "EXTLLVM.h"

namespace extemp {
  namespace EXTLLVM {
    EXPORT void llvm_destroy_zone_after_delay(llvm_zone_t* Zone, uint64_t Delay) {
      extemp::EXTZONES::llvm_zone_destroy(Zone);
    }
  }
}
