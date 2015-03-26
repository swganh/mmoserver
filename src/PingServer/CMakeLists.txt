include(MMOServerExecutable)

AddMMOServerExecutable(PingServer
    MMOSERVER_DEPS 
        Utils
        Common
        DatabaseManager
        NetworkManager
)
