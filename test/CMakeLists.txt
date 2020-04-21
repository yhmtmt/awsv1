configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
  ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
  EXCLUDE_FROM_ALL)

if(CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Test example setting
add_executable(example example.cpp)
target_link_libraries(example gtest_main)
add_test(NAME example_test COMMAND example WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Test clock setting
add_executable(test_clock test_clock.cpp ${PROJECT_SOURCE_DIR}/src/aws_clock.cpp)
target_link_libraries(test_clock gtest_main)
target_include_directories(test_clock PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_clock COMMAND test_clock WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Test coord setting
add_executable(test_coord test_coord.cpp ${PROJECT_SOURCE_DIR}/src/aws_coord.cpp)
target_link_libraries(test_coord gtest_main proj)
target_include_directories(test_coord PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_coord COMMAND test_coord WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Test nmea0183 decoder
add_executable(test_nmea test_nmea.cpp ${PROJECT_SOURCE_DIR}/src/aws_nmea.cpp ${PROJECT_SOURCE_DIR}/src/aws_nmea_gps.cpp ${PROJECT_SOURCE_DIR}/src/aws_nmea_ais.cpp)
target_link_libraries(test_nmea gtest_main proj)
target_include_directories(test_nmea PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_nmea COMMAND test_nmea WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


install(DIRECTORY DESTINATION ftest)
file(GLOB FTESTS ftest/*)
install(FILES ${FTESTS}
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE  
  DESTINATION ftest)

