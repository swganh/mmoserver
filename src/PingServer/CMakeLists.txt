include(MMOServerExecutable)

AddMMOServerExecutable(PingServer
    MMOSERVER_DEPS 
        NetworkManager   
        DatabaseManager        
        Utils
        Common
)
TARGET_LINK_LIBRARIES(PingServer "-lrt -lpthread")