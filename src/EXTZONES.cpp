#include "EXTZONES.h"
#include "EXTMutex.h"
#include "BranchPrediction.h"

#include <cstring>
#include <iostream>

THREAD_LOCAL llvm_zone_stack* tls_llvm_zone_stack = 0;
THREAD_LOCAL uint64_t tls_llvm_zone_stacksize = 0;

namespace extemp {
namespace EXTZONES {

llvm_zone_t* llvm_zone_create(uint64_t size)
{
    auto zone(static_cast<llvm_zone_t*>(malloc(sizeof(llvm_zone_t))));
    if (unlikely(!zone)) {
        abort(); // in case a leak can be analyzed post-mortem
    }
#ifdef _WIN32
	if (size == 0) {
		zone->memory = NULL;
	}
	else {
		// this crashes extempore but I have no idea why????
		// zone->memory = _aligned_malloc((size_t)size, (size_t)LLVM_ZONE_ALIGN);
		zone->memory = malloc(size_t(size));
	}
#else
    posix_memalign(&zone->memory, LLVM_ZONE_ALIGN, size_t(size));
#endif
    zone->mark = 0;
    zone->offset = 0;
    if (unlikely(!zone->memory)) {
      size = 0;
    }
    zone->size = size;
    zone->cleanup_hooks = nullptr;
    zone->memories = nullptr;
    return zone;
}

EXPORT void llvm_zone_destroy(llvm_zone_t* Zone)
{
#if DEBUG_ZONE_ALLOC
    printf("DestroyZone: %p:%p:%lld:%lld\n", Zone, Zone->memory, Zone->offset, Zone->size);
#endif
    if (Zone->memories) {
        llvm_zone_destroy(Zone->memories);
    }
    free(Zone->memory);
    free(Zone);
}

llvm_zone_t* llvm_zone_reset(llvm_zone_t* Zone)
{
    Zone->offset = 0;
    return Zone;
}

EXPORT void* llvm_zone_malloc(llvm_zone_t* zone, uint64_t size)
{
    std::cout << "llvm_zone_malloc" << std::endl;
    static extemp::EXTMutex alloc_mutex("alloc mutex");
    // TODO: is this thread-safe?
    if (!alloc_mutex.initialised()) {
        alloc_mutex.init();
    }

    extemp::EXTMutex::ScopedLock lock(alloc_mutex);

#if DEBUG_ZONE_ALLOC
    printf("MallocZone: %p:%p:%lld:%lld:%lld\n", zone, zone->memory, zone->offset, zone->size, size);
#endif

    size += LLVM_ZONE_ALIGN; // for storing size information
    if (unlikely(zone->offset + size >= zone->size)) {
        int old_zone_size = zone->size;
        bool iszero(!zone->size);
        if (size > zone->size) {
            zone->size = size;
        }
        zone->size *= 2; // keep doubling zone size for each new allocation // TODO: 1.5???
        if (zone->size < 1024) {
            zone->size = 1024; // allocate a min size of 1024 bytes
        }
        llvm_zone_t* newzone = llvm_zone_create(zone->size);
        void* tmp = newzone->memory;
        if (iszero) { // if initial zone is 0 - then replace don't extend
          zone->memory = tmp;
          free(newzone);
        } else {
            // printf("adding new memory %p:%lld to existing %p:%lld\n",newzone,newzone->size,zone,zone->size);
            newzone->memories = zone->memories;
            newzone->memory = zone->memory;
            newzone->size = old_zone_size;
            zone->memory = tmp;
            zone->memories = newzone;
        }
        llvm_zone_reset(zone);
    }
    size = (size + LLVM_ZONE_ALIGNPAD) & ~LLVM_ZONE_ALIGNPAD;
    auto newptr = reinterpret_cast<void*>(reinterpret_cast<char*>(zone->memory) + zone->offset);
    memset(newptr, 0, size); // clear memory
    newptr = reinterpret_cast<char*>(newptr) + LLVM_ZONE_ALIGN; // skip past size
    *(reinterpret_cast<uint64_t*>(newptr) - 1) = size;
    zone->offset += size;
    return newptr;
}

llvm_zone_stack* llvm_threads_get_zone_stack()
{
    return tls_llvm_zone_stack;
}

void llvm_threads_set_zone_stack(llvm_zone_stack* Stack)
{
    tls_llvm_zone_stack = Stack;
}

void llvm_push_zone_stack(llvm_zone_t* Zone)
{
    auto stack(reinterpret_cast<llvm_zone_stack*>(malloc(sizeof(llvm_zone_stack))));
    stack->head = Zone;
    stack->tail = llvm_threads_get_zone_stack();
    llvm_threads_set_zone_stack(stack);
    return;
}

llvm_zone_t* llvm_peek_zone_stack()
{
    llvm_zone_t* z = 0;
    llvm_zone_stack* stack = llvm_threads_get_zone_stack();
    if (unlikely(!stack)) {  // for the moment create a "DEFAULT" zone if stack is NULL
#if DEBUG_ZONE_STACK
        printf("TRYING TO PEEK AT A NULL ZONE STACK\n");
#endif
        llvm_zone_t* z = llvm_zone_create(1024 * 1024 * 1); // default root zone is 1M
        llvm_push_zone_stack(z);
        stack = llvm_threads_get_zone_stack();
#if DEBUG_ZONE_STACK
        printf("Creating new 1M default zone %p:%lld on ZStack:%p\n",z,z->size,stack);
#endif
        return z;
    }
    z = stack->head;
#if DEBUG_ZONE_STACK
    printf("%p: peeking at zone %p:%lld\n",stack,z,z->size);
#endif
    return z;
}

EXPORT llvm_zone_t* llvm_pop_zone_stack()
{
    auto stack(llvm_threads_get_zone_stack());
    if (unlikely(!stack)) {
#if DEBUG_ZONE_STACK
        printf("TRYING TO POP A ZONE FROM AN EMPTY ZONE STACK\n");
#endif
        return nullptr;
    }
    llvm_zone_t* head = stack->head;
    llvm_zone_stack* tail = stack->tail;
#if DEBUG_ZONE_STACK
    llvm_threads_dec_zone_stacksize();
    if (!tail) {
        printf("%p: popping zone %p:%lld from stack with no tail\n",stack,head,head->size);
    } else {
        printf("%p: popping new zone %p:%lld back to old zone %p:%lld\n",stack,head,head->size,tail->head,tail->head->size);
    }
#endif
    free(stack);
    llvm_threads_set_zone_stack(tail);
    return head;
}

void llvm_threads_inc_zone_stacksize() {
    ++tls_llvm_zone_stacksize;
}

void llvm_threads_dec_zone_stacksize() {
    --tls_llvm_zone_stacksize;
}

uint64_t llvm_threads_get_zone_stacksize() {
    return tls_llvm_zone_stacksize;
}

EXPORT void llvm_zone_print(llvm_zone_t* zone)
{
    auto tmp(zone);
    auto total_size(zone->size);
    int64_t segments(1);
    while (tmp->memories) {
        tmp = tmp->memories;
        total_size += tmp->size;
        segments++;
    }
    printf("<MemZone(%p) size(%" PRId64 ") free(%" PRId64 ") segs(%" PRId64 ")>", zone, total_size, (zone->size - zone->offset), segments);
    return;
}

EXPORT uint64_t llvm_zone_ptr_size(void* ptr) // could be inline version in llvm (as well)
{
    return *(reinterpret_cast<uint64_t*>(ptr) - 1);
}

EXPORT bool llvm_zone_copy_ptr(void* ptr1, void* ptr2)
{
    uint64_t size1 = llvm_zone_ptr_size(ptr1);
    uint64_t size2 = llvm_zone_ptr_size(ptr2);

    if (unlikely(size1 != size2)) {
        // printf("Bad LLVM ptr copy - size mismatch setting %p:%lld -> %p:%lld\n", ptr1, size1, ptr2, size2);
        return 1;
    }
    if (unlikely(!size1)) {
        // printf("Bad LLVM ptr copy - size mismatch setting %p:%lld -> %p:%lld\n", ptr1, size1, ptr2, size2);
        return 1;
    }
    // printf("zone_copy_ptr: %p,%p,%lld,%lld\n", ptr2, ptr1, size1, size2);
    std::memcpy(ptr2, ptr1, size1);
    return 0;
}

EXPORT bool llvm_ptr_in_zone(llvm_zone_t* zone, void* ptr)
{
    while (unlikely(zone && (ptr < zone->memory || ptr >= reinterpret_cast<char*>(zone->memory) + zone->size))) {
      zone = zone->memories;
    }
    return zone;
}

EXPORT void* llvm_zone_malloc_from_current_zone(uint64_t size)
{
  return llvm_zone_malloc(llvm_peek_zone_stack(), size);
}

EXPORT bool llvm_ptr_in_current_zone(void* ptr)
{
    return llvm_ptr_in_zone(llvm_peek_zone_stack(), ptr);
}

EXPORT llvm_zone_t* llvm_peek_zone_stack_extern()
{
    return llvm_peek_zone_stack();
}

EXPORT void llvm_push_zone_stack_extern(llvm_zone_t* Zone)
{
    llvm_push_zone_stack(Zone);
}

EXPORT llvm_zone_t* llvm_zone_create_extern(uint64_t Size)
{
    return llvm_zone_create(Size);
}

static THREAD_LOCAL llvm_zone_t* tls_llvm_callback_zone = 0;

static inline llvm_zone_t* llvm_threads_get_callback_zone()
{
    if (unlikely(!tls_llvm_callback_zone)) {
        tls_llvm_callback_zone = llvm_zone_create(1024 * 1024); // default callback zone 1M
    }
    return tls_llvm_callback_zone;
}

EXPORT llvm_zone_t* llvm_zone_callback_setup()
{
    auto zone(llvm_threads_get_callback_zone());
    llvm_push_zone_stack(zone);
    return llvm_zone_reset(zone);
}

void llvm_zone_mark(llvm_zone_t* zone) {
    std::cout << "llvm_zone_mark" << std::endl;
    zone->mark = zone->offset;
}

// TODO: think about llvm signed/unsigned stuff
int64_t llvm_zone_mark_size(llvm_zone_t* zone) {
    std::cout << "llvm_zone_mark_size" << std::endl;
    return zone->offset - zone->mark;
}

// TODO: use a native pointer type for portability
// I just don't remember what it is OTOH
// this is definitely undefined behaviour in C++ so I can
// see why you'd want to do it in LLVM IR
void llvm_zone_ptr_set_size(llvm_zone_t* zone, uint64_t size) {
    std::cout << "llvm_zone_ptr_set_size" << std::endl;
    uint64_t* ptr = reinterpret_cast<uint64_t *>(zone);
    uint64_t* size_ptr = ptr-1;
    *size_ptr = size;
}

} // namespace EXTLLVM
} // namespace extemp
