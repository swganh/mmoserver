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
