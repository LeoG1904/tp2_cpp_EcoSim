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

# Découvrir les sources et relancer CMake lors de leur ajout ou suppression.
# Chaque couche est une cible distincte : une dépendance interdite ne compile pas.
function(ecosim_sources SOURCES_VAR MODULES_VAR)
    set(cpp_patterns "")
    set(module_patterns "")
    foreach(directory IN LISTS ARGN)
        list(APPEND cpp_patterns "${SRC_DIR}/${directory}/*.cpp")
        list(APPEND module_patterns "${SRC_DIR}/${directory}/*.ixx")
    endforeach()
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${cpp_patterns})
    file(GLOB_RECURSE modules CONFIGURE_DEPENDS ${module_patterns})
    set(${SOURCES_VAR} ${sources} PARENT_SCOPE)
    set(${MODULES_VAR} ${modules} PARENT_SCOPE)
endfunction()

ecosim_sources(CORE_SOURCES CORE_MODULES core world animals simulation)
ecosim_sources(UI_SOURCES UI_MODULES ui)
ecosim_sources(APP_SOURCES APP_MODULES render app)

# Moteur de simulation : utilisable sans affichage
add_library(ecosim_core STATIC ${CORE_SOURCES})
target_sources(ecosim_core PUBLIC FILE_SET CXX_MODULES FILES ${CORE_MODULES})
compile_options(ecosim_core)

# Logique d'interface en C++ pur : navigation, formulaire, animation, interfaces de rendu
add_library(ecosim_ui STATIC ${UI_SOURCES})
target_sources(ecosim_ui PUBLIC FILE_SET CXX_MODULES FILES ${UI_MODULES})
compile_options(ecosim_ui)
target_link_libraries(ecosim_ui ecosim_core)

# Exécutable principal : seule cible qui contient le code Windows
add_executable(${PROJECT_NAME} ${APP_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES ${APP_MODULES})
compile_options(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ecosim_ui user32 gdi32)

# Tests unitaires : ils n'embarquent pas le rendu Windows
if(ECOSIM_BUILD_TESTS)
    include(cmake/catch2.cmake)
    file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "${TESTS_DIR}/*.cpp")
    add_executable(tests ${TEST_FILES})
    target_link_libraries(tests PRIVATE ecosim_ui Catch2::Catch2WithMain)
    include(CTest)
    # Sous Windows, la découverte Catch2 corrompt les noms accentués lors du filtrage CTest.
    add_test(NAME EcoSimSuite COMMAND tests)
endif()
