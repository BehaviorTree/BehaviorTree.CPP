# minicoro

Minicoro is single-file library for using asymmetric coroutines in C.
The API is inspired by [Lua coroutines](https://www.lua.org/manual/5.4/manual.html#6.2) but with C use in mind.

The project is being developed mainly to be a coroutine backend
for the [Nelua](https://github.com/edubart/nelua-lang) programming language.

The library assembly implementation is inspired by [Lua Coco](https://coco.luajit.org/index.html) by Mike Pall.

# Features

- Stackful asymmetric coroutines.
- Supports nesting coroutines (resuming a coroutine from another coroutine).
- Supports custom allocators.
- Storage system to allow passing values between yield and resume.
- Customizable stack size.
- Coroutine API design inspired by Lua with C use in mind.
- Yield across any C function.
- Made to work in multithread applications.
- Cross platform.
- Minimal, self contained and no external dependencies.
- Readable sources and documented.
- Implemented via assembly, ucontext or fibers.
- Lightweight and very efficient.
- Works in most C89 compilers.
- Error prone API, returning proper error codes on misuse.
- Support running with Valgrind, ASan (AddressSanitizer) and TSan (ThreadSanitizer).

# Supported Platforms

Most platforms are supported through different methods:

| Platform     | Assembly Method  | Fallback Method   |
|--------------|------------------|-------------------|
| Android      | ARM/ARM64        | N/A               |
| iOS          | ARM/ARM64        | N/A               |
| Windows      | x86_64           | Windows fibers    |
| Linux        | x86_64/i686      | ucontext          |
| Mac OS X     | x86_64/ARM/ARM64 | ucontext          |
| WebAssembly  | N/A              | Emscripten fibers / Binaryen asyncify |
| Raspberry Pi | ARM              | ucontext          |
| RISC-V       | rv64/rv32        | ucontext          |

The assembly method is used by default if supported by the compiler and CPU,
otherwise ucontext or fiber method is used as a fallback.

The assembly method is very efficient, it just take a few cycles
to create, resume, yield or destroy a coroutine.

# Caveats

- Don't use coroutines with C++ exceptions, this is not supported.
- When using C++ RAII (i.e. destructors) you must resume the coroutine until it dies to properly execute all destructors.
- To use in multithread applications, you must compile with C compiler that supports `thread_local` qualifier.
- Some unsupported sanitizers for C may trigger false warnings when using coroutines.
- The `mco_coro` object is not thread safe, you should lock each coroutine into a thread.
- Stack space is fixed, it cannot grow. By default it has about 56KB of space, this can be changed on coroutine creation.
- Take care to not cause stack overflows (run out of stack space), otherwise your program may crash or not, the behavior is undefined.
- On WebAssembly you must compile with Emscripten flag `-s ASYNCIFY=1`.
- The WebAssembly Binaryen asyncify method can be used when explicitly enabled,
you may want to do this only to use minicoro with WebAssembly native interpreters
(no Web browser). This method is confirmed to work well with Emscripten toolchain,
however it fails on other WebAssembly toolchains like WASI SDK.

# Introduction

A coroutine represents an independent "green" thread of execution.
Unlike threads in multithread systems, however,
a coroutine only suspends its execution by explicitly calling a yield function.

You create a coroutine by calling `mco_create`.
Its sole argument is a `mco_desc` structure with a description for the coroutine.
The `mco_create` function only creates a new coroutine and returns a handle to it, it does not start the coroutine.

You execute a coroutine by calling `mco_resume`.
When calling a resume function the coroutine starts its execution by calling its body function.
After the coroutine starts running, it runs until it terminates or yields.

A coroutine yields by calling `mco_yield`.
When a coroutine yields, the corresponding resume returns immediately,
even if the yield happens inside nested function calls (that is, not in the main function).
The next time you resume the same coroutine, it continues its execution from the point where it yielded.

To associate a persistent value with the coroutine,
you can  optionally set `user_data` on its creation and later retrieve with `mco_get_user_data`.

To pass values between resume and yield,
you can optionally use `mco_push` and `mco_pop` APIs,
they are intended to pass temporary values using a LIFO (Last In, First Out) style buffer.
The storage system can also be used to send and receive initial values on coroutine creation or before it finishes.

# Usage

To use minicoro, do the following in one .c file:

```c
#define MINICORO_IMPL
#include "minicoro.h"
```

You can do `#include "minicoro.h"` in other parts of the program just like any other header.

## Minimal Example

The following simple example demonstrates on how to use the library:

```c
#define MINICORO_IMPL
#include "minicoro.h"
#include <stdio.h>

// Coroutine entry function.
void coro_entry(mco_coro* co) {
  printf("coroutine 1\n");
  mco_yield(co);
  printf("coroutine 2\n");
}

int main() {
  // First initialize a `desc` object through `mco_desc_init`.
  mco_desc desc = mco_desc_init(coro_entry, 0);
  // Configure `desc` fields when needed (e.g. customize user_data or allocation functions).
  desc.user_data = NULL;
  // Call `mco_create` with the output coroutine pointer and `desc` pointer.
  mco_coro* co;
  mco_result res = mco_create(&co, &desc);
  assert(res == MCO_SUCCESS);
  // The coroutine should be now in suspended state.
  assert(mco_status(co) == MCO_SUSPENDED);
  // Call `mco_resume` to start for the first time, switching to its context.
  res = mco_resume(co); // Should print "coroutine 1".
  assert(res == MCO_SUCCESS);
  // We get back from coroutine context in suspended state (because it's unfinished).
  assert(mco_status(co) == MCO_SUSPENDED);
  // Call `mco_resume` to resume for a second time.
  res = mco_resume(co); // Should print "coroutine 2".
  assert(res == MCO_SUCCESS);
  // The coroutine finished and should be now dead.
  assert(mco_status(co) == MCO_DEAD);
  // Call `mco_destroy` to destroy the coroutine.
  res = mco_destroy(co);
  assert(res == MCO_SUCCESS);
  return 0;
}
```

_NOTE_: In case you don't want to use the minicoro allocator system you should
allocate a coroutine object yourself using `mco_desc.coro_size` and call `mco_init`,
then later to destroy call `mco_uninit` and deallocate it.

## Yielding from anywhere

You can yield the current running coroutine from anywhere
without having to pass `mco_coro` pointers around,
to this just use `mco_yield(mco_running())`.

## Passing data between yield and resume

The library has the storage interface to assist passing data between yield and resume.
It's usage is straightforward,
use `mco_push` to send data before a `mco_resume` or `mco_yield`,
then later use `mco_pop` after a `mco_resume` or `mco_yield` to receive data.
Take care to not mismatch a push and pop, otherwise these functions will return
an error.

## Error handling

The library return error codes in most of its API in case of misuse or system error,
the user is encouraged to handle them properly.

## Library customization

The following can be defined to change the library behavior:

- `MCO_API`                   - Public API qualifier. Default is `extern`.
- `MCO_MIN_STACK_SIZE`        - Minimum stack size when creating a coroutine. Default is 32768.
- `MCO_DEFAULT_STORAGE_SIZE`  - Size of coroutine storage buffer. Default is 1024.
- `MCO_DEFAULT_STACK_SIZE`    - Default stack size when creating a coroutine. Default is 57344.
- `MCO_MALLOC`                - Default allocation function. Default is `malloc`.
- `MCO_FREE`                  - Default deallocation function. Default is `free`.
- `MCO_DEBUG`                 - Enable debug mode, logging any runtime error to stdout. Defined automatically unless `NDEBUG` or `MCO_NO_DEBUG` is defined.
- `MCO_NO_DEBUG`              - Disable debug mode.
- `MCO_NO_MULTITHREAD`        - Disable multithread usage. Multithread is supported when `thread_local` is supported.
- `MCO_NO_DEFAULT_ALLOCATORS` - Disable the default allocator using `MCO_MALLOC` and `MCO_FREE`.
- `MCO_ZERO_MEMORY`           - Zero memory of stack for new coroutines and when poping storage, intended for garbage collected environments.
- `MCO_USE_ASM`               - Force use of assembly context switch implementation.
- `MCO_USE_UCONTEXT`          - Force use of ucontext context switch implementation.
- `MCO_USE_FIBERS`            - Force use of fibers context switch implementation.
- `MCO_USE_ASYNCIFY`          - Force use of Binaryen asyncify context switch implementation.
- `MCO_USE_VALGRIND`          - Define if you want run with valgrind to fix accessing memory errors.

# Benchmarks

The coroutine library was benchmarked for x86_64 counting CPU cycles
for context switch (triggered in resume or yield) and initialization.

| CPU Arch | OS       | Method   | Context switch | Initialize   | Uninitialize |
|----------|----------|----------|----------------|--------------|--------------|
| x86_64   | Linux    | assembly | 9 cycles       | 31 cycles    | 14 cycles    |
| x86_64   | Linux    | ucontext | 352 cycles     | 383 cycles   | 14 cycles    |
| x86_64   | Windows  | fibers   | 69 cycles      | 10564 cycles | 11167 cycles |
| x86_64   | Windows  | assembly | 33 cycles      | 74 cycles    | 14 cycles    |

_NOTE_: Tested on Intel Core i7-8750H CPU @ 2.20GHz with pre allocated coroutines.

# Cheatsheet

Here is a list of all library functions for quick reference:

```c
/* Structure used to initialize a coroutine. */
typedef struct mco_desc {
  void (*func)(mco_coro* co); /* Entry point function for the coroutine. */
  void* user_data;            /* Coroutine user data, can be get with `mco_get_user_data`. */
  /* Custom allocation interface. */
  void* (*malloc_cb)(size_t size, void* allocator_data); /* Custom allocation function. */
  void  (*free_cb)(void* ptr, void* allocator_data);     /* Custom deallocation function. */
  void* allocator_data;       /* User data pointer passed to `malloc`/`free` allocation functions. */
  size_t storage_size;        /* Coroutine storage size, to be used with the storage APIs. */
  /* These must be initialized only through `mco_init_desc`. */
  size_t coro_size;           /* Coroutine structure size. */
  size_t stack_size;          /* Coroutine stack size. */
} mco_desc;

/* Coroutine functions. */
mco_desc mco_desc_init(void (*func)(mco_coro* co), size_t stack_size);  /* Initialize description of a coroutine. When stack size is 0 then MCO_DEFAULT_STACK_SIZE is used. */
mco_result mco_init(mco_coro* co, mco_desc* desc);                      /* Initialize the coroutine. */
mco_result mco_uninit(mco_coro* co);                                    /* Uninitialize the coroutine, may fail if it's not dead or suspended. */
mco_result mco_create(mco_coro** out_co, mco_desc* desc);               /* Allocates and initializes a new coroutine. */
mco_result mco_destroy(mco_coro* co);                                   /* Uninitialize and deallocate the coroutine, may fail if it's not dead or suspended. */
mco_result mco_resume(mco_coro* co);                                    /* Starts or continues the execution of the coroutine. */
mco_result mco_yield(mco_coro* co);                                     /* Suspends the execution of a coroutine. */
mco_state mco_status(mco_coro* co);                                     /* Returns the status of the coroutine. */
void* mco_get_user_data(mco_coro* co);                                  /* Get coroutine user data supplied on coroutine creation. */

/* Storage interface functions, used to pass values between yield and resume. */
mco_result mco_push(mco_coro* co, const void* src, size_t len); /* Push bytes to the coroutine storage. Use to send values between yield and resume. */
mco_result mco_pop(mco_coro* co, void* dest, size_t len);       /* Pop bytes from the coroutine storage. Use to get values between yield and resume. */
mco_result mco_peek(mco_coro* co, void* dest, size_t len);      /* Like `mco_pop` but it does not consumes the storage. */
size_t mco_get_bytes_stored(mco_coro* co);                      /* Get the available bytes that can be retrieved with a `mco_pop`. */
size_t mco_get_storage_size(mco_coro* co);                      /* Get the total storage size. */

/* Misc functions. */
mco_coro* mco_running(void);                        /* Returns the running coroutine for the current thread. */
const char* mco_result_description(mco_result res); /* Get the description of a result. */
```

# Complete Example

The following is a more complete example, generating Fibonacci numbers:

```c
#define MINICORO_IMPL
#include "minicoro.h"
#include <stdio.h>

static void fail(const char* message, mco_result res) {
  printf("%s: %s\n", message, mco_result_description(res));
  exit(-1);
}

static void fibonacci_coro(mco_coro* co) {
  unsigned long m = 1;
  unsigned long n = 1;

  /* Retrieve max value. */
  unsigned long max;
  mco_result res = mco_pop(co, &max, sizeof(max));
  if(res != MCO_SUCCESS)
    fail("Failed to retrieve coroutine storage", res);

  while(1) {
    /* Yield the next Fibonacci number. */
    mco_push(co, &m, sizeof(m));
    res = mco_yield(co);
    if(res != MCO_SUCCESS)
      fail("Failed to yield coroutine", res);

    unsigned long tmp = m + n;
    m = n;
    n = tmp;
    if(m >= max)
      break;
  }

  /* Yield the last Fibonacci number. */
  mco_push(co, &m, sizeof(m));
}

int main() {
  /* Create the coroutine. */
  mco_coro* co;
  mco_desc desc = mco_desc_init(fibonacci_coro, 0);
  mco_result res = mco_create(&co, &desc);
  if(res != MCO_SUCCESS)
    fail("Failed to create coroutine", res);

  /* Set storage. */
  unsigned long max = 1000000000;
  mco_push(co, &max, sizeof(max));

  int counter = 1;
  while(mco_status(co) == MCO_SUSPENDED) {
    /* Resume the coroutine. */
    res = mco_resume(co);
    if(res != MCO_SUCCESS)
      fail("Failed to resume coroutine", res);

    /* Retrieve storage set in last coroutine yield. */
    unsigned long ret = 0;
    res = mco_pop(co, &ret, sizeof(ret));
    if(res != MCO_SUCCESS)
      fail("Failed to retrieve coroutine storage", res);
    printf("fib %d = %lu\n", counter, ret);
    counter = counter + 1;
  }

  /* Destroy the coroutine. */
  res = mco_destroy(co);
  if(res != MCO_SUCCESS)
    fail("Failed to destroy coroutine", res);
  return 0;
}
```

# Updates

- **08-Jun-2022**: Minicoro has been awarded by the [Icculus Microgrant 2021](https://icculus.org/microgrant/), thanks @icculus for supporting open source work.
- **26-Jan-2022**: Added support for WebAssembly outside the WebBrowser using Binaryen asyncify pass.
- **01-Sep-2021**: Added support for DOSBox (MS-DOS Emulator).
- **30-Aug-2021**: Fix stack overflow crash on Windows 32 bits.
- **22-Aug-2021**: Added checks for stack overflow and iOS support (thanks @srberg).
- **12-Mar-2021**: Added support for RISC-V RV32.
- **19-Jan-2021**: Fix compilation and issues on Mac OS X, release v0.1.1.
- **19-Jan-2021**: First release, v0.1.0.
- **18-Jan-2021**: Fix issues when using Clang on Linux.
- **17-Jan-2021**: Add support for RISC-V 64 bits.
- **16-Jan-2021**: Add support for Mac OS X x86_64, thanks @RandyGaul for testing, debugging and researching about it.
- **15-Jan-2021**: Make assembly method the default one on Windows x86_64. Redesigned the storage API, thanks @RandyGaul for the suggestion.
- **14-Jan-2021**: Add support for running with ASan (AddressSanitizer) and TSan (ThreadSanitizer).
- **13-Jan-2021**: Add support for ARM and WebAssembly. Add Public Domain and MIT No Attribution license.
- **12-Jan-2021**: Some API changes and improvements.
- **11-Jan-2021**: Support valgrind and add benchmarks.
- **10-Jan-2021**: Minor API improvements and document more.
- **09-Jan-2021**: Library created.

# Donation

I'm a full-time open source developer.
Any amount of the donation will be appreciated and could bring me encouragement to keep supporting this and other open source projects.

[![Become a Patron](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/edubart)

# License

Your choice of either Public Domain or MIT No Attribution, see LICENSE file.
