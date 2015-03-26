include(MMOServerExecutable)

AddMMOServerExecutable(LoginServer
    MMOSERVER_DEPS 
        Utils
        Common
        DatabaseManager
        NetworkManager        
)
