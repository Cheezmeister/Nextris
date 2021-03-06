#CMake script for Spheres of Influence
#Run 'cmake <source_directory>' to generate a makefile
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.8)

#definitions
set(PROJECT_NAME "Nextris")
set(APP_NAME nextris)
set(PROJECT_VERSION 1 2 0)
set(DESCRIPTION "Tetris with a twist")
set(README README.txt)
set(LICENSE ${README})
set(AUTHORS "Brendan Luchen")
set(EXTRA_MODULES_IN "cmake/Modules")
set(CMAKE_BUILD_TYPE Debug)

# Allow emscripten for JavaScript port
option(USE_EMSCRIPTEN "Set this true if deploying to JS with emscripten" OFF)
if (USE_EMSCRIPTEN)
  set(APP_SUFFIX ".js")
endif()

#extract the proper versioning breakdown
list(GET PROJECT_VERSION 0 CPACK_PACKAGE_VERSION_MAJOR )
list(GET PROJECT_VERSION 1 CPACK_PACKAGE_VERSION_MINOR )
list(GET PROJECT_VERSION 2 CPACK_PACKAGE_VERSION_PATCH )
set(VERSION_STRING "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# Allow emscripten for JavaScript port
option(USE_EMSCRIPTEN "Set this true if deploying to JS with emscripten" OFF)
if (CMAKE_C_COMPILER MATCHES "emcc")
  set(USE_EMSCRIPTEN ON)
endif()
if (USE_EMSCRIPTEN)
  set(COMPILE_FLAGS  "-DEXPORTED_FUNCTIONS=\"['_js_setup', '_js_cleanup', '_js_update']\"")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2") # -O3 fails to link, this works somewhy :/
  set(APP_SUFFIX ".js")
else()
  set(COMPILE_FLAGS "-DNO_STDIO_REDIRECT")
endif()

set(WIN_DIR "${CMAKE_SOURCE_DIR}/win")
# set(EXTRA_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src")

#Tell CMake where to find extra scripts for this project
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/${EXTRA_MODULES_IN}")

#declare the project
project(${PROJECT_NAME})

#Find required tools (SDL)
find_package(SDL REQUIRED)
list(APPEND EXTRA_INCLUDE_DIRECTORIES ${SDL_INCLUDE_DIR})
if(NOT USE_EMSCRIPTEN)
  list(APPEND LINKED_LIBS ${SDL_LIBRARY})
endif()

#Find optional tools (PA)
find_package(PortAudio)
if (PORTAUDIO_FOUND)
   list(APPEND EXTRA_INCLUDE_DIRECTORIES ${PORTAUDIO_INCLUDE_DIR})
   list(APPEND LINKED_LIBS ${PORTAUDIO_LIBRARIES})
   add_definitions("-DHAVE_PORTAUDIO=1")
endif()

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


#grab source files
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)
list(APPEND SOURCES
  src/audio.cpp
  src/block.cpp
  src/debug.cpp
  src/display.cpp
  src/field.cpp
  src/options.cpp
  src/game.cpp
  src/score.cpp
  src/utils.cpp
  )
   list(APPEND SOURCES src/main.cpp)
if (NOT USE_EMSCRIPTEN)
   list(APPEND SOURCES CMakeLists.txt) #include this script for convenience
else()
   list(APPEND SOURCES src/jsbridge.c)
  add_definitions("-DEMSCRIPTEN=1")
endif()

#declare the target program
add_executable(${APP_NAME} ${SOURCES} ${EXTRA_SOURCES} ${HEADERS})


#add some useful preprocessor defines
set_property(
    TARGET ${APP_NAME} PROPERTY COMPILE_DEFINITIONS
    ${COMPILEDEFS}
    PROJECT_NAME="${PROJECT_NAME}"
    AUTHOR_NAMES="${AUTHORS}"
)

set_target_properties(${APP_NAME} PROPERTIES
  OUTPUT_NAME "${APP_NAME}-${VERSION_STRING}${APP_SUFFIX}"
)
#pass on the flags
set(CMAKE_CXX_FLAGS ${COMPILE_FLAGS} ${CMAKE_CXX_FLAGS} )
set(CMAKE_C_FLAGS   ${COMPILE_FLAGS} ${CMAKE_C_FLAGS} )

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




include(CPack)

