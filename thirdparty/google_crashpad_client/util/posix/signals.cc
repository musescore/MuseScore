// Copyright 2017 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "util/posix/signals.h"

#include <unistd.h>

#include <vector>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/stl_util.h"

namespace crashpad {

namespace {

// These are the core-generating signals.
//
// On macOS, these come from 10.12.3 xnu-3789.41.3/bsd/sys/signalvar.h sigprop:
// entries with SA_CORE are in the set.
//
// For Linux, see linux-4.4.52/kernel/signal.c get_signal() and
// linux-4.4.52/include/linux/signal.h sig_kernel_coredump(): signals in
// SIG_KERNEL_COREDUMP_MASK are in the set.
constexpr int kCrashSignals[] = {
    SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGILL,
    SIGQUIT,
    SIGSEGV,
    SIGSYS,
    SIGTRAP,
#if defined(SIGEMT)
    SIGEMT,
#endif  // defined(SIGEMT)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
    SIGXCPU,
    SIGXFSZ,
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
};

// These are the non-core-generating but terminating signals.
//
// On macOS, these come from 10.12.3 xnu-3789.41.3/bsd/sys/signalvar.h sigprop:
// entries with SA_KILL but not SA_CORE are in the set. SIGKILL is excluded
// because it is uncatchable.
//
// For Linux, see linux-4.4.52/kernel/signal.c get_signal() and
// linux-4.4.52/include/linux/signal.h sig_kernel_coredump(),
// sig_kernel_ignore(), and sig_kernel_stop(): signals not in
// SIG_KERNEL_COREDUMP_MASK, SIG_KERNEL_IGNORE_MASK, or SIG_KERNEL_STOP_MASK are
// in the set. SIGKILL is excluded because it is uncatchable (it’s in
// SIG_KERNEL_ONLY_MASK and qualifies for sig_kernel_only()). Real-time signals
// in the range [SIGRTMIN, SIGRTMAX) also have termination as the default
// action, although they are not listed here.
constexpr int kTerminateSignals[] = {
    SIGALRM,
    SIGHUP,
    SIGINT,
    SIGPIPE,
    SIGPROF,
    SIGTERM,
    SIGUSR1,
    SIGUSR2,
    SIGVTALRM,
#if defined(SIGPWR)
    SIGPWR,
#endif  // defined(SIGPWR)
#if defined(SIGSTKFLT)
    SIGSTKFLT,
#endif  // defined(SIGSTKFLT)
#if defined(OS_APPLE)
    SIGXCPU,
    SIGXFSZ,
#endif  // defined(OS_APPLE)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
    SIGIO,
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
};

bool InstallHandlers(const std::vector<int>& signals,
                     Signals::Handler handler,
                     int flags,
                     Signals::OldActions* old_actions,
                     const std::set<int>* unhandled_signals) {
  bool success = true;
  for (int sig : signals) {
    if (unhandled_signals &&
        unhandled_signals->find(sig) != unhandled_signals->end()) {
      continue;
    }
    success &= Signals::InstallHandler(
        sig,
        handler,
        flags,
        old_actions ? old_actions->ActionForSignal(sig) : nullptr);
  }
  return success;
}

bool IsSignalInSet(int sig, const int* set, size_t set_size) {
  for (size_t index = 0; index < set_size; ++index) {
    if (sig == set[index]) {
      return true;
    }
  }
  return false;
}

}  // namespace

struct sigaction* Signals::OldActions::ActionForSignal(int sig) {
  DCHECK_GT(sig, 0);
  const size_t slot = sig - 1;
  DCHECK_LT(slot, base::size(actions_));
  return &actions_[slot];
}

// static
bool Signals::InstallHandler(int sig,
                             Handler handler,
                             int flags,
                             struct sigaction* old_action) {
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = flags | SA_SIGINFO;
  action.sa_sigaction = handler;
  if (sigaction(sig, &action, old_action) != 0) {
    PLOG(ERROR) << "sigaction " << sig;
    return false;
  }
  return true;
}

// static
bool Signals::InstallDefaultHandler(int sig) {
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = SIG_DFL;
  return sigaction(sig, &action, nullptr) == 0;
}

// static
bool Signals::InstallCrashHandlers(Handler handler,
                                   int flags,
                                   OldActions* old_actions,
                                   const std::set<int>* unhandled_signals) {
  return InstallHandlers(
      std::vector<int>(kCrashSignals,
                       kCrashSignals + base::size(kCrashSignals)),
      handler,
      flags,
      old_actions,
      unhandled_signals);
}

// static
bool Signals::InstallTerminateHandlers(Handler handler,
                                       int flags,
                                       OldActions* old_actions) {
  return InstallHandlers(
      std::vector<int>(kTerminateSignals,
                       kTerminateSignals + base::size(kTerminateSignals)),
      handler,
      flags,
      old_actions,
      nullptr);
}

// static
bool Signals::WillSignalReraiseAutonomously(const siginfo_t* siginfo) {
  // Signals received other than via hardware faults, such as those raised
  // asynchronously via kill() and raise(), and those arising via hardware traps
  // such as int3 on x86 (resulting in SIGTRAP but advancing the instruction
  // pointer), will not reoccur on their own when returning from the signal
  // handler.
  //
  // Unfortunately, on macOS, when SIGBUS (on all CPUs) and SIGILL and SIGSEGV
  // (on arm64) is received asynchronously via kill(), siginfo->si_code makes it
  // appear as though it was actually received via a hardware fault. See 10.15.6
  // xnu-6153.141.1/bsd/dev/i386/unix_signal.c sendsig() and 10.15.6
  // xnu-6153.141.1/bsd/dev/arm/unix_signal.c sendsig(). Received
  // asynchronously, these signals will not re-raise themselves autonomously,
  // but this function (acting on information from the kernel) behaves as though
  // they will. This isn’t ideal, but these signals occurring asynchronously is
  // an unexpected condition. The alternative, to never treat these signals as
  // autonomously re-raising, is a bad idea because the explicit re-raise would
  // lose properties associated with the the original signal, which are valuable
  // for debugging and are visible to a Mach exception handler. Since these
  // signals are normally received synchronously in response to a hardware
  // fault, don’t sweat the unexpected asynchronous case.
  //
  // SIGSEGV on macOS on x86[_64] originating from a general protection fault is
  // a more difficult case: si_code is cleared, making the signal appear
  // asynchronous. See 10.15.6 xnu-6153.141.1/bsd/dev/i386/unix_signal.c
  // sendsig().
  const int sig = siginfo->si_signo;
  const int code = siginfo->si_code;

  // Only these signals can be generated from hardware faults and can re-raise
  // autonomously.
  return (sig == SIGBUS ||
          sig == SIGFPE ||
          sig == SIGILL ||
          sig == SIGSEGV) &&

         // The signal was only generated from a hardware fault if the code is a
         // positive number not matching one of these SI_* constants. See
         // “Signal Actions” under XRAT “Rationale”/B.2.4 “Signal Concepts” in
         // POSIX.1-2008, 2016 Edition, regarding si_code. The historical
         // behavior does not use these SI_* constants and signals generated
         // asynchronously show up with a code of 0. On macOS, the SI_*
         // constants are defined but never used, and the historical value of 0
         // remains. See 10.12.3 xnu-3789.41.3/bsd/kern/kern_sig.c
         // psignal_internal().
         (code > 0 &&
          code != SI_ASYNCIO &&
          code != SI_MESGQ &&
          code != SI_QUEUE &&
          code != SI_TIMER &&
          code != SI_USER &&
#if defined(SI_DETHREAD)
          code != SI_DETHREAD &&
#endif  // defiend(SI_DETHREAD)
#if defined(SI_KERNEL)
          // In Linux, SI_KERNEL is used for signals that are raised by the
          // kernel in software, opposing SI_USER. See
          // linux-4.4.52/kernel/signal.c __send_signal(). Signals originating
          // from hardware faults do not use this SI_KERNEL, but a proper signal
          // code translated in architecture-specific code from the
          // characteristics of the hardware fault.
          code != SI_KERNEL &&
#endif  // defined(SI_KERNEL)
#if defined(SI_SIGIO)
          code != SI_SIGIO &&
#endif  // defined(SI_SIGIO)
#if defined(SI_TKILL)
          code != SI_TKILL &&
#endif  // defined(SI_TKILL)
          true);
}

// static
void Signals::RestoreHandlerAndReraiseSignalOnReturn(
    const siginfo_t* siginfo,
    const struct sigaction* old_action) {
  // Failures in this function should _exit(kFailureExitCode). This is a quick
  // and quiet failure. This function runs in signal handler context, and it’s
  // difficult to safely be loud from a signal handler.
  constexpr int kFailureExitCode = 191;

  struct sigaction default_action;
  sigemptyset(&default_action.sa_mask);
  default_action.sa_flags = 0;
  default_action.sa_handler = SIG_DFL;

  const struct sigaction* restore_action =
      old_action ? old_action : &default_action;

  // Try to restore restore_action. If that fails and restore_action was
  // old_action, the problem may have been that old_action was bogus, so try to
  // set the default action.
  const int sig = siginfo->si_signo;
  if (sigaction(sig, restore_action, nullptr) != 0 && old_action &&
      sigaction(sig, &default_action, nullptr) != 0) {
    _exit(kFailureExitCode);
  }

  // Explicitly re-raise the signal if it will not re-raise itself. Because
  // signal handlers normally execute with their signal blocked, this raise()
  // cannot immediately deliver the signal. Delivery is deferred until the
  // signal handler returns and the signal becomes unblocked. The re-raised
  // signal will appear with the same context as where it was initially
  // triggered.
  if (!WillSignalReraiseAutonomously(siginfo) && raise(sig) != 0) {
    _exit(kFailureExitCode);
  }
}

// static
bool Signals::IsCrashSignal(int sig) {
  return IsSignalInSet(sig, kCrashSignals, base::size(kCrashSignals));
}

// static
bool Signals::IsTerminateSignal(int sig) {
  return IsSignalInSet(sig, kTerminateSignals, base::size(kTerminateSignals));
}

}  // namespace crashpad
