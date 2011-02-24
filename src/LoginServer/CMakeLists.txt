include(MMOServerExecutable)

AddMMOServerExecutable(LoginServer
    MMOSERVER_DEPS 
        NetworkManager
        DatabaseManager
        Common
        Utils         
)
