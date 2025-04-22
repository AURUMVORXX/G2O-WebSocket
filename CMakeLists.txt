cmake_minimum_required(VERSION 3.21)

project(G2OWS VERSION 2.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(G2OWS_MODULE G2OWS)

add_library(${G2OWS_MODULE} SHARED)

# Defining suffix for output file
if(DEFINED OUT_FILE_SUFFIX)
	set_target_properties(${PROJECT_NAME}
			PROPERTIES 
				PREFIX ""
				SUFFIX ".${OUT_FILE_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
	)
endif()

# Dependencies and sources
add_subdirectory(dependencies)
add_subdirectory(source)