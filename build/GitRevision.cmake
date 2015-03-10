include (FindGit)
if (GIT_EXECUTABLE)
      execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_REVISION
            )
      endif (GIT_EXECUTABLE)

if (NOT GIT_REVISION)
      set(GIT_REVISION "Unknown")
      endif(NOT GIT_REVISION)

set (REVISION_H "${PROJECT_SOURCE_DIR}/mscore/revision.h")

if (EXISTS "${REVISION_H}")
  file(READ "${REVISION_H}" TEST_GIT_REVISION)
else (EXISTS "${REVISION_H}")
  set(TEST_GIT_REVISION "")
endif (EXISTS "${REVISION_H}")

if (NOT ("${TEST_GIT_REVISION}" STREQUAL "${GIT_REVISION}"))
  file(WRITE "${REVISION_H}" "${GIT_REVISION}")
endif (NOT ("${TEST_GIT_REVISION}" STREQUAL "${GIT_REVISION}"))
