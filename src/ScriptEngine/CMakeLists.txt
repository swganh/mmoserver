include(MMOServerLibrary)

AddMMOServerLibrary(ScriptEngine
    MMOSERVER_DEPS
        Utils
        Common
    LIBRARIES
        mmo::lua
        mmo::tolua
        mmo::noise
)

# tutorial_progress_unittest.cc loads data/script from the source tree.
if(TARGET ScriptEngine_tests)
    target_compile_definitions(ScriptEngine_tests PRIVATE MMOSERVER_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
endif()
