include(MMOServerExecutable)

AddMMOServerExecutable(ZoneServer
    MMOSERVER_DEPS
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager
        MessageLib
        SwgProtocol
        ScriptEngine
    LIBRARIES
        mmo::lua
        mmo::tolua
        mmo::noise
)

# Visual Studio solutions: the zone F5 starts; change it in the project's
# Debugging properties.
set_target_properties(ZoneServer PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--ZoneName tutorial")
