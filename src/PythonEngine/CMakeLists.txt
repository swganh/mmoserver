include(MMOServerLibrary)

AddMMOServerLibrary(PythonEngine
    ADDITIONAL_INCLUDE_DIRS
        ${PYTHON_INCLUDE_DIRS}
    DEBUG_LIBRARIES
		#${Boost_PYTHON3_LIBRARY}
		F:/SWGANH/dev/mmoserver/deps/src/boost/bin.v2/libs/python/build/msvc-10.0/debug/threading-multi/boost_python3-vc100-mt-gd-1_45.lib
		#F:/MasterDeps/boost/boost_1_44/lib/boost_python-vc100-mt-gd-1_44.lib
        ${PYTHON_LIBRARIES}
	OPTIMIZED_LIBRARIES
		#${Boost_PYTHON3_LIBRARY}
		F:/SWGANH/dev/mmoserver/deps/src/boost/bin.v2/libs/python/build/msvc-10.0/debug/threading-multi/boost_python3-vc100-mt-gd-1_45.lib
        #F:/MasterDeps/boost/boost_1_44/lib/boost_python-vc100-mt-1_44.lib
        ${PYTHON_LIBRARIES}
)

