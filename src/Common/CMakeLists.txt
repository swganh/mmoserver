# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)          
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)          
FILE(GLOB COMMON_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_unittest.cc)
LIST(REMOVE_ITEM SOURCES ${COMMON_TEST_SOURCES}) # Remove the unit tests from the sources list.        

# Create the Common library
ADD_LIBRARY(Common STATIC ${SOURCES} ${HEADERS})                       
TARGET_LINK_LIBRARIES(Common 
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${TBB_LIBRARIES}
    Utils
)

# Enable testing and add gtest to the include directories.
ENABLE_TESTING()
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

# Create an executable for the test and link it to gtest and anh
ADD_EXECUTABLE(CommonTests ${COMMON_TEST_SOURCES})
TARGET_LINK_LIBRARIES(CommonTests ${GTEST_BOTH_LIBRARIES} Utils Common)
GTEST_ADD_TESTS(CommonTests "" ${COMMON_TEST_SOURCES})

# Some automated systems need an xml report which requires a second run of the
# tests. To prevent these unnecessary runnings during normal builds the
# following option is set with a default of false.
OPTION(ENABLE_TEST_REPORT 
    "Enables the generation of a test report when running tests" FALSE)
IF(ENABLE_TEST_REPORT)
    ADD_TEST(AllTests CommonTests "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/")
ENDIF()
    
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