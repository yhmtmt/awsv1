add_library(n2k_ngt1 SHARED f_n2k_ngt1.cpp)

target_include_directories(n2k_ngt1 PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS n2k_ngt1 DESTINATION lib)
