# Télécharger Raylib depuis GitHub pour l'affichage 2D
include(FetchContent)
# Pas besoin des exemples de Raylib, seulement de la bibliothèque
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Le code tiers de Raylib ne doit pas être bloqué par nos avertissements :
# on désactive temporairement "warnings as errors" le temps de le compiler.
# Notre propre code garde -Werror via cmake/compile_options.cmake.
set(EcoSimWarningAsErrorBackup ${CMAKE_COMPILE_WARNING_AS_ERROR})
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

set(CMAKE_COMPILE_WARNING_AS_ERROR ${EcoSimWarningAsErrorBackup})
