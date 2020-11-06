# Linux Syscall Support (LSS)

Every so often, projects need to directly embed Linux system calls instead of
calling the implementations in the system runtime library.

This project provides a header file that can be included into your application
whenever you need to make direct system calls.

The goal is to provide an API that generally mirrors the standard C library
while still making direct syscalls.  We try to hide some of the differences
between arches when reasonably feasible.  e.g. Newer architectures no longer
provide an `open` syscall, but do provide `openat`.  We will still expose a
`sys_open` helper by default that calls into `openat` instead.

We explicitly do not expose the raw syscall ABI including all of its historical
warts to the user.  We want people to be able to easily make a syscall, not have
to worry that on some arches size args are swapped or they are shifted.

Please be sure to review the Caveats section below however.

## How to include linux\_syscall\_support.h in your project

You can either copy the file into your project, or preferably, you can set up
Git submodules to automatically pull from our source repository.

## Supported targets

The following architectures/ABIs have been tested (at some point) and should
generally work.  If you don't see your combo listed here, please double check
the header itself as this list might be out of date.

* x86 32-bit (i.e. i386, i486, i586, i686, Intel, AMD, etc...)
* [x86_64 64-bit](https://en.wikipedia.org/wiki/X86-64) (i.e. x86-64, amd64, etc...)
* [x32 32-bit](https://sites.google.com/site/x32abi/)
* [ARM 32-bit](https://en.wikipedia.org/wiki/ARM_architecture) OABI
* [ARM 32-bit](https://en.wikipedia.org/wiki/ARM_architecture) EABI (i.e. armv6, armv7, etc...)
* AARCH64 64-bit (i.e. arm64, armv8, etc...)
* PowerPC 32-bit (i.e. ppc, ppc32, etc...)
* MIPS 32-bit o32 ABI
* MIPS 32-bit n32 ABI
* MIPS 64-bit n64 ABI

## API

By default, you can just add a `sys_` prefix to any function you want to call.
So if you want to call `open(...)`, use `sys_open(...)` instead.

### Knobs

The linux\_syscall\_support.h header provides many knobs for you to control
the exported API.  These are all documented in the top of the header in a big
comment block, so refer to that instead.

## Caveats

### ABI differences

Some functions that the standard C library exposes use a different ABI than
what the Linux kernel uses.  Care must be taken when making syscalls directly
that you use the right structure and flags.  e.g. Most C libraries define a
`struct stat` (commonly in `sys/stat.h` or `bits/stat.h`) that is different
from the `struct stat` the kernel uses (commonly in `asm/stat.h`).  If you use
the wrong structure layout, then you can see errors like memory corruption or
weird/shifted values.  If you plan on making syscalls directly, you should
focus on headers that are available under the `linux/` and `asm/` namespaces.

Note: LSS provides structs for most of these cases.  For `sys_stat()`, it
provides `struct kernel_stat` for you to use.

### Transparent backwards compatibility with older kernels

While some C libraries (notably, glibc) take care to fallback to older syscalls
when running on older kernels, there is no such support in LSS.  If you plan on
trying to run on older kernels, you will need to handle errors yourself (e.g.
`ENOSYS` when using a too new syscall).

Remember that this can happen with new flag bits too.  e.g. The `O_CLOEXEC`
flag was added to many syscalls, but if you try to run use it on older kernels,
it will fail with `EINVAL`.  In that case, you must handle the fallback logic
yourself.

### Variable arguments (varargs)

We do not support vararg type functions.  e.g. While the standard `open()`
function can accept 2 or 3 arguments (with the mode field being optional),
the `sys_open()` function always requires 3 arguments.

## Bug reports & feature requests

If you wish to report a problem or request a feature, please file them in our
[bug tracker](https://bugs.chromium.org/p/linux-syscall-support/issues/).

Please do not post patches to the tracker.  Instead, see below for how to send
patches to us directly.

While we welcome feature requests, please keep in mind that it is unlikely that
anyone will find time to implement them for you.  Sending patches is strongly
preferred and will often move things much faster.

## Projects that use LSS

* [Chromium](https://www.chromium.org/)
* [Breakpad](https://chromium.googlesource.com/breakpad/breakpad)
* [Native Client](https://developer.chrome.com/native-client), in nacl\_bootstrap.c

## How to get an LSS change committed

### Review

You get your change reviewed, you can upload it to
[Gerrit](https://chromium-review.googlesource.com/q/project:linux-syscall-support+status:open)
using `git cl upload` from
[Chromium's depot-tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html).

### Testing

Tests are found in the [tests/](./tests/) subdirectory.  It does not (yet) offer
100% coverage, but should grow over time.

New commits that update/change/add syscall wrappers should include tests for
them too.  Consult the [test documentation](./tests/README.md) for more details.

To run, just run `make` inside the tests directory.  It will compile & execute
the tests locally.

There is some limited cross-compile coverage available if you run `make cross`.
It only compiles things (does not execute at all).

### Rolling into Chromium

If you commit a change to LSS, please also commit a Chromium change to update
`lss_revision` in
[Chromium's DEPS](https://chromium.googlesource.com/chromium/src/+/master/DEPS)
file.

This ensures that the LSS change gets tested, so that people who commit later
LSS changes don't run into problems with updating `lss_revision`.
