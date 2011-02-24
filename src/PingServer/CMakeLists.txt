include(MMOServerExecutable)

AddMMOServerExecutable(PingServer
    MMOSERVER_DEPS 
        NetworkManager
        DatabaseManager
        Common
        Utils   
)
