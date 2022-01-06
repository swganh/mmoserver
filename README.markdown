# MMOServer #

The MMOServer is the flagship project for the [SWG:ANH Team][1]. It is a cross platform massively multiplayer game server intended to emulate the [Star Wars Galaxies][2] Pre-Combat Upgrade experience. The base of the server is written in C++ with some LUA sprinkled in.


## Building/Installing ##

**Pre-Requisits Windows**

*   CMake 2.8 or higher

    [Download][3] the latest version of CMake for your OS.

*   C++ Compatible Compiler

    Windows: Visual Studio 2013 required 
	
	[Download][7]
	
	MySQL Server 5.1
	
	[Download][8]
	
	MySql Workbench (optional)
	
	[Download][9]    
	
	
**Pre-Requisits Unix**

*   CMake 2.8 or higher

    [Download][3] the latest version of CMake for your OS.
	
*   C++ Compatible Compiler
	
	Unix: GCC 4.6 or higher is required
	

### Windows Builds ###

To build the server on Windows simply double-click the build_server.bat file in the project root. This will download and build all the dependencies and sources and then generate a /bin directory with the server executables.

### Unix Builds ###

To build the server on Unix platforms run the bootstrap.sh script in the project root. This will download and build all the dependencies and sources. Once the script has completed you can issue further builds from within the "build" directory:

    ./boot_server.sh
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/opt/local ..
    make install
    
You can use the -DCMAKE\_INSTALL\_PREFIX flag to specify a custom output directory for the make install command. 

## Useful Links ##

*   [Bugtracker][4]
*   [Wiki][5]
*   [Forum][6]

  [1]: http://swganh.com/
  [2]: http://starwarsgalaxies.com/
  [3]: http://cmake.org/cmake/resources/software.html
  [4]: http://bugtracker.swganh.com/
  [5]: http://wiki.swganh.org/
  [6]: http://www.swganh.com/anh_community/
  [7]: https://my.visualstudio.com/Downloads?q=visual%20studio%202013&wt.mc_id=o~msft~vscom~older-downloads
  [8]: https://downloads.mysql.com/archives/community/
  [9]: https://downloads.mysql.com/archives/workbench/
