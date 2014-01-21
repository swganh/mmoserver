/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

--
-- Create swganh users
--

GRANT ALL PRIVILEGES ON *.* TO 'chatserver'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'connectionserver'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'loginserver'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'corellia'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'dantooine'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'dathomir'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'endor'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'lok'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'naboo'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'rori'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'talus'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'tanaab'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'tatooine'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'tutorial'@'%' IDENTIFIED BY 'swganh';
GRANT ALL PRIVILEGES ON *.* TO 'yavin4'@'%' IDENTIFIED BY 'swganh';

--
-- Administrator Privileges for ALL databases (USE WITH CAUTION)
--
-- Simply uncomment the line below to apply
--
-- GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'password goes here' WITH GRANT OPTION;
