# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES} ${GLOG_INCLUDE_DIR})

# Create the Utils library
ADD_EXECUTABLE(ConnectionServer ${SOURCES} ${HEADERS})         
TARGET_LINK_LIBRARIES(ConnectionServer
    NetworkManager
    DatabaseManager
    Common
    Utils
    ${MYSQL_LIB}
    debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
    debug ${Boost_SYSTEM_LIBRARY_DEBUG}
    debug ${Boost_THREAD_LIBRARY_DEBUG}
    debug ${GLOG_LIBRARY_DEBUG}
    debug ${MysqlConnectorCpp_LIBRARY_DEBUG}
    debug ${TBB_LIBRARY_DEBUG}
    debug ${TBB_MALLOC_LIBRARY_DEBUG}
    debug ${ZLIB_LIBRARY_DEBUG}
    optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
    optimized ${Boost_THREAD_LIBRARY_RELEASE}
    optimized ${GLOG_LIBRARY_RELEASE}
    optimized ${MysqlConnectorCpp_LIBRARY_RELEASE}
    optimized ${TBB_LIBRARY}
    optimized ${TBB_MALLOC_LIBRARY}
    optimized ${ZLIB_LIBRARY_RELEASE}
)

IF(WIN32)
    SET_TARGET_PROPERTIES(ConnectionServer PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
	TARGET_LINK_LIBRARIES(ConnectionServer "winmm.lib" "ws2_32.lib")
	
    ADD_CUSTOM_COMMAND(TARGET ConnectionServer POST_BUILD
        COMMAND call \"${PROJECT_SOURCE_DIR}/tools/windows/postbuild.bat\" \"${PROJECT_SOURCE_DIR}\" \"${PROJECT_BINARY_DIR}\" \"\$\(ConfigurationName\)\"
    )
        
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tools/windows/user_project.vcxproj.in 
	    ${CMAKE_CURRENT_BINARY_DIR}/ConnectionServer.vcxproj.user @ONLY)
ELSE()
    INSTALL(TARGETS ConnectionServer RUNTIME DESTINATION bin)
ENDIF()
