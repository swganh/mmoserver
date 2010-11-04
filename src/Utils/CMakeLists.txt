# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)       
FILE(GLOB UTILS_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_unittest.cc)        

# Create the Utils library
ADD_LIBRARY(Utils STATIC ${SOURCES} ${HEADERS})

# Create an executable for the test and link it to gtest and anh
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})
ADD_EXECUTABLE(UtilsTests ${UTILS_TEST_SOURCES})
TARGET_LINK_LIBRARIES(UtilsTests
    Utils     
    ${GTEST_BOTH_LIBRARIES}
    debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
    debug ${Boost_SYSTEM_LIBRARY_DEBUG}
    debug ${Boost_THREAD_LIBRARY_DEBUG}
    debug ${TBB_LIBRARY_DEBUG}
    debug ${TBB_MALLOC_LIBRARY_DEBUG}
    optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
    optimized ${Boost_THREAD_LIBRARY_RELEASE}
    optimized ${TBB_LIBRARY}
    optimized ${TBB_MALLOC_LIBRARY})
GTEST_ADD_TESTS(UtilsTests "" ${UTILS_TEST_SOURCES})

# On Win32 tbb is a dll so we need to add its bin directory to the path so it
# can be loaded properly when the tests are run.
IF(WIN32)
    SET(LD_PATH "${TBB_INSTALL_DIR}/bin/$ENV{TBB_ARCH_PLATFORM};$ENV{PATH}")
    
    # IMPORTANT NOTE: The set_tests_properties(), below, internally
    # stores its name/value pairs with a semicolon delimiter.
    # because of this we must protect the semicolons in the path
    string(REPLACE ";" "\\;" LD_PATH "${LD_PATH}")
    
    # Add the dll path for each test
    FOREACH(test_name ${GTEST_ADD_TEST_NAMES})
        SET_TESTS_PROPERTIES(${test_name} PROPERTIES ENVIRONMENT "PATH=${LD_PATH}")
    ENDFOREACH()
ENDIF()
