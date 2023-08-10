file(GLOB_RECURSE serverSources ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(serverShared SHARED ${serverSources} "includes/common.h")

