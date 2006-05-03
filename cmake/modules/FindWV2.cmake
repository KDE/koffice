# - Try to find the WV2 library
# Once done this will define
#
#  WV2_FOUND - system has the WV2 library
#  WV2_INCLUDE_DIR - the WV2 include directory
#  WV2_LIBRARIES - The libraries needed to use WV2

IF (WV2_LIBRARIES AND WV2_INCLUDE_DIR)
  # in cache already
  SET(WV2_FOUND TRUE)

ELSE (WV2_LIBRARIES AND WV2_INCLUDE_DIR)
	
  FIND_PROGRAM(WV2CONFIG_EXECUTABLE NAMES wv2-config PATHS
     /usr/bin
     /usr/local/bin
  )

  # if wv2-config has been found
  IF (WV2CONFIG_EXECUTABLE)
    EXEC_PROGRAM(${WV2CONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE WV2_LIBRARIES)

    EXEC_PROGRAM(${WV2CONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE WV2_INCLUDE_DIR)

    IF (WV2_LIBRARIES AND WV2_INCLUDE_DIR)
      SET(WV2_FOUND TRUE)
      message(STATUS "Found wv2: ${WV2_LIBRARIES}")
    ENDIF (WV2_LIBRARIES AND WV2_INCLUDE_DIR)

    # ensure that they are cached
    set(WV2_INCLUDE_DIR ${WV2_INCLUDE_DIR} CACHE INTERNAL "The wv2 include path")
    set(WV2_LIBRARIES ${WV2_LIBRARIES} CACHE INTERNAL "The libraries needed to use libraries")

  ENDIF (WV2CONFIG_EXECUTABLE)

ENDIF (WV2_LIBRARIES AND WV2_INCLUDE_DIR)

