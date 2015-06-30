/* InternalTask -- */

#include "InternalTask.h"
#include <assert.h>

namespace parallel {

InternalTask::InternalTask(TaskCompletion *comp)
    : completion(comp), _affinity(0xffffffff) {
  assert(uintptr_t(completion) < 0x0000ffffffffffff);
}
}
