include(MMOServerLibrary)

AddMMOServerLibrary(PythonEngine
    ADDITIONAL_INCLUDE_DIRS
        F:/Python26/include/
    DEBUG_LIBRARIES
		F:/MasterDeps/boost/boost_1_44/lib/boost_python-vc100-mt-gd-1_44.lib
        F:/Python26/libs/python26.lib
	OPTIMIZED_LIBRARIES
        F:/MasterDeps/boost/boost_1_44/lib/boost_python-vc100-mt-1_44.lib
        F:/Python26/libs/python26.lib
)

