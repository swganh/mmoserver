include(MMOServerLibrary)

AddMMOServerLibrary(libanh
    ADDITIONAL_SOURCE_DIRS
        event_dispatcher
		scripting
	ADDITIONAL_INCLUDE_DIRS
        ${PYTHON_INCLUDE_DIRS}
    DEBUG_LIBRARIES
		${Boost_PYTHON3_LIBRARY}
        ${PYTHON_LIBRARIES}
	OPTIMIZED_LIBRARIES
		${Boost_PYTHON3_LIBRARY}
        ${PYTHON_LIBRARIES}
)
