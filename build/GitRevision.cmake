include (FindGit)
if (GIT_EXECUTABLE)
      execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            OUTPUT_VARIABLE GIT_REVISION
            )
      endif (GIT_EXECUTABLE)

if (NOT GIT_REVISION)
      set(GIT_REVISION "Unknown")
      endif(NOT GIT_REVISION)

file(WRITE "${PROJECT_SOURCE_DIR}/mscore/revision.h" "${GIT_REVISION}")
