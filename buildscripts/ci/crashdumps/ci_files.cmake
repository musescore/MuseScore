set(CI_FILES
    README.md
    view_dump.sh

    ci_generate_and_upload.cmake

    ci_generate_dumpsyms.cmake
    generate_syms.cmake
    ci_sentry_dumpsyms_upload.cmake

    posix/generate_breakpad_symbols.py
    win/generate_breakpad_symbols.py

    win/dump_syms.7z
    linux/aarch64/dump_syms.7z
    linux/armv7l/dump_syms.7z
    linux/x86-64/dump_syms.7z
    macos/dump_syms.7z
)
