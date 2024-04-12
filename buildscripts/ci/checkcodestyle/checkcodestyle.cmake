
if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This file is a script")
endif()

set(SCAN_DIR ${CMAKE_ARGV3})

set(UNCRUSTIFY_BIN ${CMAKE_CURRENT_LIST_DIR}/tools/linux/uncrustify_073)
set(SCAN_BIN ${CMAKE_CURRENT_LIST_DIR}/tools/linux/scan_files)
set(CONFIG ${CMAKE_CURRENT_LIST_DIR}/uncrustify_muse.cfg)
set(UNTIDY_FILE ".untidy")

execute_process(
    COMMAND bash ${CMAKE_CURRENT_LIST_DIR}/run_scan.sh ${SCAN_BIN} ${UNCRUSTIFY_BIN} ${CONFIG} ${UNTIDY_FILE} ${SCAN_DIR}
)

execute_process(
    COMMAND git diff --name-only
    OUTPUT_VARIABLE GIT_DIFF_OUT
)

if (GIT_DIFF_OUT)

    set(MSG
        "Error: Code style is incorrect in these files:

        ${GIT_DIFF_OUT}

        Please run tools/codestyle/uncrustify_run_file.sh on these files and
        then amend your commit.

        $ git show --name-only '*.h' '*.cpp' | xargs tools/codestyle/uncrustify_run_file.sh
        $ git show --name-only '*.h' '*.cpp' | xargs git add
        $ git commit --amend --no-edit

        If your PR contains multiple commits then you must do an interactive
        rebase and amend each commit in turn.

        $ git -c sequence.editor='sed -i s/^pick/edit/' rebase -i HEAD~\${NUM_COMMITS}

        Where \${NUM_COMMITS} is the number of commits in your PR.

        The required changes are...
    ")

    message(${MSG})

    execute_process(
        COMMAND git diff
        OUTPUT_VARIABLE GIT_DIFF_OUT
    )

    message(${GIT_DIFF_OUT})

endif()

