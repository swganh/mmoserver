ACLOCAL_AMFLAGS = -I tools/m4 --install
AM_CXXFLAGS     = -I$(top_srcdir)/src \
  $(SWGANH_CXXFLAGS) \
  $(BOOST_CPPFLAGS)

# Utils library - noinstall shared library
noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = src/Utils/bstring.cpp \
  src/Utils/clock.cpp \
  src/Utils/EventHandler.cpp \
  src/Utils/rand.cpp \
  src/Utils/Scheduler.cpp \
  src/Utils/StreamColors.cpp \
  src/Utils/Timer.cpp \
  src/Utils/utils.cpp \
  src/Utils/VariableTimeScheduler.cpp

libutils_la_CPPFLAGS = -fpermissive -fshort-wchar 
libutils_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# LogManager library - noinstall shared library
noinst_LTLIBRARIES += liblogmanager.la
liblogmanager_la_SOURCES = src/LogManager/Log.cpp \
  src/LogManager/LogManager.cpp
	
liblogmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 
liblogmanager_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# Common library - noinstall shared library
noinst_LTLIBRARIES += libcommon.la
libcommon_la_SOURCES = src/Common/atMacroString.cpp \
  src/Common/BuildInfo.cpp \
  src/Common/bytebuffer.cpp \
  src/Common/DispatchClient.cpp \
  src/Common/MessageDispatch.cpp \
  src/Common/MessageFactory.cpp 
    
libcommon_la_CPPFLAGS = -fpermissive -fshort-wchar 
libcommon_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# ConfigManager library - noinstall shared library
noinst_LTLIBRARIES += libconfigmanager.la
libconfigmanager_la_SOURCES = src/ConfigManager/ConfigFile.cpp \
  src/ConfigManager/ConfigManager.cpp
    
libconfigmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 

# Math library - noinstall shared library
noinst_LTLIBRARIES += libmath.la
libmath_la_SOURCES = src/MathLib/Quaternion.cpp \
  src/MathLib/Vector3.cpp

libmath_la_CPPFLAGS = -fpermissive -fshort-wchar 

# DatabaseManager library - noinstall shared library
noinst_LTLIBRARIES += libdatabasemanager.la
libdatabasemanager_la_SOURCES = src/DatabaseManager/Database.cpp \
  src/DatabaseManager/DatabaseImplementationMySql.cpp \
  src/DatabaseManager/DatabaseManager.cpp \
  src/DatabaseManager/DatabaseResult.cpp \
  src/DatabaseManager/DatabaseWorkerThread.cpp \
  src/DatabaseManager/DataBindingFactory.cpp \
  src/DatabaseManager/Transaction.cpp

libdatabasemanager_la_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar 
libdatabasemanager_la_LIBADD = libutils.la

# NetworkManager library - noinstall shared library
noinst_LTLIBRARIES += libnetworkmanager.la
libnetworkmanager_la_SOURCES = src/NetworkManager/CompCryptor.cpp \
  src/NetworkManager/NetConfig.cpp \
  src/NetworkManager/NetworkClient.cpp \
  src/NetworkManager/NetworkManager.cpp \
  src/NetworkManager/PacketFactory.cpp \
  src/NetworkManager/Service.cpp \
  src/NetworkManager/Session.cpp \
  src/NetworkManager/SessionFactory.cpp \
  src/NetworkManager/SocketReadThread.cpp \
  src/NetworkManager/SocketWriteThread.cpp

libnetworkmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 
libnetworkmanager_la_LIBADD = libutils.la libcommon.la liblogmanager.la

# MessageLib library - noinstall shared library
noinst_LTLIBRARIES += libmessagelib.la
libmessagelib_la_SOURCES = src/MessageLib/BuildingMessages.cpp \
	src/MessageLib/CommonMessages.cpp \
	src/MessageLib/CreatureMessages.cpp \
	src/MessageLib/HarvesterMessages.cpp \
	src/MessageLib/IntangibleMessages.cpp \
	src/MessageLib/InternalMessages.cpp \
	src/MessageLib/ManSchematicMessages.cpp \
	src/MessageLib/MessageLib.cpp \
	src/MessageLib/MissionMessages.cpp \
	src/MessageLib/ObjControllerMessages.cpp \
	src/MessageLib/PlayerMessages.cpp \
	src/MessageLib/ResourceContainerMessages.cpp \
	src/MessageLib/StaticMessages.cpp \
	src/MessageLib/StructureMessages.cpp \
	src/MessageLib/SystemMessages.cpp \
	src/MessageLib/TangibleMessages.cpp
	
libmessagelib_la_CPPFLAGS = -I$(top_srcdir)/deps/spatialindex/include -I$(top_srcdir)/deps/spatialindex/tools/include -I$(top_srcdir)/deps/noise/src -fpermissive -fshort-wchar 
libmessagelib_la_LIBADD = libutils.la libcommon.la liblogmanager.la

# ScriptEngine library - noinstall shared library
noinst_LTLIBRARIES += libscriptengine.la
libscriptengine_la_SOURCES = src/ScriptEngine/Script.cpp \
  src/ScriptEngine/ScriptEngine.cpp \
  src/ScriptEngine/ScriptEngineLib.cpp \
  src/ScriptEngine/ScriptEventListener.cpp \
  src/ScriptEngine/ScriptSupport.cpp \
  src/ScriptEngine/glue_files/LuaInterface.cpp

libscriptengine_la_CPPFLAGS = $(LUA_INCLUDE) -I$(top_srcdir)/deps/noise/src -fpermissive -fshort-wchar 
libscriptengine_la_LIBADD = $(LUA_LIB)

# AdminServer - executable
bin_PROGRAMS = adminserver
adminserver_SOURCES = src/AdminServer/AdminServer.cpp \
  src/AdminServer/CharacterAdminHandler.cpp \
  src/AdminServer/PlanetMapHandler.cpp

adminserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar 
adminserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# ChatServer - executable
bin_PROGRAMS += chatserver
chatserver_SOURCES = src/ChatServer/Channel.cpp \
  src/ChatServer/ChatAvatarId.cpp \
  src/ChatServer/ChatManager.cpp \
  src/ChatServer/ChatMessageLib.cpp \
  src/ChatServer/ChatMessages.cpp \
  src/ChatServer/ChatServer.cpp \
  src/ChatServer/CSRManager.cpp \
  src/ChatServer/CSRMessages.cpp \
  src/ChatServer/GroupManager.cpp \
  src/ChatServer/GroupMessages.cpp \
  src/ChatServer/GroupObject.cpp \
  src/ChatServer/Player.cpp \
  src/ChatServer/TradeManagerChat.cpp \
  src/ChatServer/TradeManagerHelp.cpp \
  src/ChatServer/TradeMessages.cpp

chatserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar -Wno-invalid-offsetof
chatserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# ConnectionServer - executable
bin_PROGRAMS += connserver
connserver_SOURCES = src/ConnectionServer/ClientManager.cpp \
	src/ConnectionServer/ConnectionDispatch.cpp \
	src/ConnectionServer/ConnectionServer.cpp \
	src/ConnectionServer/MessageRouter.cpp \
	src/ConnectionServer/ServerManager.cpp
	
connserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar
connserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# LoginServer - executable
bin_PROGRAMS += loginserver
loginserver_SOURCES = src/LoginServer/LoginManager.cpp \
	src/LoginServer/LoginServer.cpp
	
loginserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar -Wno-invalid-offsetof
loginserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# PingServer - executable
bin_PROGRAMS += pingserver
pingserver_SOURCES = src/PingServer/PingServer.cpp

pingserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive
pingserver_LDADD = liblogmanager.la libutils.la libconfigmanager.la libcommon.la libdatabasemanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# ZoneServer - executable
bin_PROGRAMS += zoneserver

zoneserver_SOURCES = src/ZoneServer/ActiveConversation.cpp \
	src/ZoneServer/AdminManager.cpp \
	src/ZoneServer/AttackableCreature.cpp \
	src/ZoneServer/AttackableStaticNpc.cpp \
	src/ZoneServer/BadgeRegion.cpp \
	src/ZoneServer/BadgeRegionFactory.cpp \
	src/ZoneServer/Bank.cpp \
	src/ZoneServer/BankTerminal.cpp \
	src/ZoneServer/BazaarTerminal.cpp \
	src/ZoneServer/Buff.cpp \
	src/ZoneServer/BuffDBItem.cpp \
	src/ZoneServer/BuffEvent.cpp \
	src/ZoneServer/BuffManager.cpp \
	src/ZoneServer/BuildingFactory.cpp \
	src/ZoneServer/BuildingObject.cpp \
	src/ZoneServer/Camp.cpp \
	src/ZoneServer/CampRegion.cpp \
	src/ZoneServer/CampTerminal.cpp \
	src/ZoneServer/CellFactory.cpp \
	src/ZoneServer/CellObject.cpp \
	src/ZoneServer/CharacterBuilderTerminal.cpp \
	src/ZoneServer/CharacterLoginHandler.cpp \
	src/ZoneServer/CharSheetManager.cpp \
	src/ZoneServer/City.cpp \
	src/ZoneServer/CityFactory.cpp \
	src/ZoneServer/CloningTerminal.cpp \
	src/ZoneServer/CombatManager.cpp \
	src/ZoneServer/Container.cpp \
	src/ZoneServer/ContainerObjectFactory.cp \
	src/ZoneServer/Conversation.cpp \
	src/ZoneServer/ConversationManager.cpp \
	src/ZoneServer/CraftBatch.cpp \
	src/ZoneServer/CraftingSession.cpp \
	src/ZoneServer/CraftingSessionFactory.cpp \
	src/ZoneServer/CraftingStation.cpp \
	src/ZoneServer/CraftingTool.cpp \
	src/ZoneServer/CreatureEventFunctions.cpp \
	src/ZoneServer/CreatureFactory.cpp \
	src/ZoneServer/CreatureObject.cpp \
	src/ZoneServer/CreatureSpawnRegion.cpp \
	src/ZoneServer/CurrentResource.cpp \
	src/ZoneServer/Datapad.cpp \
	src/ZoneServer/DatapadFactory.cpp \
	src/ZoneServer/Deed.cpp \
	src/ZoneServer/DraftSchematic.cpp \
	src/ZoneServer/DraftSlot.cpp \
	src/ZoneServer/DraftWeight.cpp \
	src/ZoneServer/ElevatorTerminal.cpp \
	src/ZoneServer/EnqueueValidator.cpp \
	src/ZoneServer/EntertainerManager.cpp \
	src/ZoneServer/EquipManager.cpp \
	src/ZoneServer/EVAbility.cpp \
	src/ZoneServer/EVCmdProperty.cpp \
	src/ZoneServer/EVPosture.cpp \
	src/ZoneServer/EVQueueSize.cpp \
	src/ZoneServer/EVState.cpp \
	src/ZoneServer/EVSurveySample.cpp \
	src/ZoneServer/EVWeapon.cpp \
	src/ZoneServer/FactoryBase.cpp \
	src/ZoneServer/FillerNPC.cpp \
	src/ZoneServer/Firework.cpp \
	src/ZoneServer/FireworkManager.cpp \
	src/ZoneServer/Food.cpp \
	src/ZoneServer/Furniture.cpp \
	src/ZoneServer/GroupManager.cpp \
	src/ZoneServer/GroupManagerHandler.cpp \
	src/ZoneServer/Ham.cpp \
	src/ZoneServer/HamProperty.cpp \
	src/ZoneServer/HarvesterFactory.cpp \
	src/ZoneServer/HarvesterObject.cpp \
	src/ZoneServer/Heightmap.cpp \
	src/ZoneServer/ImageDesignManager.cpp \
	src/ZoneServer/Instrument.cpp \
	src/ZoneServer/InsuranceTerminal.cpp \
	src/ZoneServer/IntangibleFactory.cpp \
	src/ZoneServer/IntangibleObject.cpp \
	src/ZoneServer/Inventory.cpp \
	src/ZoneServer/InventoryFactory.cpp \
	src/ZoneServer/Item.cpp \
	src/ZoneServer/ItemFactory.cpp \
	src/ZoneServer/LairObject.cpp \
	src/ZoneServer/ManufacturingSchematic.cpp \
	src/ZoneServer/Medicine.cpp \
	src/ZoneServer/MedicManager.cpp \
	src/ZoneServer/MissionBag.cpp \
	src/ZoneServer/MissionFactory.cpp \
	src/ZoneServer/MissionManager.cpp \
	src/ZoneServer/MissionObject.cpp \
	src/ZoneServer/MissionTerminal.cpp \
	src/ZoneServer/MovingObject.cpp \
	src/ZoneServer/noiseutils.cpp \
	src/ZoneServer/nonPersistantObjectFactory.cpp \
	src/ZoneServer/NonPersistentItemFactory.cpp \
	src/ZoneServer/NonPersistentNpcFactory.cpp \
	src/ZoneServer/NpcManager.cpp \
	src/ZoneServer/NPCObject.cpp \
	src/ZoneServer/ObjControllerCommandMessage.cpp \
	src/ZoneServer/ObjControllerEvent.cpp \
	src/ZoneServer/Object.cpp \
	src/ZoneServer/ObjectContainer.cpp \
	src/ZoneServer/ObjectController.cpp \
	src/ZoneServer/ObjectControllerCommandMap.cpp \
	src/ZoneServer/ObjectControllerDispatch.cpp \
	src/ZoneServer/ObjectFactory.cpp \
	src/ZoneServer/ObjectFactoryCallback.cpp \
	src/ZoneServer/OCAdminHandlers.cpp \
	src/ZoneServer/OCArtisanHandlers.cpp \
	src/ZoneServer/OCBioEngineerHandlers.cpp \
	src/ZoneServer/OCBountyHunterHandlers.cpp \
	src/ZoneServer/OCCallbackHandlers.cpp \
	src/ZoneServer/OCCarbineerHandlers.cpp \
	src/ZoneServer/OCCharacterSheethandlers.cpp \
	src/ZoneServer/OCCombatHandlers.cpp \
	src/ZoneServer/OCCombatMedicHandlers.cpp \
	src/ZoneServer/OCCommandoHandlers.cpp \
	src/ZoneServer/OCCommonHandlers.cpp \
	src/ZoneServer/OCContactListsHandlers.cpp \
	src/ZoneServer/OCCraftingHandlers.cpp \
	src/ZoneServer/OCCreatureHandlerHandlers.cpp \
	src/ZoneServer/OCDatapadHandlers.cpp \
	src/ZoneServer/OCDataTransformhandlers.cpp \
	src/ZoneServer/OCDestroyHandlers.cpp \
	src/ZoneServer/OCDoctorHandlers.cpp \
	src/ZoneServer/OCEntertainerHandlers.cpp \
	src/ZoneServer/OCFencerHandlers.cpp \
	src/ZoneServer/OCForceDefenseHandlers.cpp \
	src/ZoneServer/OCForceEnhancementHandlers.cpp \
	src/ZoneServer/OCForceHealingHandlers.cpp \
	src/ZoneServer/OCForceLightsaberHandlers.cpp \
	src/ZoneServer/OCForcePowerHandlers.cpp \
	src/ZoneServer/OCGroupHandlers.cpp \
	src/ZoneServer/OCMarksmanHandlers.cpp \
	src/ZoneServer/OCMedicHandlers.cpp \
	src/ZoneServer/OCMissionHandlers.cpp \
	src/ZoneServer/OCNpcConversationhandlers.cpp \
	src/ZoneServer/OCPetHandlers.cpp \
	src/ZoneServer/OCPikemanHandlers.cpp \
	src/ZoneServer/OCPistoleerHandlers.cpp \
	src/ZoneServer/OCPostureHandlers.cpp \
	src/ZoneServer/OCRangerHandlers.cpp \
	src/ZoneServer/OCResourceContainerHandlers.cpp \
	src/ZoneServer/OCRiflemanHandlers.cpp \
	src/ZoneServer/OCScoutHandlers.cpp \
	src/ZoneServer/OCSkillHandlers.cpp \
	src/ZoneServer/OCSmugglerHandlers.cpp \
	src/ZoneServer/OCSpatialHandlers.cpp \
	src/ZoneServer/OCSquadLeaderHandlers.cpp \
	src/ZoneServer/OCStructureHandlers.cpp \
	src/ZoneServer/OCSwordsmanHandlers.cpp \
	src/ZoneServer/OCTerasKasiHandlers.cpp \
	src/ZoneServer/OCTradeHandlers.cpp \
	src/ZoneServer/PersistentNpcFactory.cpp \
	src/ZoneServer/PlayerEventFunctions.cpp \
	src/ZoneServer/PlayerObject.cpp \
	src/ZoneServer/PlayerObjectFactory.cpp \
	src/ZoneServer/PlayerStructure.cpp \
	src/ZoneServer/ProcessValidator.cpp \
	src/ZoneServer/PVHam.cpp \
	src/ZoneServer/PVPosture.cpp \
	src/ZoneServer/PVState.cpp \
	src/ZoneServer/QTRegion.cpp \
	src/ZoneServer/QTRegionFactory.cpp \
	src/ZoneServer/QuadTree.cpp \
	src/ZoneServer/QuadTreeNode.cpp \
	src/ZoneServer/QuestGiver.cpp \
	src/ZoneServer/RadialMenu.cpp \
	src/ZoneServer/RadialMenuItem.cpp \
	src/ZoneServer/RegionFactory.cpp \
	src/ZoneServer/RegionObject.cpp \
	src/ZoneServer/Resource.cpp \
	src/ZoneServer/ResourceCategory.cpp \
	src/ZoneServer/ResourceCollectionCommand.cpp \
	src/ZoneServer/ResourceCollectionManager.cpp \
	src/ZoneServer/ResourceContainer.cpp \
	src/ZoneServer/ResourceContainerFactory.cpp \
	src/ZoneServer/ResourceManager.cpp \
	src/ZoneServer/ResourceType.cpp \
	src/ZoneServer/SchematicGroup.cpp \
	src/ZoneServer/SchematicManager.cpp \
	src/ZoneServer/Scout.cpp \
	src/ZoneServer/ScoutManager.cpp \
	src/ZoneServer/Shuttle.cpp \
	src/ZoneServer/ShuttleFactory.cpp \
	src/ZoneServer/Skill.cpp \
	src/ZoneServer/SkillManager.cpp \
	src/ZoneServer/SpawnRegion.cpp \
	src/ZoneServer/SpawnRegionFactory.cpp \
	src/ZoneServer/StaticObject.cpp \
	src/ZoneServer/Stomach.cpp \
	src/ZoneServer/StructureManager.cpp \
	src/ZoneServer/SurveyTool.cpp \
	src/ZoneServer/TangibleFactory.cpp \
	src/ZoneServer/TangibleObject.cpp \
	src/ZoneServer/Terminal.cpp \
	src/ZoneServer/TerminalFactory.cpp \
	src/ZoneServer/TicketCollector.cpp \
	src/ZoneServer/TicketCollectorFactory.cpp \
	src/ZoneServer/Trade.cpp \
	src/ZoneServer/TradeManager.cpp \
	src/ZoneServer/Trainer.cpp \
	src/ZoneServer/TravelMapHandler.cpp \
	src/ZoneServer/TravelTerminal.cpp \
	src/ZoneServer/TravelTicket.cpp \
	src/ZoneServer/TreasuryManager.cpp \
	src/ZoneServer/Tutorial.cpp \
	src/ZoneServer/UIButton.cpp \
	src/ZoneServer/UIDropdownBox.cpp \
	src/ZoneServer/UIElement.cpp \
	src/ZoneServer/UIInputBox.cpp \
	src/ZoneServer/UIInputField.cpp \
	src/ZoneServer/UIListBox.cpp \
	src/ZoneServer/UIManager.cpp \
	src/ZoneServer/UIMessageBox.cpp \
	src/ZoneServer/UITransferBox.cpp \
	src/ZoneServer/UIWindow.cpp \
	src/ZoneServer/Vehicle.cpp \
	src/ZoneServer/VehicleFactory.cpp \
	src/ZoneServer/WaypointFactory.cpp \
	src/ZoneServer/WaypointObject.cpp \
	src/ZoneServer/Weapon.cpp \
	src/ZoneServer/Wearable.cpp \
	src/ZoneServer/WeightsBatch.cpp \
	src/ZoneServer/WorldConfig.cpp \
	src/ZoneServer/WorldManager.cpp \
	src/ZoneServer/ZoneServer.cpp \
	src/ZoneServer/ZoneTree.cpp
	
zoneserver_CPPFLAGS = $(MYSQL_CFLAGS) -I$(top_srcdir)/deps/spatialindex/include -I$(top_srcdir)/deps/spatialindex/tools/include -I$(top_srcdir)/deps/noise/src -fpermissive -fshort-wchar -Wno-invalid-offsetof
zoneserver_LDADD = liblogmanager.la libutils.la libconfigmanager.la libcommon.la libmath.la libdatabasemanager.la libscriptengine.la libmessagelib.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# MMOServer tests
TESTS=mmoserver_tests
check_PROGRAMS = $(TESTS)
mmoserver_tests_SOURCES = test/main.cpp \
  test/Utils/TestCmpistr.cpp

mmoserver_tests_CPPFLAGS = $(GTEST_CPPFLAGS)
mmoserver_tests_LDADD = libutils.la \
  $(BOOST_LDFLAGS) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  $(GTEST_LIBS)

# Keep documentation files when creating a distributable version of the source.
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

# When creating a distributable version of the source we want the .svn directories 
# removed. This hook accomplishes that.
dist-hook:
	-rm -rf 'find $(distdir) -name .svn'
