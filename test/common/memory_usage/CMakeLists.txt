project(memory_usage)

add_library(${PROJECT_NAME} STATIC memory_usage.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE .. ../.. ../../../include ../../..)

target_compile_options(${PROJECT_NAME} PRIVATE
  ${TBB_CXX_STD_FLAG}
  ${TBB_MMD_FLAG}
  ${TBB_DSE_FLAG}
  ${TBB_COMMON_COMPILE_FLAGS}
  ${TEST_LIB_COMPILE_FLAGS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:DEBUG>:TBB_USE_DEBUG>
  TBB_CYGWIN_USE_WIN32=1
  TBB_TEST_NO_DOCTEST_CONFIG=1
)

