# Check if this is a in tree build.
if (NOT DEFINED LLVM_MAIN_SRC_DIR)
  project(Polly)
  cmake_minimum_required(VERSION 3.20.0)
  set(POLLY_STANDALONE_BUILD TRUE)
endif()
set(LLVM_SUBPROJECT_TITLE "Polly")

# Must go below project(..)
include(GNUInstallDirs)

if(POLLY_STANDALONE_BUILD)
  # Where is LLVM installed?
  find_package(LLVM CONFIG REQUIRED)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_CMAKE_DIR})

  separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
  add_definitions(${LLVM_DEFINITIONS_LIST})
  list(APPEND CMAKE_REQUIRED_DEFINITIONS ${LLVM_DEFINITIONS_LIST})

  include(HandleLLVMOptions)
  include(AddLLVM)

  # Add the llvm header path.
  include_directories(${LLVM_INCLUDE_DIRS})

  # Sources available, too?
  if (LLVM_BUILD_MAIN_SRC_DIR)
    set(default_llvm_src "${LLVM_BUILD_MAIN_SRC_DIR}")
  else()
    set(default_llvm_src "${CMAKE_CURRENT_SOURCE_DIR}/../llvm")
  endif()
  set(LLVM_SOURCE_ROOT ${default_llvm_src} CACHE PATH "Path to LLVM source tree")

  # Enable unit tests if available.
  set(POLLY_GTEST_AVAIL 0)
  if(TARGET llvm_gtest)
    set(POLLY_GTEST_AVAIL 1)
  endif()

  if (LLVM_ENABLE_PIC)
    # Make sure the isl c files are built as fPIC if possible
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif ()

  # Set directory for polly-isl-test.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
else ()
  set(LLVM_SOURCE_ROOT "${LLVM_MAIN_SRC_DIR}")
  set(POLLY_GTEST_AVAIL 1)
endif ()

set(POLLY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(POLLY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(NOT DEFINED LLVM_COMMON_CMAKE_UTILS)
  set(LLVM_COMMON_CMAKE_UTILS ${POLLY_SOURCE_DIR}/../cmake)
endif()

# Make sure that our source directory is on the current cmake module path so that
# we can include cmake files from this directory.
list(INSERT CMAKE_MODULE_PATH 0
  "${POLLY_SOURCE_DIR}/cmake"
  "${LLVM_COMMON_CMAKE_UTILS}/Modules"
  )

include("polly_macros")

# Add appropriate flags for GCC
if (CMAKE_COMPILER_IS_GNUCXX)
  # FIXME: Turn off exceptions, RTTI:
  # -fno-exceptions -fno-rtti
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common -Woverloaded-virtual -Wno-long-long -Wall -W -Wno-unused-parameter -Wwrite-strings -fno-exceptions -fno-rtti")
elseif (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
  add_definitions("-D_HAS_EXCEPTIONS=0")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif ()

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(POLLY_ENABLE_BARVINOK
  "Enable barvinok for parametric Ehrhart trip-count computation" OFF)

if(POLLY_ENABLE_BARVINOK)
  include(ExternalProject)

  set(BARVINOK_SRC "${CMAKE_SOURCE_DIR}/../../../third-party/barvinok")
  set(ISL_SRC      "${BARVINOK_SRC}/isl")
  set(ISL_INSTALL      "${CMAKE_BINARY_DIR}/isl-install")
  set(BARVINOK_INSTALL "${CMAKE_BINARY_DIR}/barvinok-install")

  if(LLVM_ENABLE_PIC)
    set(BARVINOK_WITH_PIC "--with-pic")
  else()
    set(BARVINOK_WITH_PIC "")
  endif()

  # Build ISL from barvinok's bundled isl/ — the single ISL used by both
  # Polly and barvinok (avoids duplicate symbols at link time).
  # autogen.sh must run first because isl/configure is not checked in.
  ExternalProject_Add(isl_ext
    SOURCE_DIR ${ISL_SRC}
    CONFIGURE_COMMAND
      sh -c "test -f ${ISL_SRC}/configure || (cd ${ISL_SRC} && ./autogen.sh)"
      COMMAND ${ISL_SRC}/configure
        --prefix=${ISL_INSTALL}
        --enable-shared
        --enable-static
        ${BARVINOK_WITH_PIC}
    BUILD_COMMAND   make -j
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS ${ISL_INSTALL}/lib/libisl.a
  )

  # Build barvinok against the same isl-install/.
  # polylib/configure is not checked in either — run its autogen.sh first.
  ExternalProject_Add(barvinok_ext
    SOURCE_DIR ${BARVINOK_SRC}
    DEPENDS    isl_ext
    CONFIGURE_COMMAND
      sh -c "test -f ${BARVINOK_SRC}/polylib/configure || (cd ${BARVINOK_SRC}/polylib && ./autogen.sh)"
      COMMAND ${BARVINOK_SRC}/configure
        --prefix=${BARVINOK_INSTALL}
        --with-isl=system
        --with-isl-prefix=${ISL_INSTALL}
        --with-polylib=bundled
        --enable-shared
        --enable-static
        ${BARVINOK_WITH_PIC}
    BUILD_COMMAND   make -j
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS
      ${BARVINOK_INSTALL}/lib/libbarvinok.a
      ${BARVINOK_INSTALL}/lib/libpolylibgmp.a
  )

  # ISL imported target — Polly links this instead of its bundled PollyISL.
  file(MAKE_DIRECTORY ${ISL_INSTALL}/include)
  add_library(ISL STATIC IMPORTED GLOBAL)
  set_target_properties(ISL PROPERTIES
    IMPORTED_LOCATION             ${ISL_INSTALL}/lib/libisl.a
    INTERFACE_INCLUDE_DIRECTORIES ${ISL_INSTALL}/include
  )
  add_dependencies(ISL isl_ext)

  # Override the POLLY_BUNDLED_ISL default: we provide our own ISL target.
  set(POLLY_BUNDLED_ISL OFF CACHE BOOL
    "Use the bundled version of libisl included in Polly" FORCE)
  # Keep Polly's source ISL include path so Polly-specific headers like
  # isl/isl-noexceptions.h (not present in standard ISL) remain visible.
  set(ISL_INCLUDE_DIRS
    ${ISL_INSTALL}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/External/isl/include
  )
  set(ISL_TARGET ISL)

  # Barvinok imported target.
  file(MAKE_DIRECTORY ${BARVINOK_INSTALL}/include)
  add_library(Barvinok STATIC IMPORTED GLOBAL)
  set_target_properties(Barvinok PROPERTIES
    IMPORTED_LOCATION             ${BARVINOK_INSTALL}/lib/libbarvinok.a
    INTERFACE_INCLUDE_DIRECTORIES ${BARVINOK_INSTALL}/include
    INTERFACE_LINK_LIBRARIES      ${BARVINOK_INSTALL}/lib/libpolylibgmp.a
  )
  add_dependencies(Barvinok barvinok_ext)

  add_compile_definitions(POLLY_HAVE_BARVINOK)
  find_library(NTL_LIBRARY ntl REQUIRED)
  find_library(GMP_LIBRARY gmp REQUIRED)
endif()

option(POLLY_BUNDLED_ISL "Use the bundled version of libisl included in Polly" ON)
if (NOT POLLY_BUNDLED_ISL)
  if (NOT TARGET ISL)
    # POLLY_ENABLE_BARVINOK already created the ISL target above; only call
    # find_package when we genuinely need a system/external ISL.
    find_package(ISL MODULE REQUIRED)
    message(STATUS "Using external libisl ${ISL_VERSION} in: ${ISL_PREFIX}")
  endif()
  set(ISL_TARGET ISL)
else()
  set(ISL_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/lib/External/isl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/External/isl/include
  )
  set(ISL_TARGET PollyISL)
endif()

include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${ISL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/External/pet/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/External
  ${CMAKE_CURRENT_BINARY_DIR}/include
  )

if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
  install(DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
    PATTERN "*.h"
    )

  install(DIRECTORY ${POLLY_BINARY_DIR}/include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "CMakeFiles" EXCLUDE
    )
endif()

# add_llvm_pass_plugin() already declares the option, but we need access to
# it earlier than that.
set(link_into_tools_default OFF)
if (LLVM_TOOL_POLLY_BUILD)
  set(link_into_tools_default ON)
endif()
option(LLVM_POLLY_LINK_INTO_TOOLS "Statically link Polly into tools (if available)" ${link_into_tools_default})

add_definitions( -D_GNU_SOURCE )

add_subdirectory(docs)
add_subdirectory(lib)
add_subdirectory(test)
if (POLLY_GTEST_AVAIL)
  add_subdirectory(unittests)
endif ()
add_subdirectory(cmake)
# TODO: docs.


configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/include/polly/Config/config.h.cmake
                ${POLLY_BINARY_DIR}/include/polly/Config/config.h )

# Add target to check formatting of polly files
file( GLOB_RECURSE files *.h lib/*.cpp lib/*.c tools/*.cpp tools/*.c tools/*.h unittests/*.cpp)
file( GLOB_RECURSE external lib/External/*.h lib/External/*.c lib/External/*.cpp isl_config.h)
list( REMOVE_ITEM files ${external})

set(check_format_depends)
set(update_format_depends)
set(i 0)
foreach (file IN LISTS files)
  add_custom_command(OUTPUT polly-check-format${i}
    COMMAND clang-format -sort-includes -style=llvm ${file} | diff -u ${file} -
    VERBATIM
    COMMENT "Checking format of ${file}..."
  )
  list(APPEND check_format_depends "polly-check-format${i}")

  add_custom_command(OUTPUT polly-update-format${i}
    COMMAND clang-format -sort-includes -i -style=llvm ${file}
    VERBATIM
    COMMENT "Updating format of ${file}..."
  )
  list(APPEND update_format_depends "polly-update-format${i}")

  math(EXPR i ${i}+1)
endforeach ()

add_custom_target(polly-check-format DEPENDS ${check_format_depends})
set_target_properties(polly-check-format PROPERTIES FOLDER "Polly/Metatargets")

add_custom_target(polly-update-format DEPENDS ${update_format_depends})
set_target_properties(polly-update-format PROPERTIES FOLDER "Polly/Metatargets")
