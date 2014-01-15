include(MMOServerExecutable)

AddMMOServerExecutable(PingServer
    MMOSERVER_DEPS 
	libanh
        Utils
        Common
        DatabaseManager
        NetworkManager
)
