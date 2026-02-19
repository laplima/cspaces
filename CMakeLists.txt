# ACETAO CMake - MAIN
# (C) 2025 Luiz Lima Jr. All rights reserved.

cmake_minimum_required(VERSION 3.30)

project(cspaces)

set(IDL TupleSpace.idl)			# IDL files
set(IDL_DIR idl)				# IDL files location

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_BUILD_TYPE Debug)

set(SRC_DIR "src")
set(SUBPROJECTS
	cspaces
	csclient
	client
	)

find_package(spdlog)

# Generate ${mylibs}

set(libs
	ACE TAO
	TAO_PortableServer
	TAO_AnyTypeCode
	TAO_CosNaming
	TAO_IORTable
	colibry_static)

# Platform-dependent additional libraries
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(libs ${libs} )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(libs ${libs} )
endif()

set(mylibs)
foreach(lib_ ${libs})
	set(alib_ ${lib}-NOTFOUND)
	find_library(alib_ ${lib_} PATHS
		"$ENV{ACE_ROOT}/lib" "$ENV{COLIBRY}/lib" "/opt/local/lib")
	list(APPEND mylibs ${alib_})
endforeach(lib_)

# IDL ------------------------------------------

set(IDL_DIR "${PROJECT_SOURCE_DIR}/${IDL_DIR}")
# keep stubs in the build dir
set(STUBS_DIR "${CMAKE_CURRENT_BINARY_DIR}/stubs")
file(MAKE_DIRECTORY "${STUBS_DIR}")

set(IDL_IMPL)
set(IDL_OUTPUT)
set(SKELETONS)
set(STUBS)
foreach(idl ${IDL})
	get_filename_component(idl ${idl} NAME_WE)
	list(APPEND SKELETONS
		${STUBS_DIR}/${idl}S.cpp
		${STUBS_DIR}/${idl}C.cpp)
	list(APPEND STUBS
		${STUBS_DIR}/${idl}C.cpp)
	list(APPEND IDL_OUTPUT
		${STUBS_DIR}/${idl}C.h
		${STUBS_DIR}/${idl}C.inl
		${STUBS_DIR}/${idl}S.h)
	list(APPEND IDL_IMPL ${idl}I.cpp)
endforeach(idl)
list(APPEND IDL_OUTPUT ${SKELETONS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# tao_idl custom command

set(idls_)
foreach(idl ${IDL})
	list(APPEND idls_ ${IDL_DIR}/${idl})
endforeach(idl)

add_custom_command(
	OUTPUT ${IDL_OUTPUT}
	DEPENDS ${idls_}
	COMMAND tao_idl
	ARGS ${idls_} -o "${STUBS_DIR}"
	COMMENT "Generating stubs + skeletons"
	WORKING_DIRECTORY ${IDL_DIR}
	VERBATIM)

add_custom_target(${CMAKE_PROJECT_NAME}_stubs DEPENDS ${IDL_OUTPUT})

# INCLUDES ------------------------------------------

include_directories(
    "${STUBS_DIR}"
    "$ENV{ACE_ROOT}"
    "$ENV{TAO_ROOT}"
    "$ENV{TAO_ROOT}/orbsvcs"
    "$ENV{COLIBRY}/include")

# PLATFORM-DEPENDENT COMPILER OPTIONS

add_compile_options(-Wno-deprecated-declarations)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		# add_compile_options(-fdelayed-template-parsing -m64)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		include_directories("/opt/local/include")
endif()

# Subprojects
foreach(proj ${SUBPROJECTS})
	add_subdirectory("${SRC_DIR}/${proj}")
endforeach(proj)
