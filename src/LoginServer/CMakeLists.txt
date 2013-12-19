include(MMOServerExecutable)

AddMMOServerExecutable(LoginServer
    MMOSERVER_DEPS 
        DatabaseManager
        NetworkManager        
        Utils
        Common
)
