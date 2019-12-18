add_library(control_aws1 SHARED f_control_aws1.cpp)

target_include_directories(control_aws1 PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS control_aws1 DESTINATION lib)
