project(koffice)

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

 ############
#############
## Options ##
#############
############

 ######################
#######################
## Constants defines ##
#######################
######################

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "8.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "8")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

#
# WARNING: don't change the SHOULD_BUILD_app if an application
# doesn't build, but rather add the following line:
# option(BUILD_app "Build directory app" OFF)
#
#eg:

option(GHNS "support Get Hot New Stuff" ON)

if (DEFINED BUILD_kword )
    set(SHOULD_BUILD_KWORD ${BUILD_kword})
else (DEFINED BUILD_kword )
    set(SHOULD_BUILD_KWORD TRUE)
endif (DEFINED BUILD_kword )

if (DEFINED BUILD_kspread )
    set(SHOULD_BUILD_KSPREAD ${BUILD_kspread})
else (DEFINED BUILD_kspread )
    set(SHOULD_BUILD_KSPREAD TRUE)
endif (DEFINED BUILD_kspread )

if (DEFINED BUILD_karbon )
    set(SHOULD_BUILD_KARBON ${BUILD_karbon})
else (DEFINED BUILD_karbon )
    set(SHOULD_BUILD_KARBON TRUE)
endif (DEFINED BUILD_karbon )

if (DEFINED BUILD_kivio )
    set(SHOULD_BUILD_KIVIO ${BUILD_kivio})
else (DEFINED BUILD_kivio )
    set(SHOULD_BUILD_KIVIO TRUE)
endif (DEFINED BUILD_kivio )

if (DEFINED BUILD_showcase )
    set(SHOULD_BUILD_SHOWCASE ${BUILD_showcase})
else (DEFINED BUILD_showcase )
    set(SHOULD_BUILD_SHOWCASE TRUE)
endif (DEFINED BUILD_showcase )

set(SHOULD_BUILD_KOUNAVAIL TRUE)
set(SHOULD_BUILD_SCRIPTING TRUE)

if(KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################
set(KDE_MIN_VERSION "4.5.0")
find_package(KDE4 4.5.0 REQUIRED)

find_package(Qt4 4.7.0 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)
include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

 ################################
#################################
## Check for OpenEXR           ##
#################################
################################

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)

 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Look for OpenGL
##

macro_optional_find_package(OpenGL)
set(HAVE_OPENGL 0)

if(OPENGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
    if(QT_QTOPENGL_FOUND)
        message(STATUS "Found Qt OpenGL support")
        set(HAVE_OPENGL 1)
    else(QT_QTOPENGL_FOUND)
        message(STATUS "Did NOT find Qt OpenGL support. Check your Qt configuration")
    endif(QT_QTOPENGL_FOUND)
else(OPENGL_FOUND)
    message(STATUS "Did NOT find OpenGL libraries")
endif(OPENGL_FOUND)

macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Optionally required by flake")

##
## Test for create ressources
##
macro_optional_find_package(CreateResources)
macro_log_feature(CREATERESOURCES_FOUND "Create Resources" "Create Resources (brushes, palettes etc.)" "http://create.freedesktop.org" FALSE "" "Recommended at runtime for KOffice for optional extra resources.")

##
## Test for kdepimlibs
##
macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "KDE PIMLibs" "KDE Personal Information Management Libraries" "http://www.kde.org/" FALSE "" "Required by the KDE address book integration (available as a module in KDE)")

##
## Test for boost
##
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "" "Required by showcase")

##
## Test for eigen2
##
macro_optional_find_package(Eigen2)
macro_log_feature(EIGEN2_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "2.0" "Required by KSpread")

##
## Test for QCA2
##
macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0" "Required for encrypted OpenDocument files and encrypted xls files support (available as a module in kdesupport)")

##
## Test for soprano
##
macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "KDE4 RDF handling library" "http://soprano.sourceforge.net/" FALSE "" "Required to handle RDF metadata in ODF")
if(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF FALSE)
    set(SOPRANO_INCLUDE_DIR "")
else(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF TRUE)
    add_definitions( -DSHOULD_BUILD_RDF )
endif(NOT Soprano_FOUND)


##
## Test for lcms
##
macro_optional_find_package(LCMS)
set(REQUIRED_LCMS_VERSION 118)
if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS TRUE)
else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION FALSE)
    set(HAVE_LCMS FALSE)
endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

macro_optional_find_package(LCMS2)
macro_log_feature(LCMS2_FOUND "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "2.0" "Optional replacement for LittleCMS 1.18. Can be used for color management")
if(LCMS2_FOUND)
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS TRUE)
endif(LCMS2_FOUND)

macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "1.18" "Required for color management")

#Set the build of TextShape changetraker

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})

if(WIN32)
    # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
    execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(TO_CMAKE_PATH "${_dir}" __dir)
    find_path(KDE4_ICON_DIR oxygen PATHS
        ${__dir}
    )
    message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")

    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )


else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)

##
## Test endianess
##
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

##
## Generate a file for prefix information
##

 #############################
##############################
## Add the benchmark target ##
##############################
#############################

include(MacroKOfficeAddBenchmark)

 ####################
#####################
## Define includes ##
#####################
####################

# WARNING: make sure that QT_INCLUDES is the first directory to be added to include_directory before
#          any other include directory

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces )

# koodf is at the bottom of the stack
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf
                   ${CMAKE_BINARY_DIR}/libs/odf
                   ${KDE4_INCLUDES})

# pigment depends on koplugin and lcms
set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/pigment
                   ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops
                   ${CMAKE_SOURCE_DIR}/libs/pigment/resources
                   ${QT_INCLUDES}
                   ${KDE4_INCLUDES})

# flake depends on koodf and pigment
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake
                   ${KOODF_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools
                   ${CMAKE_BINARY_DIR}/libs/flake)

# KoText depends on flake
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext
                    # this one should not be here, its needed for rdf stuff right now...
                   ${CMAKE_SOURCE_DIR}/libs/main
                   ${CMAKE_BINARY_DIR}/libs/kotext
                   ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                   ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                   ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                   ${SOPRANO_INCLUDE_DIR}
                   ${FLAKE_INCLUDES})

# komain depends on kotext & flake
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${KOTEXT_INCLUDES}
                    ${PIGMENT_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/main/config)

if(SHOULD_BUILD_SCRIPTING)
  set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
endif(SHOULD_BUILD_SCRIPTING)

# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

# koproperty
SET(KOPROPERTY_TARGETNAME koproperty)                           # TODO remove when we move to independent place
SET(KOPROPERTY_LIBS ${KOPROPERTY_TARGETNAME})                   # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_VERSION ${GENERIC_KOFFICE_LIB_VERSION})      # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION})  # TODO remove when we move to independent place
SET(KOPROPERTY_USE_KOLIBS TRUE)                                 # TODO remove when we move to independent place
set(KOPROPERTY_INCLUDES ${KOODF_INCLUDES}
                        ${CMAKE_SOURCE_DIR}/libs
                        ${CMAKE_SOURCE_DIR}/libs/widgets)

 #############################################
##############################################
## Detect which applications can be compile ##
##############################################
#############################################

#
# WARNING: when adding a dependency if there is allready a line
# containing SHOULD_BUILD_app, don't add a new one, just add
# the test to the if construct
#


### KSpread: eigen2
if(NOT EIGEN2_FOUND)
    set(SHOULD_BUILD_KSPREAD FALSE)
endif(NOT EIGEN2_FOUND)

### showcase: boost
if(NOT Boost_FOUND)
    set(SHOULD_BUILD_SHOWCASE FALSE)
endif(NOT Boost_FOUND)

 ###################
####################
## Subdirectories ##
####################
###################

if(SHOULD_BUILD_KWORD)
    add_subdirectory(kword)
endif(SHOULD_BUILD_KWORD)

if (SHOULD_BUILD_KIVIO)
    add_subdirectory(kivio)
endif (SHOULD_BUILD_KIVIO)

if (SHOULD_BUILD_KOUNAVAIL)
    add_subdirectory(kounavail)
endif (SHOULD_BUILD_KOUNAVAIL)

if(SHOULD_BUILD_SHOWCASE)
  add_subdirectory(showcase)
endif(SHOULD_BUILD_SHOWCASE)

if(SHOULD_BUILD_KSPREAD)
    add_subdirectory(kspread)
endif(SHOULD_BUILD_KSPREAD)

if(SHOULD_BUILD_KARBON)
  add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)

macro_optional_add_subdirectory(doc)

# non-app directories are moved here because they can depend on SHOULD_BUILD_{appname} variables set above
add_subdirectory(libs)
add_subdirectory(interfaces)
add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(tools)
add_subdirectory(filters)

macro_display_feature_log()

macro(macro_display_application_enabled _appName _shouldBuild)
if( BUILD_${_appName} AND ${_shouldBuild} MATCHES "[Tt][Rr][Uu][Ee]" )
  message(STATUS ${_appName} )
endif( BUILD_${_appName} AND ${_shouldBuild} MATCHES "[Tt][Rr][Uu][Ee]" )
endmacro(macro_display_application_enabled)

message(STATUS "---------------- The following applications will be built ----------------" )

macro_display_application_enabled(kword ${SHOULD_BUILD_KWORD})
macro_display_application_enabled(kspread ${SHOULD_BUILD_KSPREAD})
macro_display_application_enabled(showcase ${SHOULD_BUILD_SHOWCASE})
macro_display_application_enabled(kivio ${SHOULD_BUILD_KIVIO})
macro_display_application_enabled(karbon ${SHOULD_BUILD_KARBON})

message(STATUS "--------------------------------------------------------------------------" )

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

