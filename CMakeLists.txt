cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

option(BUILD_SAMPLES "Build samples" OFF)

project(slot_map)

add_library(spl_slotmap_headers INTERFACE)
add_library(spl::spl_slotmap_headers ALIAS spl_slotmap_headers)

install(TARGETS spl_slotmap_headers EXPORT spl_slotmap_headers-config)
install(EXPORT spl_slotmap_headers-config
	NAMESPACE spl::
	FILE spl_slotmap_headers-config.cmake
	DESTINATION share/spl_slotmap_headers
)

install(DIRECTORY include/
	DESTINATION include
)

target_include_directories(
	spl_slotmap_headers INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

if (${BUILD_SAMPLES})
	add_executable(test samples/test/main.cpp)
	set_target_properties(test PROPERTIES CXX_STANDARD 17)
	target_link_libraries(test spl::spl_slotmap_headers)
endif()