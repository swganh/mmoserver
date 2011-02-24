include(MMOServerExecutable)

AddMMOServerExecutable(ChatServer
    MMOSERVER_DEPS 
        NetworkManager
        DatabaseManager
        Common
        Utils        
)
