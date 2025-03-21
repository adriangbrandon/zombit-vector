IF(NOT EXISTS "/Users/adrian/CLionProjects/succ-vectors/cmake-build-release/external/sdsl-lite/external/libdivsufsort/install_manifest.txt")
  MESSAGE(FATAL_ERROR "Cannot find install manifest: \"/Users/adrian/CLionProjects/succ-vectors/cmake-build-release/external/sdsl-lite/external/libdivsufsort/install_manifest.txt\"")
ENDIF(NOT EXISTS "/Users/adrian/CLionProjects/succ-vectors/cmake-build-release/external/sdsl-lite/external/libdivsufsort/install_manifest.txt")

FILE(READ "/Users/adrian/CLionProjects/succ-vectors/cmake-build-release/external/sdsl-lite/external/libdivsufsort/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")

SET(NUM 0)
FOREACH(file ${files})
  IF(EXISTS "$ENV{DESTDIR}${file}")
    MESSAGE(STATUS "Looking for \"$ENV{DESTDIR}${file}\" - found")
    SET(UNINSTALL_CHECK_${NUM} 1)
  ELSE(EXISTS "$ENV{DESTDIR}${file}")
    MESSAGE(STATUS "Looking for \"$ENV{DESTDIR}${file}\" - not found")
    SET(UNINSTALL_CHECK_${NUM} 0)
  ENDIF(EXISTS "$ENV{DESTDIR}${file}")
  MATH(EXPR NUM "1 + ${NUM}")
ENDFOREACH(file)

SET(NUM 0)
FOREACH(file ${files})
  IF(${UNINSTALL_CHECK_${NUM}})
    MESSAGE(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    EXEC_PROGRAM(
      "/Applications/CLion.app/Contents/bin/cmake/mac/x64/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
      OUTPUT_VARIABLE rm_out
      RETURN_VALUE rm_retval
      )
    IF(NOT "${rm_retval}" STREQUAL 0)
      MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    ENDIF(NOT "${rm_retval}" STREQUAL 0)
  ENDIF(${UNINSTALL_CHECK_${NUM}})
  MATH(EXPR NUM "1 + ${NUM}")
ENDFOREACH(file)

FILE(REMOVE "/Users/adrian/CLionProjects/succ-vectors/cmake-build-release/external/sdsl-lite/external/libdivsufsort/install_manifest.txt")
