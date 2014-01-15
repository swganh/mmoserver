include(MMOServerExecutable)

AddMMOServerExecutable(LoginServer
    MMOSERVER_DEPS 
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager        
)
