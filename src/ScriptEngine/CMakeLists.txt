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
