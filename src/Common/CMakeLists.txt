include(MMOServerLibrary)

AddMMOServerLibrary(Common
    MMOSERVER_DEPS 
        Utils 
    ADDITIONAL_INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}
    ADDITIONAL_LIBRARY_DIRS
        ${Boost_LIBRARY_DIRS}
        ${TBB_LIBRARY_DIRS}
    DEBUG_LIBRARIES
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${TBB_DEBUG_LIBRARIES}
    OPTIMIZED_LIBRARIES
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${TBB_LIBRARIES}
)

