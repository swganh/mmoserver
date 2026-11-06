include(MMOServerLibrary)

AddMMOServerLibrary(Utils)

# clock.cpp uses the Windows multimedia timer (timeGetTime, timeBeginPeriod).
if(WIN32)
    target_link_libraries(Utils PUBLIC winmm)
endif()
