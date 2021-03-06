project(cisTEM)
cmake_minimum_required(VERSION 2.8)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Debug mode
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Experimental build option
option(BUILD_EXPERIMENTAL_FEATURES "Build experimental parts of cisTEM" OFF)
if (BUILD_EXPERIMENTAL_FEATURES)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEXPERIMENTAL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEXPERIMENTAL")
endif(BUILD_EXPERIMENTAL_FEATURES)

#
# Setup findFFTW
#
configure_file(downloadFindFFTW.cmake.in findFFTW-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
    message(FATAL_ERROR "CMake step for findFFTW failed: ${result}")
    else()
    message("CMake step for findFFTW completed (${result}).")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
    message(FATAL_ERROR "Build step for findFFTW failed: ${result}")
endif()

set(findFFTW_DIR ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${findFFTW_DIR}")
	
#
# Let's look for single-precision FFTW
#
find_package(FFTW REQUIRED COMPONENTS FLOAT_LIB)
include_directories(${FFTW_INCLUDE_DIRS})

#
# Let's look for wxWidgets (we can rely on cmake's built-in)
#
set(wxWidgets_USE_DEBUG false)	
find_package(wxWidgets REQUIRED COMPONENTS core base std net xml richtext aui)
include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS}) 

#
# Let's look for LibTIFF
#
find_package(TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIRS})

#
# Let's figure out which SVN revision we are building (if indeed we're using SVN at all)
#
find_package(Subversion)
if(SUBVERSION_FOUND)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.svn")
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} cisTEM)
        message("Current SVN revision is ${cisTEM_WC_REVISION}")
        execute_process(    COMMAND "${Subversion_SVN_EXECUTABLE}" propget --revprop -r "${cisTEM_WC_REVISION}" git-commit  "https://github.com/ngrigorieff/cisTEM/trunk"
                            OUTPUT_VARIABLE GIT_COMMIT_FROM_SVN )
        string(REGEX REPLACE "\n$" "" GIT_COMMIT_FROM_SVN "${GIT_COMMIT_FROM_SVN}")
        message("Current Git commit from SVN is ${GIT_COMMIT_FROM_SVN}")
        add_definitions(" -DCISTEM_SVN_REV=\"\\\"${cisTEM_WC_REVISION}\\\"\" " " -DCISTEM_GIT_COMMIT=\"\\\"${GIT_COMMIT_FROM_SVN}\\\"\" ")
    endif()
endif() 

#
# Let's see whether we have git
#
find_package(Git)
if(Git_FOUND)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        execute_process(    COMMAND "${GIT_EXECUTABLE}" log -1 "--format=%H"
                            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                            OUTPUT_VARIABLE GIT_COMMIT )
        string(REGEX REPLACE "\n$" "" GIT_COMMIT "${GIT_COMMIT}")
        message("Current Git commit is ${GIT_COMMIT}")
        add_definitions(" -DCISTEM_GIT_COMMIT=\"\\\"${GIT_COMMIT}\\\"\" " )
    endif()
endif()

#
# Add the source directory 
#
add_subdirectory(src)
