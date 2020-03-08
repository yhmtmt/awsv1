#Filter message's flatbuffer definition
file(GLOB SAMPLE_FLATS *.fbs)
set(SAMPLE_FLAT_SRC_DIR ./)


flatc_generate_cpp(SAMPLE_FLAT_HDRS SAMPLE_FLAT_BFBS ${CMAKE_CURRENT_SOURCE_DIR}  sample_msg.fbs)
message("flatbuffer description file: ${SAMPLE_FLAT_HDRS}")
install(FILES ${SAMPLE_FLAT_BFBS}
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  DESTINATION lib)

add_library(sample SHARED f_sample.cpp ${SAMPLE_FLAT_HDRS})


target_include_directories(sample PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS sample DESTINATION lib)

file(GLOB TESTS test/*)
install(FILES ${TESTS}
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  DESTINATION ftest)

