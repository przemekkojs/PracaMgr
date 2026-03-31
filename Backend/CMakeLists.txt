cmake_minimum_required(VERSION 3.15)
project(Backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_PYTHON_VERSION 3.12)

set(Python3_ROOT_DIR "C:/Users/przem/AppData/Local/Programs/Python/Python312")
set(Python3_EXECUTABLE "C:/Users/przem/AppData/Local/Programs/Python/Python312/python.exe")

find_package(Python3 EXACT 3.12 REQUIRED COMPONENTS Interpreter Development)

add_subdirectory(lib/pybind11)

add_library(rtmidi STATIC
    lib/RtMidi.cpp
 "cpp/miniaudio_impl.cpp")

target_include_directories(rtmidi PUBLIC lib)
target_compile_definitions(rtmidi PUBLIC __WINDOWS_MM__)
target_link_libraries(rtmidi PUBLIC winmm)

add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE lib)

add_library(signalsmith-dsp INTERFACE)
target_include_directories(signalsmith-dsp INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(core
    cpp/mainModule.cpp
    cpp/modelModule.cpp
    cpp/module.cpp
    cpp/samplesModule.cpp
    cpp/signal.cpp
    cpp/synthModule.cpp
    cpp/voices.cpp
    cpp/miniaudio_impl.cpp
    cpp/metrices.cpp)

target_include_directories(core PUBLIC h)

target_link_libraries(core
    PUBLIC
    rtmidi
    miniaudio
)

add_executable(Backend main.cpp)

target_link_libraries(Backend PRIVATE core)

pybind11_add_module(organ
    bindings/bindings.cpp
)

target_link_libraries(organ
    PRIVATE
    core
    rtmidi
    Python3::Python
)

set_target_properties(organ PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python"
)

add_custom_command(TARGET organ POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${CMAKE_SOURCE_DIR}/python
        ${Python3_EXECUTABLE} -m pybind11_stubgen organ
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)

add_custom_command(TARGET organ POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/local/samples
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/local/samples
        ${CMAKE_BINARY_DIR}/local/samples
)