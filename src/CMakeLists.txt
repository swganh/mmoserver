option(DISABLE_LEGACY "disables legacy projects" FALSE)

# libraries

add_subdirectory(anh)

# executables

if (NOT DISABLE_LEGACY)
    ADD_SUBDIRECTORY(Utils)
    ADD_SUBDIRECTORY(Common)
    ADD_SUBDIRECTORY(NetworkManager)
    ADD_SUBDIRECTORY(DatabaseManager)
    ADD_SUBDIRECTORY(MessageLib)
    ADD_SUBDIRECTORY(SwgProtocol)
    ADD_SUBDIRECTORY(ScriptEngine)
    ADD_SUBDIRECTORY(ChatServer)
    ADD_SUBDIRECTORY(ConnectionServer)
    ADD_SUBDIRECTORY(LoginServer)
    ADD_SUBDIRECTORY(PingServer)
    ADD_SUBDIRECTORY(ZoneServer)
endif()