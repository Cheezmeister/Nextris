#CMake script for Spheres of Influence
#Run 'cmake <source_directory>' to generate a makefile
cmake_minimum_required(VERSION 2.6)

#definitions
set(PROJECT_NAME "Nextris")
set(APP_NAME nextris)
set(PROJECT_VERSION 1 0 0)
set(DESCRIPTION "Tetris with a twist")
set(README README.txt)
set(LICENSE ${README})
set(AUTHORS "Brendan Luchen")
set(EXTRA_MODULES_IN "cmake/Modules")
set(TOOLS_NEEDED SDL)
set(CMAKE_BUILD_TYPE Debug)
set(COMPILE_FLAGS "-Wall")

set(WIN_DIR "${CMAKE_SOURCE_DIR}/win")
set(EXTRA_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src")

if(WIN32)
    file(GLOB EXTRA_SOURCES ${WIN_DIR}/*.c)
    list(APPEND DLLFILES ${WIN_DIR}/SDL.dll)
    if(MSVC)
        #for math.h
        set(COMPILEDEFS ${COMPILEDEFS} _USE_MATH_DEFINES) 
    endif(MSVC)
endif(WIN32)

if(UNIX)
if(${DEVKITPPC}) #for Wii port?
   set(CMAKE_SYSTEM_NAME Generic)
   set(CMAKE_FIND_ROOT_PATH "${DEVKITPPC}")
   set(CMAKE_CXX_COMPILER "${DEVKITPPC}/bin/powerpc-gekko/g++")
   set(CMAKE_C_COMPILER "${DEVKITPPC}/bin/powerpc-gekko/gcc")
endif(${DEVKITPPC})
endif(UNIX)

################################################################################
# Below be boilerplate!
################################################################################

#tell CMake where to find extra scripts for this project
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/${EXTRA_MODULES_IN}")

#declare the project
project(${PROJECT_NAME})

#grab source files
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c
                          ${CMAKE_SOURCE_DIR}/src/*.cpp
                          CMakeLists.txt) #include this script for convenience

#message("Found sources: " ${SOURCES} ${EXTRA_SOURCES} ${HEADERS})

#declare the target program
add_executable(${APP_NAME} ${SOURCES} ${EXTRA_SOURCES} ${HEADERS})

#add some useful preprocessor defines
add_definitions(${COMPILEDEFS})
set_property(
    TARGET ${APP_NAME} PROPERTY COMPILE_DEFINITIONS
    ${COMPILEDEFS}
    PROJECT_NAME="${PROJECT_NAME}"
    AUTHOR_NAMES="${AUTHORS}"
)

#pass on the flags
set(CMAKE_CXX_FLAGS ${COMPILE_FLAGS})
set(CMAKE_C_FLAGS   ${COMPILE_FLAGS})

#find required tools
foreach(TOOL ${TOOLS_NEEDED})
    find_package(${TOOL} REQUIRED)
    list(APPEND EXTRA_INCLUDE_DIRECTORIES ${${TOOL}_INCLUDE_DIR})
    list(APPEND LINKED_LIBS ${${TOOL}_LIBRARY})
    string(TOUPPER ${TOOL} TOOL) #most vars are set in all uppercase
    list(APPEND EXTRA_INCLUDE_DIRECTORIES ${${TOOL}_INCLUDE_DIR})
    list(APPEND LINKED_LIBS ${${TOOL}_LIBRARY})
endforeach(TOOL)

#find optional tools
find_package(PortAudio)
if (PORTAUDIO_FOUND)
   list(APPEND EXTRA_INCLUDE_DIRECTORIES ${PORTAUDIO_INCLUDE_DIR})
   list(APPEND LINKED_LIBS ${PORTAUDIO_LIBRARIES})
   add_definitions("-DHAVE_PORTAUDIO=1")
endif()

target_link_libraries(${APP_NAME} ${LINKED_LIBS})
include_directories(${EXTRA_INCLUDE_DIRECTORIES})

#install the binary to bin under the install directory
install(TARGETS ${APP_NAME}
    DESTINATION bin
)
install(FILES ${README} ${LICENSE} ${ADDL_INSTALL_FILES}
    DESTINATION .
)
install(FILES ${DLLFILES}
    DESTINATION bin
)






#extract the proper versioning breakdown
list(GET PROJECT_VERSION 0 CPACK_PACKAGE_VERSION_MAJOR )
list(GET PROJECT_VERSION 1 CPACK_PACKAGE_VERSION_MINOR )
list(GET PROJECT_VERSION 2 CPACK_PACKAGE_VERSION_PATCH )

include(CPack)

