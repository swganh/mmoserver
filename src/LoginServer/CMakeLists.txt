include(MMOServerExecutable)

AddMMOServerExecutable(LoginServer
    MMOSERVER_DEPS 
        DatabaseManager    
        NetworkManager           
        Utils
        Common
)
TARGET_LINK_LIBRARIES(LoginServer "-lrt -lpthread")