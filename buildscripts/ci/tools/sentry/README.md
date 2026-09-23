# Crash dumps

## Capturing

Crashes are captured by [Crashpad](https://chromium.googlesource.com/crashpad/crashpad/+/master/README.md).
The client is part of the Diagnostics module and starts a `crashpad_handler`
process at startup (see `DiagnosticsModule::onInit`); the handler snapshots the
crashing process, writes a minidump next to the logs, and — if the user agreed
to send crash reports — uploads it to Sentry.

## Debug files

A minidump on its own is a list of addresses. Turning it into a readable stack
needs two kinds of information, which on most platforms live in two files:

| | debug info (functions, files, lines) | unwind info (walking the stack) |
|---|---|---|
| macOS | `mscore.dSYM` | `mscore` |
| Windows | `MuseScore4.pdb` | `MuseScore4.exe` |
| Linux | the unstripped binary | the same binary |

CI uploads these to Sentry in the `Upload debug files` step, which runs
`buildscripts/ci/tools/sentry/upload_debug_files.sh`. Sentry reads the formats
directly, so nothing is converted along the way.

Two things to know:

- Only **stable** builds upload debug files. Nightly builds still report
  crashes, but those reports stay unsymbolicated.
- On macOS the DWARF stays in the object files, so a separate `dsymutil` step
  (`buildscripts/ci/macos/generate_dsym.sh`) collects it into a dSYM before
  packaging, while the object files and the debug map are still around.

Debug files must match the build the dump came from. They are matched by debug
id — `LC_UUID` on macOS, the PDB GUID on Windows, the build id on Linux — which
survives stripping and signing, so the files collected during the build stay
valid for what ships.

## Reading a dump locally

Get the debug files for that exact build from Sentry first:
**Project Settings → Debug Files**, search by the debug id from the report.

**macOS** — LLDB reads minidumps natively:

```sh
lldb --core crash.dmp MuseScore\ 4.app/Contents/MacOS/mscore
(lldb) add-dsym /path/to/mscore.dSYM
(lldb) bt all
```

**Windows** — open the dump in WinDbg and point it at the PDB:

```
.sympath+ C:\path\to\symbols
.reload /f
!analyze -v
```

**Linux** — there is no comparable native viewer; read the report in Sentry.

To see what a debug file actually contains before trusting it:

```sh
sentry-cli debug-files check /path/to/file
```
