include(MMOServerExecutable)

AddMMOServerExecutable(ChatServer
    MMOSERVER_DEPS 
        DatabaseManager
        NetworkManager 
        Utils
        Common
       
)
