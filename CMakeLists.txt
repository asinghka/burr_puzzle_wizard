cmake_minimum_required(VERSION 3.22)

project(burr_puzzle_wizard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(BURR_PUZZLE_WIZARD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(BURR_PUZZLE_WIZARD_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(BURR_PUZZLE_WIZARD_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/lib)

if(WINDOWS)
	set(BURR_PUZZLE_WIZARD_LIBS ${BURR_PUZZLE_WIZARD_LIBRARY_DIR}/glew32s.lib
			${BURR_PUZZLE_WIZARD_LIBRARY_DIR}/SDL2.lib
			${BURR_PUZZLE_WIZARD_LIBRARY_DIR}/SDL2main.lib
			opengl32.lib)
	include_directories(${CMAKE_SOURCE_DIR}/include_windows)
elseif(APPLE)
	find_package(OpenGL REQUIRED)
	find_package(SDL2 REQUIRED)
	set(BURR_PUZZLE_WIZARD_LIBS /Library/Frameworks/SDL2.framework/SDL2 OpenGL::GL /opt/homebrew/lib/libGLEW.dylib)
	include_directories(${SDL2_INCLUDE_DIRS})
	include_directories(/opt/homebrew/include)
endif()

file(GLOB_RECURSE IMGUI_CPP_FILES ${BURR_PUZZLE_WIZARD_INCLUDE_DIR}/imgui/*.c**)

file(GLOB_RECURSE BURR_PUZZLE_WIZARD_CPP_FILES ${BURR_PUZZLE_WIZARD_SOURCE_DIR}/*.c**)
file(GLOB_RECURSE BURR_PUZZLE_WIZARD_HPP_FILES ${BURR_PUZZLE_WIZARD_SOURCE_DIR}/*.h**)

include_directories(${BURR_PUZZLE_WIZARD_INCLUDE_DIR})

add_executable(burr_puzzle_wizard 
		${BURR_PUZZLE_WIZARD_CPP_FILES} 
		${BURR_PUZZLE_WIZARD_HPP_FILES}
		${IMGUI_CPP_FILES})

target_compile_definitions(burr_puzzle_wizard PRIVATE GLEW_STATIC)

target_link_libraries(burr_puzzle_wizard ${BURR_PUZZLE_WIZARD_LIBS})