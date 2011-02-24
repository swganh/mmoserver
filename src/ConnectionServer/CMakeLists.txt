include(MMOServerExecutable)

AddMMOServerExecutable(ConnectionServer
    MMOSERVER_DEPS 
        NetworkManager
        DatabaseManager
        Common
        Utils   
)
