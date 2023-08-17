file(GLOB_RECURSE test_sources ${PROJECT_SOURCE_DIR}/tests/*.cpp)


foreach(test_source ${test_sources})
	get_filename_component(test_name ${test_source} NAME_WE)
	add_executable(${test_name} ${test_source})
	target_link_libraries( ${test_name} PRIVATE serverShared)
endforeach()