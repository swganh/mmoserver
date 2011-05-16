#   Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
#
#   The MySQL Connector/C++ is licensed under the terms of the GPL
#   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
#   MySQL Connectors. There are special exceptions to the terms and
#   conditions of the GPL as it is applied to this software, see the
#   FLOSS License Exception
#   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.

##########################################################################
MACRO(_MYSQL_CONFIG VAR _regex _opt)
		EXECUTE_PROCESS(COMMAND ${MYSQL_CONFIG_EXECUTABLE} ${_opt}
				OUTPUT_VARIABLE _mysql_config_output
				)
		SET(_var ${_mysql_config_output})
		STRING(REGEX MATCHALL "${_regex}([^ ]+)" _mysql_config_output "${_mysql_config_output}")
		STRING(REGEX REPLACE "^[ \t]+" "" _mysql_config_output "${_mysql_config_output}")
		STRING(REGEX REPLACE "[\r\n]$" "" _mysql_config_output "${_mysql_config_output}")
		STRING(REGEX REPLACE "${_regex}" "" _mysql_config_output "${_mysql_config_output}")
		SEPARATE_ARGUMENTS(_mysql_config_output)
		SET(${VAR} ${_mysql_config_output})
ENDMACRO(_MYSQL_CONFIG _regex _opt)


IF (NOT MYSQL_CONFIG_EXECUTABLE AND NOT WIN32)
	IF (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
		SET(MYSQL_CONFIG_EXECUTABLE "$ENV{MYSQL_DIR}/bin/mysql_config")
	ELSE (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
		FIND_PROGRAM(MYSQL_CONFIG_EXECUTABLE
			NAMES mysql_config
			DOC "full path of mysql_config"
			PATHS	/usr/bin
				/usr/local/bin
				/opt/mysql/mysql/bin
				/usr/local/mysql/bin
		)
	ENDIF (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
ENDIF (NOT MYSQL_CONFIG_EXECUTABLE AND NOT WIN32)

#-------------- FIND MYSQL_INCLUDE_DIR ------------------
SET(MYSQL_CXXFLAGS "")
IF(MYSQL_CONFIG_EXECUTABLE AND NOT WIN32)
	_MYSQL_CONFIG(MYSQL_INCLUDE_DIR "(^| )-I" "--include")
	MESSAGE(STATUS "mysql_config was found ${MYSQL_CONFIG_EXECUTABLE}")
	EXECUTE_PROCESS(COMMAND ${MYSQL_CONFIG_EXECUTABLE} "--cflags"
					OUTPUT_VARIABLE _mysql_config_output
					)
	STRING(REGEX MATCHALL "-m([^\r\n]+)" MYSQL_LINK_FLAGS "${_mysql_config_output}")
	STRING(REGEX REPLACE "[\r\n]$" "" MYSQL_CXXFLAGS "${_mysql_config_output}")
#	ADD_DEFINITIONS("${MYSQL_CXXFLAGS}")
ELSE (MYSQL_CONFIG_EXECUTABLE AND NOT WIN32)
	MESSAGE(STATUS "ENV{MYSQL_DIR} = $ENV{MYSQL_DIR}")
	FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
			$ENV{MYSQL_INCLUDE_DIR}
			$ENV{MYSQL_DIR}/include
			/usr/include/mysql
			/usr/local/include/mysql
			/opt/mysql/mysql/include
			/opt/mysql/mysql/include/mysql
			/usr/local/mysql/include
			/usr/local/mysql/include/mysql
			$ENV{ProgramFiles}/MySQL/*/include
			$ENV{SystemDrive}/MySQL/*/include)
ENDIF (MYSQL_CONFIG_EXECUTABLE AND NOT WIN32)
#----------------- FIND MYSQL_LIB_DIR -------------------


IF (WIN32)
	# Set lib path suffixes
	# dist = for mysql binary distributions
	# build = for custom built tree
	IF (CMAKE_BUILD_TYPE STREQUAL Debug)
		SET(libsuffixDist debug)
		SET(libsuffixBuild Debug)
	ELSE (CMAKE_BUILD_TYPE STREQUAL Debug)
		SET(libsuffixDist opt)
		SET(libsuffixBuild Release)
		ADD_DEFINITIONS(-DDBUG_OFF)
	ENDIF (CMAKE_BUILD_TYPE STREQUAL Debug)

	FIND_LIBRARY(MYSQL_LIB NAMES mysqlclient
				 PATHS
				 $ENV{MYSQL_DIR}/lib/${libsuffixDist}
				 $ENV{MYSQL_DIR}/lib/${libsuffixBuild}
				 $ENV{MYSQL_DIR}/lib 			#mysqlclient may be in lib for some c/c distros
				 $ENV{MYSQL_DIR}/libmysql/${libsuffixBuild}
				 $ENV{MYSQL_DIR}/client/${libsuffixBuild}
				 $ENV{ProgramFiles}/MySQL/*/lib/${libsuffixDist}
				 $ENV{ProgramFiles}/MySQL/*/lib
				 $ENV{SystemDrive}/MySQL/*/lib/${libsuffixDist}
				 $ENV{SystemDrive}/MySQL/*/lib)
	IF(MYSQL_LIB)
		SET(MYSQLCPPCONN_DYNLOAD_MYSQL_LIB MYSQL_LIB)
		GET_FILENAME_COMPONENT(MYSQL_LIB_DIR ${MYSQL_LIB} PATH)
	ENDIF(MYSQL_LIB)
	ADD_DEFINITIONS("-DDYNLOAD_MYSQL_LIB=\"${MYSQLCPPCONN_DYNLOAD_MYSQL_LIB}\"")
ELSE (WIN32)
	IF (MYSQL_CONFIG_EXECUTABLE)
		_MYSQL_CONFIG(MYSQL_LIBRARIES    "(^| )-l" "--libs_r")
		_MYSQL_CONFIG(MYSQL_LIB_DIR "(^| )-L" "--libs_r")

		EXECUTE_PROCESS(COMMAND ${MYSQL_CONFIG_EXECUTABLE} "--version"
						OUTPUT_VARIABLE __MYSQL_VERSION
						)
		# Test it
		#SET(__MYSQL_VERSION "4.0.10")
		# trim the string
		STRING(REGEX REPLACE "[\r\n]$" "" __MYSQL_VERSION "${__MYSQL_VERSION}")
		STRING(REGEX MATCHALL "(4.1.[0-9]+|[5-9].[0-9]+.[0-9]+)" MYSQL_VERSION "${__MYSQL_VERSION}")

		FIND_LIBRARY(MYSQLCPPCONN_DYNLOAD_MYSQL_LIB NAMES mysqlclient_r
					 PATHS
					 $ENV{MYSQL_DIR}/libmysql_r/.libs
					 $ENV{MYSQL_DIR}/lib
					 $ENV{MYSQL_DIR}/lib/mysql
					 /usr/lib/mysql
					 /usr/local/lib/mysql
					 /usr/local/mysql/lib
					 /usr/local/mysql/lib/mysql
					 /opt/mysql/mysql/lib
					 /opt/mysql/mysql/lib/mysql)

		ADD_DEFINITIONS("-DDYNLOAD_MYSQL_LIB=\"${MYSQLCPPCONN_DYNLOAD_MYSQL_LIB}\"")

	ELSE (MYSQL_CONFIG_EXECUTABLE)
		FIND_LIBRARY(MYSQL_LIB NAMES mysqlclient_r
					 PATHS
					 $ENV{MYSQL_DIR}/libmysql_r/.libs
					 $ENV{MYSQL_DIR}/lib
					 $ENV{MYSQL_DIR}/lib/mysql
					 /usr/lib/mysql
					 /usr/local/lib/mysql
					 /usr/local/mysql/lib
					 /usr/local/mysql/lib/mysql
					 /opt/mysql/mysql/lib
					 /opt/mysql/mysql/lib/mysql)
		SET(MYSQL_LIBRARIES mysqlclient_r )
		IF(MYSQL_LIB)
			GET_FILENAME_COMPONENT(MYSQL_LIB_DIR ${MYSQL_LIB} PATH)
			SET(MYSQLCPPCONN_DYNLOAD_MYSQL_LIB MYSQL_LIB)
		ENDIF(MYSQL_LIB)
	ENDIF (MYSQL_CONFIG_EXECUTABLE)
ENDIF (WIN32)



SET(VERBOSE 1)
IF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)

	MESSAGE(STATUS "MySQL Include dir: ${MYSQL_INCLUDE_DIR}")
	MESSAGE(STATUS "MySQL Library    : ${MYSQL_LIB}")
	MESSAGE(STATUS "MySQL Library dir: ${MYSQL_LIB_DIR}")
	MESSAGE(STATUS "MySQL CXXFLAGS: ${MYSQL_CXXFLAGS}")
	MESSAGE(STATUS "MySQL Link flags: ${MYSQL_LINK_FLAGS}")
	IF (MYSQL_VERSION)
		MESSAGE(STATUS "MySQL Version: ${MYSQL_VERSION}")
	ELSE (MYSQL_VERSION)
		IF(WIN32)
			#SET(CMAKE_REQUIRED_LIBRARIES ${MYSQL_LIB})
		ELSE(WIN32)
			# For now this works only on *nix
			SET(CMAKE_REQUIRED_LIBRARIES ${MYSQL_LIBRARIES})
			SET(CMAKE_REQUIRED_INCLUDES ${MYSQL_INCLUDE_DIR})
			CHECK_FUNCTION_EXISTS("mysql_set_character_set" HAVE_SET_CHARSET)
			SET(CMAKE_REQUIRED_LIBRARIES)
			SET(CMAKE_REQUIRED_INCLUDES)
			IF (HAVE_SET_CHARSET)
				MESSAGE(STATUS "libmysql version - ok")
			ELSE (HAVE_SET_CHARSET)
				MESSAGE(FATAL_ERROR "Versions < 4.1.13 (for MySQL 4.1.x) and < 5.0.7 for (MySQL 5.0.x) are not supported. Please update your libraries.")
			ENDIF (HAVE_SET_CHARSET)
		ENDIF(WIN32)
	ENDIF(MYSQL_VERSION)

	MESSAGE(STATUS "MySQL Include dir: ${MYSQL_INCLUDE_DIR}")
	MESSAGE(STATUS "MySQL Library dir: ${MYSQL_LIB_DIR}")
	MESSAGE(STATUS "MySQL CXXFLAGS: ${MYSQL_CXXFLAGS}")
	MESSAGE(STATUS "MySQL Link flags: ${MYSQL_LINK_FLAGS}")
	MESSAGE(STATUS "MySQL dynamic load test library: ${MYSQLCPPCONN_DYNLOAD_MYSQL_LIB}")
	#IF(MYSQLCPPCONN_DYNLOAD_MYSQL_LIB)
		#ADD_DEFINITIONS("-DDYNLOAD_MYSQL_LIB=\"${MYSQLCPPCONN_DYNLOAD_MYSQL_LIB}\"")
	#ENDIF(MYSQLCPPCONN_DYNLOAD_MYSQL_LIB)

	INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})
	LINK_DIRECTORIES(${MYSQL_LIB_DIR})

	SET(MYSQL_FOUND TRUE)
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)
	IF(NOT WIN32)
		MESSAGE(SEND_ERROR "mysql_config wasn't found, -DMYSQL_CONFIG_EXECUTABLE=...")
	ENDIF(NOT WIN32)
	MESSAGE(FATAL_ERROR "Cannot find MySQL. Include dir: ${MYSQL_INCLUDE_DIR}  library dir: ${MYSQL_LIB_DIR} cxxflags: ${MYSQL_CXXFLAGS}")
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)

