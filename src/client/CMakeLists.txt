# CLIENT
cmake_minimum_required(VERSION 3.30)

project(client)   # name of the executable

set(sources
	${STUBS}
	client.cpp)

add_executable(${PROJECT_NAME} ${sources})
add_dependencies(${PROJECT_NAME} ${CMAKE_PROJECT_NAME}_stubs)
target_compile_options(${PROJECT_NAME} PUBLIC)
target_link_libraries(${PROJECT_NAME}
	${mylibs} spdlog::spdlog_header_only)

# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${INSTALL_DIR}")
