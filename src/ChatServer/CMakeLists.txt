include(MMOServerExecutable)


AddMMOServerExecutable(ChatServer
MMOSERVER_DEPS
DatabaseManager
NetworkManager
Utils
Common
)
TARGET_LINK_LIBRARIES(ChatServer "-lrt -lpthread")