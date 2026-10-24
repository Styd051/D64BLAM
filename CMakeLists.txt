cmake_minimum_required(VERSION 3.10)
project(BLAM C)

include_directories(include)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(BLAM
	src/common.c
	src/macro.c
	src/main.c
	src/parse.c
	src/script.c
)

