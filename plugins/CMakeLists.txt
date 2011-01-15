if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)


include_directories(
        ${KOMAIN_INCLUDES}
)

macro_optional_add_subdirectory( scan )
macro_optional_add_subdirectory( colorengines )
macro_optional_add_subdirectory( artistictextshape )
macro_optional_add_subdirectory( dockers )
macro_optional_add_subdirectory( variables )
macro_optional_add_subdirectory( defaultTools )
macro_optional_add_subdirectory( pathshapes )
macro_optional_add_subdirectory( pictureshape )
macro_optional_add_subdirectory( textshape )
