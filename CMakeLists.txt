﻿# A semi-recent version of CMake is required for CPM
cmake_minimum_required(VERSION 3.15)

project(VTIL-Samples)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Lil' hacking never hurt anyone
#macro(file op)
#	if(op STREQUAL "GLOB" OR op STREQUAL "GLOB_RECURSE")
#		set(ARGS ${ARGN})
#		list(REMOVE_ITEM ARGS CONFIGURE_DEPENDS)
#		message("file(${ARGS})")
#		_file(${op} ${ARGS})
#	else()
#		_file(${op} ${ARGN})
#	endif()
#endmacro()

# https://github.com/TheLartians/CPM.cmake/releases
include(CPM.cmake)

# Add VTIL-Core dependency
CPMAddPackage(
	NAME VTIL-Core
	GITHUB_REPOSITORY vtil-project/VTIL-Core
	GIT_TAG ff94eef4483b07fbe970b7b80bbfae3b52524f16
)

# Example projects
add_subdirectory(Logger)
add_subdirectory(Intro)
add_subdirectory(Simplification)
add_subdirectory(Brainfuck)
