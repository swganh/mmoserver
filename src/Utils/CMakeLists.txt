include(MMOServerLibrary)

AddMMOServerLibrary(Utils)
TARGET_LINK_LIBRARIES(Utils "-lrt -lpthread")