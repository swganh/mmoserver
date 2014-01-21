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

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

use swganh;

--
-- Definition of table `biography_templates`
--

DROP TABLE IF EXISTS `biography_templates`;
CREATE TABLE `biography_templates` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `biography` text NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `biography_templates`
--

/*!40000 ALTER TABLE `biography_templates` DISABLE KEYS */;
INSERT INTO `biography_templates` (`id`,`biography`) VALUES 
 (3,'Jenna Zan Arbor will die by my <SPECIES>s hand.  For too long, her \"experiments\" have plagued the citizens of <PLANET>.  My eyes will close never again until she has perished. While exploring the wilderness of <PLANET>, three of my <PROFESSION> colleagues and I stumbled upon what we perceived to be an abandoned structure. The doors ajar, we proceeded to enter the the building with our blasters ready. Little did we know we had discovered one of Jenna Zan Arbor\'s secret research facilities, populated by twisted and sinister beings of unimaginable horror. Two of my companions were instantly killed as one of these beings sprang out of the shadows and plunged its claw-like arms through their torsos.  Only grace and my honed <PROFESSION> instincts allowed me to remove myself from the carnage as my third friend fell victim to another ominous being.  After dispatching the two beasts and mourning my companions\' fate for a time, I searched the facility hoping to locate some clue that would lead me to the source of these malevolent beings - and there it was: a data pad with the locations of Zan Arbor\'s other secret facilities on <PLANET>...'),
 (4,'The Galactic Games will be held on <PLANET> this cycle! Fortune has smiled upon me!  No longer will I be a struggling <PROFESSION>, barely eking out a living like the rest of the <SPECIES>s! With my unrivalled <PROFESSION> skills, I will easily be appointed to Team <PLANET>!  After we are victorious, and my talent is revealed to rich patrons from across the galaxy, I will be able to leave <PLANET> and continue to train somewhere more significant...more lucrative! Not since Maxo Vista has another Galactic Games\\\' champion been lauded with such praise or showered with such riches as I will be...soon enough.  I will usurp his fame... I will become the most famous <SPECIES> ever to travel the galaxy.'),
 (5,'Some say the Tuskens are savages. I say they are much more savage beings in the \"civilized\" sectors of <CITY>.  I would much prefer a Gaffi Stick to the noggin for stealing the last bantha steak to a blaster to the gut wielded by a trigger-happy Sarrish. As a small child, I was abandoned by my <SPECIES> parents in the wastelands of Tatooine.  A Tusken scout, realizing what was transpiring before his very eyes, slaughtered them mercilessly and returned to his camp with me in tow.  Being raised by such a proud and strong society was harsh, but instilled in me a vigor and ferocity that has become all but necessary in my dealings as a <PROFESSION>. After being banished from the tribe at 23, a common phenomenon in Tusken society, I somehow made my way to my home planet of <PLANET> in search of any surviving family members.  I found several members of the <surname> clan thriving in <city>, but after various mishaps and altercations I found myself shunned by yet another \"family\" and was relegated once again to the role of outsider.  Now is my time to exact revenge on all <SPECIES>s of <PLANET> - they are loathsome bunch and deserve no mercy. They will receive none from me.'),
 (6,'After following the Galactic Civil War for 23 years with no real interest in involvement, I was finally forced into the conflict after my parents, Ja\'ndak and Ranay <surname>, were brutally assaulted by Imperial troops garrisoned on <PLANET>.  This attack was unprovoked: My parents were in <city> bartering for needed provisions when an Imperial crackdown force descended on the city and began harassing innocent citizens.  My father valiantly stepped between three stormtroopers who were beating a child with the butts of their carbines and he was gunned down immediately. I am a novice <PROFESSION> at best.  However, I am patient, and I will study without cessation until I am proficient enough to be of service to the Rebellion.  Until that day comes, nothing will stop me from learning as much as I can about being the best <prof> known to <PLANET>. Someday, through the bloodied lenses of his battered helmet, a stormtrooper will look up at my burning eyes, my sweat-soaked hair, and realize that it is his time to die.  In the transient moments before death, he will also see the fate of the Empire that he murdered for, and he will realize that the days of this corrupt Empire are numbered.  Justice and redemption shall prevail once again.'),
 (7,'I feel it all around me - it\'s a blessing and a curse.  I have a natural aptitude for the Force, and I feel lost, but also found at the same time. I first recognized my gift while residing at the B\'omarr monastery - the moments of solitude afforded me there allowed me to develop my connection to the Force. After this epiphany, I decided to remove myself from the monastery, relocate to <PLANET>, and pursue the path of the <PROFESSION>.  Afterall, <SPECIES>s have historically been some of the most succcessful <PROFESSION>s in the galaxy.  One who possess my gift would be no exception. However, I\'m still conflicted: Shall I use my predanatural sensitivity to the force to pursue the path of virtue and harmony?  Or should exploit the lesser beings and amass a fortune in credits?  I have not yet decided, but it is time to begin my journey...\r\n');
/*!40000 ALTER TABLE `biography_templates` ENABLE KEYS */;



/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;