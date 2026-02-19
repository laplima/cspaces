# SERVER
cmake_minimum_required(VERSION 3.30)

project(cspaces)   # name of the executable

set(sources
	${SKELETONS}
	${IDL_IMPL}
	ThreadPool.cpp
	TupleSpaceI.cpp
	server.cpp)

add_executable(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} ${CMAKE_PROJECT_NAME}_stubs)
target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_compile_options(${PROJECT_NAME} PUBLIC)
target_link_libraries(${PROJECT_NAME}
	${mylibs} spdlog::spdlog_header_only)
