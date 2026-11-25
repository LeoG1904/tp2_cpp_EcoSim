# Configuration de CMake pour TP C++ Moderne esimed
cmake_minimum_required(VERSION 3.29)

# Nom du projet et de l'exécutable généré
project(EcoSim)

if(NOT WIN32)
    message(FATAL_ERROR "EcoSim requires Windows for its Win32/GDI renderer")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
# Forcer l'utilisation de C++ 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Tout doit être parfait
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
# Ne pas utiliser les extensions de CMake
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
endif()

# Pour "import std"
include(cmake/std.cmake)
include(cmake/compile_options.cmake)
option(ECOSIM_BUILD_TESTS "Build Catch2 tests (downloads Catch2)" OFF)

# Répertoire contenant les sources des exercices
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Répertoire contenant les sources des tests unitaires
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Découvrir les sources et relancer CMake lors de leur ajout ou suppression
# Note : la recherche est récursive, les sous-répertoires sont pris en compte
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${SRC_DIR}/*.cpp"
)
file(GLOB_RECURSE MODULE_FILES CONFIGURE_DEPENDS
        "${SRC_DIR}/*.ixx"
)
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS
        "${TESTS_DIR}/*.cpp"
)

# Les tests utilisent le main fourni par Catch2
set(TEST_SRC_FILES ${SRC_FILES} ${TEST_FILES})
list(FILTER TEST_SRC_FILES EXCLUDE REGEX "(^|[/\\\\])main\\.cpp$")

# Exécutable principal
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        ${MODULE_FILES}
)
compile_options(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} user32 gdi32)

# Exécutable et liste des fichiers sources des tests unitaires
if(ECOSIM_BUILD_TESTS)
    include(cmake/catch2.cmake)
    add_executable(tests ${TEST_SRC_FILES})
    target_sources(tests PUBLIC FILE_SET CXX_MODULES FILES ${MODULE_FILES})
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain std-cxx-modules)
    target_link_libraries(tests PRIVATE user32 gdi32)
    include(CTest)
    # Sous Windows, la découverte Catch2 corrompt les noms accentués lors du filtrage CTest.
    add_test(NAME EcoSimSuite COMMAND tests)
endif()

