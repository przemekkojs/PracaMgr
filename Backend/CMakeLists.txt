# CMakeList.txt: projekt CMake dla elementu Backend, dołącz źródło i zdefiniuj
# tutaj logikę specyficzną dla projektu.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Backend")

# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (Backend "main.cpp"  "h/mainModule.h" "h/samplesModule.h" "h/synthModule.h" "h/modelModule.h" "h/module.h" "h/signal.h" "h/voices.h" "cpp/mainModule.cpp" "cpp/modelModule.cpp" "cpp/module.cpp" "cpp/samplesModule.cpp" "cpp/signal.cpp" "cpp/synthModule.cpp" "cpp/voices.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Backend PROPERTY CXX_STANDARD 20)
endif()

# TODO: Dodaj testy i zainstaluj elementy docelowe w razie potrzeby.
