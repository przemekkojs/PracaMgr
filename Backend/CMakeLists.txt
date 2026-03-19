cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Backend")

add_executable (Backend "main.cpp"  "h/mainModule.h" "h/samplesModule.h" "h/synthModule.h" "h/modelModule.h" "h/module.h" "h/signal.h" "h/voices.h" "cpp/mainModule.cpp" "cpp/modelModule.cpp" "cpp/module.cpp" "cpp/samplesModule.cpp" "cpp/signal.cpp" "cpp/synthModule.cpp" "cpp/voices.cpp")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Backend PROPERTY CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_PYTHON_VERSION 3.12)
set(Python3_ROOT_DIR "C:/Users/przem/AppData/Local/Programs/Python/Python312")
set(Python3_EXECUTABLE "C:/Users/przem/AppData/Local/Programs/Python/Python312/python.exe")
find_package(Python3 EXACT 3.12 REQUIRED COMPONENTS Interpreter Development)

message("Python includes: ${Python3_INCLUDE_DIRS}")
message("Python libs:     ${Python3_LIBRARIES}")
message("Python exe:      ${Python3_EXECUTABLE}")

add_subdirectory(lib/pybind11)
find_package(pybind11 CONFIG REQUIRED)

add_library(rtmidi
    lib/RtMidi.cpp
)

target_include_directories(rtmidi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(rtmidi PUBLIC __WINDOWS_MM__)
target_link_libraries(rtmidi PUBLIC winmm)

pybind11_add_module(organ
    cpp/mainModule.cpp
)

target_link_libraries(organ PRIVATE rtmidi Python3::Python)

message("Python module output dir: ${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(organ PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python"
)

add_custom_command(TARGET organ POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${CMAKE_SOURCE_DIR}/python
        ${Python3_EXECUTABLE} -m pybind11_stubgen organ
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)
