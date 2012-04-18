include(MMOServerExecutable)

AddMMOServerExecutable(ChatServer
    MMOSERVER_DEPS 
        Utils
        Common
        DatabaseManager
        NetworkManager        
)
