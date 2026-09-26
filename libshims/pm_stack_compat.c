#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef void (*inc_strong_fn)(const void *, const void *);

struct inc_strong_call {
    inc_strong_fn function;
    const void *object;
    const void *id;
};

static void *inc_strong_on_worker(void *opaque)
{
    struct inc_strong_call *call = opaque;
    call->function(call->object, call->id);
    return NULL;
}

/* pm-service creates its Binder service on main's stack. Android 14 RefBase
 * rejects a first strong reference from the same stack page. Keep the real
 * RefBase operation, but perform that one call from a separate thread. */
__attribute__((visibility("default")))
void pm_inc_strong(const void *object, const void *id)
    __asm__("_ZNK7android7RefBase9incStrongEPKv");

void pm_inc_strong(const void *object, const void *id)
{
    static inc_strong_fn real_inc_strong;
    inc_strong_fn function;
    pthread_attr_t attributes;
    void *stack_base = NULL;
    size_t stack_size = 0;
    bool on_this_stack = false;

    function = __atomic_load_n(&real_inc_strong, __ATOMIC_ACQUIRE);
    if (function == NULL) {
        function = (inc_strong_fn)dlsym(RTLD_NEXT,
                "_ZNK7android7RefBase9incStrongEPKv");
        if (function == NULL)
            abort();
        __atomic_store_n(&real_inc_strong, function, __ATOMIC_RELEASE);
    }

    if (pthread_getattr_np(pthread_self(), &attributes) == 0) {
        if (pthread_attr_getstack(&attributes, &stack_base, &stack_size) == 0) {
            uintptr_t address = (uintptr_t)object;
            uintptr_t base = (uintptr_t)stack_base;
            on_this_stack = address >= base && address - base < stack_size;
        }
        pthread_attr_destroy(&attributes);
    }

    if (on_this_stack) {
        struct inc_strong_call call = { function, object, id };
        pthread_t worker;
        if (pthread_create(&worker, NULL, inc_strong_on_worker, &call) != 0)
            abort();
        if (pthread_join(worker, NULL) != 0)
            abort();
    } else {
        function(object, id);
    }
}
