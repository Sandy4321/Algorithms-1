PROJECT(setikit C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set SETIkit library version 
SET(setikit_VERSION_MAJOR 0)
SET(setikit_VERSION_MINOR 1)
SET(setikit_VERSION "${setikit_VERSION_MAJOR}.${setikit_VERSION_MINOR}.$(setikit_VERSION_PATH}")

# Define the paths for the compiled libraries and executables for our out-of-source compilation
SET (LIBRARY_OUTPUT_PATH ${setikit_BINARY_DIR}/lib CACHE PATH "Single output directory for building all libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${setikit_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")


# include and library paths required for the projects
SET(lib_INCLUDE_PATHS 
  $ENV{FFTW_HOME}/include
  )
SET(lib_LIBRARY_PATHS 
  $ENV{FFTW_HOME}/lib
  )

# Do things differently on OSX
if(APPLE)
  set(lib_INCLUDE_PATHS ${lib_INCLUDE_PATHS} /sw/include)
  set(lib_LIBRARY_PATHS ${lib_LIBRARY_PATHS} /sw/lib)
endif(APPLE)

# Directory where custom cmake files reside
SET (CMAKE_MODULE_PATH ${setikit_SOURCE_DIR}/CMake)

# Check for mandatory libraries; abort and do not create a makefile if these are not found.
find_package(FFTW)
find_package(GSL)

SET(ALL_FOUND 1)
IF(FFTW_FOUND)
  SET(HAVE_LIBFFTW 1)
  INCLUDE_DIRECTORIES(${FFTW_INCLUDE_DIR})
  LINK_LIBRARIES(${FFTW_LIBRARIES})
ELSE()
  SET(HAVE_FFTW 0)
  SET(ALL_FOUND 0)
  MESSAGE("Fatal: FFTW not found.")
ENDIF(FFTW_FOUND)

IF(GSL_FOUND)
  SET(HAVE_GSL 1)
  INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
  LINK_LIBRARIES(${GSL_LIBRARIES})
ELSE()
  SET(HAVE_GSL 0)
  SET(ALL_FOUND 0)
  MESSAGE("Fatal: GNU Scientific Library (GSL) not found.")
ENDIF(GSL_FOUND)

IF(ALL_FOUND)
  MESSAGE("===================================================================")
  MESSAGE("-- Congratulations! All required libraries were successfully found.")
  MESSAGE("===================================================================")
ELSE()
  MESSAGE("===================================================================")
  MESSAGE(FATAL_ERROR "-- Not all required libraries were found. Aborting.")
  MESSAGE("===================================================================")
ENDIF(ALL_FOUND)

# Done searching libraries. Start building SETIkit.
MESSAGE("-- Ready to build SETIkit")

INCLUDE_DIRECTORIES(
  ${setikit_SOURCE_DIR}/src
  ${setikit_BINARY_DIR}/src
  )

SUBDIRS(src)
