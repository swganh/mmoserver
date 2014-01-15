include(MMOServerExecutable)

AddMMOServerExecutable(ChatServer
    MMOSERVER_DEPS 
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager        
)
