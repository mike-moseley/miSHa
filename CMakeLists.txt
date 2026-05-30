cmake_minimum_required(VERSION 3.10)
project(c-project)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE sources
		"${CMAKE_SOURCE_DIR}/src/*.c"
)
add_executable(miSHa ${sources})
target_include_directories(miSHa PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_compile_options(miSHa PRIVATE -Wall -Wextra -g -pedantic)
target_compile_definitions(miSHa PRIVATE _POSIX_C_SOURCE=200809L)

# Unity testing
enable_testing()

set(UNITY_SRC "${CMAKE_SOURCE_DIR}/test/unity/unity.c")

file(GLOB_RECURSE test_sources
	"${CMAKE_SOURCE_DIR}/src/*.c"
)
list(FILTER test_sources EXCLUDE REGEX ".*/main\\.c$")

function(add_unity_test target source)
	add_executable(${target} ${source} ${UNITY_SRC} ${test_sources})
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/test"
	)
	target_include_directories(${target} PRIVATE
		"${CMAKE_SOURCE_DIR}/include"
		"${CMAKE_SOURCE_DIR}/test/unity"
	)
	target_compile_options(${target} PRIVATE -Wall -Wextra -g)
	add_test(NAME ${target} COMMAND ${target})
endfunction()

# Add test targets to this list
add_custom_target(tests DEPENDS
	test_input
	test_lexer
	test_parser
	test_executor
)

add_unity_test(test_input test/test_input.c)
add_unity_test(test_lexer test/test_lexer.c)
add_unity_test(test_parser test/test_parser.c)
add_unity_test(test_executor test/test_executor.c)
