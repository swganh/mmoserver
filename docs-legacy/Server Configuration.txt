Server Configuration is roughly divided into the Networklayer Configuration and the Zone configuration.

This is important, as ALL servers have a networklayer, but not all servers are zones.
Additionally we have no way (and plainly dont need to) to access the db from the networklayer.

Thus the networklayer will have its initialization in the configuration files (Adminserver.cfg, tatooine.cfg) whereas the world configuration will happen through the db.

Networklayer Configuration

Networkconfiguration will be administered through Netconfig.h
it reads token out the config file and offers them to the networklayer on demand.

mReliableSizeServerServer		= gConfig->read<int>("ReliablePacketSizeServerServer",2000);

ReliablePacketSizeServerServer is the token in the config file, 2000 is an initializationvalue used in case the token cannot be found.


ReliablePacketSizeServerServer used in the socket / packet / session layer to determine the size of reliable packets in server server ( zone connection) communication. Reliables are the biggest possible packet and thus used to assert maxpacketsize

ReliablePacketSizeServerZone used in the socket / packet / session layer to determine the size of reliable packets in server zone ( connection client) communication. Reliables are the biggest possible packet and thus used to assert maxpacketsize

UnReliablePacketSizeServerServer used in the socket / packet / session layer to determine the size of unreliable packets in server server ( zone connection) communication. unreliables are the fastpath packets which have no storing for poss resends

UnReliablePacketSizeServerZone used in the socket / packet / session layer to determine the size of reliable packets in server zone ( connection client) communication. unreliables are the fastpath packets which have no storing for poss resends

mServerPacketWindow is the max size of the packetwindow (size of the packetqueue containing both send and unsend packets) for server server communication. If the packetqueue is full no new packets will be generated. Packets are removed from the packetqueue once they have been acknowledged. Unreliables are not part of that queue

mClientPacketWindow is the max size of the packetwindow (size of the packetqueue containing both send and unsend packets) for server client communication. If the packetqueue is full no new packets will be generated. Packets are removed from the packetqueue once they have been acknowledged. Unreliables are not part of that queue

ClusterBindAdress is the IP of the networkadapter used for the connectionserver to connect to the zone / admin / chat servers
In case you have zoneservers on different machines in the internet it is an outward IP, when all servers are on one single machine its the IP of this machine in the homenetwork

ClusterBindPort is the port the connectionserver uses to establish communication with the other servers
we need to give it explicitely as it is looked up by other servers in the db to establish connection

BindAdress is the IP of the connectionserver with which it connects to the clients - so its the outwards facing IP
For all other servers it is the IP of the machine they reside on. The reason is that we are not able to detect our adapters IP on multihomed hosts like Vista and the server needs to update its IP on the db

BindPort is the port of the connectionserver with which to communicate with its clients for all other servers it is the IP with which the server communicates with the connectionserver


configserver.cfg of the testcenter

# Networking Configuration
BindAddress=64.7.136.227 		this is the outward facing IP it will be accessed by the clients over the internet
BindPort=44991 				this is the outwardfacing port it will be accessed by the clients over the internet
ClusterBindAddress=192.168.100.83 	this is the homenetworkadress of the machine the tc resides on - in case other 					servers are on the internet it neds to be an outwardfacing networkadress, too

ClusterBindPort=5000 			thats the port we write in the db for the other servers to connect to us

bool logtext = gConfig->read<bool>("Zone_Error_Log_Text",(bool)true);  -> Zone_Admin_Log_Text etc
Zone_Error_Log_Text is a bool which determines whether  a specific log (the error log) is printed to screen it will always be printed to the db
Zone_Admin_Log_Text is a bool which determines whether  a specific log (the admin log) is printed to screen it will always be printed to the db
Zone_transaction_Log_Text is a bool which determines whether  a specific log (the transaction log) is printed to screen it will always be printed to the db

gLogger->createErrorLog("AdminServer",(LogLevel)(gConfig->read<int>("ErrorLogLevel",2)),
		(bool)(gConfig->read<std::string>("ErrorLogToFile", true)),
		(bool)(gConfig->read<bool>("ErrorConsoleOut",true)),
		(bool)(gConfig->read<bool>("ErrorLogAppend",true)));

(Error)logtofile determines whether the log will be created as a file
(Error)ConsoleOut determines whether the log will be printed on the console
(Error)logAppend determines whether the log (if printed to file) will be appended
other logging systems planned are (Admin) (Transaction)




 (char*)(gConfig->read<std::string>("DBLogServer")).c_str(),									   		gConfig->read<int>("DBLogPort"),
		(char*)(gConfig->read<std::string>("DBLogUser")).c_str(),
		(char*)(gConfig->read<std::string>("DBLogPass")).c_str(),
		(char*)(gConfig->read<std::string>("DBLogName")).c_str());

the same as the db data for the server only that we are able to select a separate db for logging
it will default to server values if no db is found




recommended IP settings for all servers for a developer tc on one machine is 127.0.0.1 to be set in all config files. In the galaxy table you should enter the real IP of the machine. Additionally you need to supply a unique port for every server - 5000 to 6000 seems like a good range

WorldConfiguration is handled over WorldConfig

worldconfig loads its attributes from config_server with the attributes names tored in config_server_attributes. all attributes have a server name attached to it. The worldconfig loads the attributes attached to 'all' first. Afterwards it will load those attached to its planetname. Thus a planet specific setting will overwrite a global configuration.
Global Configuration 'all' means that we dont have to add global changes separately for every server as no server specific attribute means the global one applies.

these attributes are either saved into worldconfigspecific variables and can be retrieved by float			viewingRange = (float)gWorldConfig->getPlayerViewingRange();
this is true for 

uint16				getPlayerViewingRange(){ return mPlayerViewingRange; }
uint16				getPlayerChatRange(){ return mPlayerChatRange; }
uint32				getServerTimeInterval(){ return mServerTimeInterval; }
uint32				getServerTimeSpeed(){ return mServerTimeSpeed; }
uint8				getPlayerMaxIncaps(){ return mPlayerMaxIncaps; }
uint32				getBaseIncapTime(){ return mPlayerBaseIncapTime; }
uint32				getIncapResetTime(){ return mIncapResetTime; }			
float				mHealthRegenDivider,mActionRegenDivider,mMindRegenDivider;
these above can also be accessed as attributes


those down do not get their settings from the db (yet) and cannot be accessed as attributes
bool				isTutorial() { return (mTutorialEnabled && (mZoneId == 41)); }
void				enableTutorial() { mTutorialEnabled = true; }
void				disableTutorial() { mTutorialEnabled = false; }
void				enableInstance() { mInstanceEnabled = true; }


other settings can only be gotten through the attribute way

int8 mot[255] = "welcome to swgAnh";
string motD = gWorldConfig->getConfiguration("motD",mot);

or

if(price > gWorldConfig->getConfiguration("Server_Bazaar_MaxPrice",20000))

the first string is the name of the attribute UPPER LOWER CASE MATTERS!!!!!! the second is an optional (but probably rather useful) initialization value which is returned in case the attribute cannot be found!!!!!

mReliableSizeServerServer		= gConfig->read<int>("ReliablePacketSizeServerServer",1000);
mUnReliableSizeServerServer	= gConfig->read<int>("UnReliablePacketSizeServerServer",500);

mReliableSizeServerClient		= gConfig->read<int>("ReliablePacketSizeServerClient",495);
mUnReliableSizeServerClient	= gConfig->read<int>("UnReliablePacketSizeServerClient",495);

mServerPacketWindow			= gConfig->read<int>("ServerPacketWindowSize",800);
mClientPacketWindow			= gConfig->read<int>("ClientPacketWindowSize",8);


Packetsize should NOT exceed 1450 to prevent fragmenting and deformed packets. the client requires in standar 495 size packets, though that can be altered. However the packetsize of 495 is chosen as to have the packets as reliable as possible in internet communication. Bigger packetsizes will only make sense for the server server communication


==================================================
==================================================

Attributes List

Zone_Player_NalargonUse is the range we allow a player to use a nalargon default (if attribute is not set) is 6 m

======================

Group_MissionUpdate_Time gives the time intervall a normal player triggers a groupwaypoint update when moving

mGroupMissionUpdateTime = gWorldConfig->getConfiguration("Group_MissionUpdate_Time",30000);
	if(mGroupMissionUpdateTime < 1000 || mGroupMissionUpdateTime > 60000)
	{
		mGroupMissionUpdateTime = 30000;

======================

added 19.8.09
this gives the maximum amount missions, waypoints and datapad items.
Please note that the maximum is 255 and 6 for missions
that is because the missionsystem uses a bitmask to identify missions in the bag
below you see the respective values set if the variable isnt initialized
mWayPointCapacity = gWorldConfig->getConfiguration("Player_DataPad_WayPointCapacity",(uint8)100);
mCapacity = gWorldConfig->getConfiguration("Player_Datapad_Capacity",(uint8)100);
mMissionCapacity = gWorldConfig->getConfiguration("Player_Datapad_MissionCapacity",(uint8)2);

==================================================

added 30/9/09
Player_Max_Lots = gWorldConfig->getConfiguration("Player_Max_Lots",(uint8)10);
gives the maximum amount of lots for a player - should be the same over all zones ...

==================================================

added 18/10/09
time = gWorldConfig->getConfiguration("Zone_BuildingFenceInterval",(uint32)10000);

gives the ms interval the building fence is in the world until the harvester gets spawned

==================================================

float fAdminListDistance = gWorldConfig->getConfiguration("Player_Admin_List_Distance",(float)32.0);

Player_Admin_List_Distance is the distance someone is allowed to manipulate the adminlist of a structure of

==================================================

uint32 structureCheckIntervall = gWorldConfig->getConfiguration("Zone_structureCheckIntervall",(uint32)3600);
this is the intervall in seconds in which structures like harvesters will be checked for condition or theire activated (harv, factory) status

==================================================

uint32 logout = gWorldConfig->getConfiguration("Player_LogOut_Time",(uint32)30);
is the time in seconds required for a proper logout on /logout In which the player will have to remain in the world.
at least (logoutSpacer) seconds, max 300 seconds; every (Player_LogOut_Spacer) seconds the Player will notified of the remaining time

uint32 logoutspacer = gWorldConfig->getConfiguration("Player_LogOut_Spacer",(uint32)30);
if(logoutSpacer > logout)
logoutSpacer must at least be 1;

==================================================

uint32 timeOut = gWorldConfig->getConfiguration("Zone_Player_Logout",300);
gives the time the zone retains the playerobject in memory once the player has logged out. After this time the player gets saved and removed from memory

==================================================

this is the hamcost associated with burstrun

uint16 actioncost = gWorldConfig->getConfiguration("Player_BurstRun_Action",(uint16)300);
uint16 healthcost = gWorldConfig->getConfiguration("Player_BurstRun_Health",(uint16)300);
uint16 mindcost	  = gWorldConfig->getConfiguration("Player_BurstRun_Mind",(uint16)0);

==================================================

these are the burstrun timers
uint32 br_length		= gWorldConfig->getConfiguration("Player_BurstRun_Time",(uint32)60);
uint32 br_coolD			= gWorldConfig->getConfiguration("Player_BurstRun_CoolDown",(uint32)600);

==================================================

float distance = gWorldConfig->getConfiguration("Player_heal_distance",(float)6.0);
thats the distance a medic might have to still be able to heal a target

==================================================
thats te distance after which a standard UI will close if you move away from the associated object
distance = gWorldConfig->getConfiguration("Player_UI_Closure",(float)30.0);


==================================================
thats the distance you need to access a nearby ticketTerminal
distance = gWorldConfig->getConfiguration("Player_TicketTerminalAccess_Distance",(float)10.0);


==================================================
the amount of money necessary to insure your items
mMoney			= gConfig->read<int>("Player_ItemInsuranceFee",100);

==================================================
the timer after which an ID session will close if the customer doesnt accept / close it
uint32 idTimer	= gWorldConfig->getConfiguration("Player_Timer_IDSessionTimeOut",(uint32)60000);