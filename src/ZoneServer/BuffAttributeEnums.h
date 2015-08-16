
/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

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

enum BuffAttributeCRC
{
    opAttributeStomach_drink	=   0xB9BC47C4,
    opAttributeUses_Remaining	=	0x1E497382,
    opAttributeAttr_Health		=	0x446B4174,
    opAttributeAttr_Mind		=	0xE54F80BB,
    opBACRC_PerformanceMind		=	0x11C1772E,
    opBACRC_PerformanceFocus	=	0x2E77F586,
    opBACRC_PerformanceWill		=	0x3EC6FCB6,
    opAttribute_Mask_Scent	    =   0xf9c0d603

};

enum BuffAttributes {
    time_remaining = 1,
    accelerate_entertainer_buff = 0xac537651,
    accuracy = 0x36c59ec2,
    attr_action = 0x97af44ff,
    attr_constitution = 0xb7d2595,
    attr_focus = 0x51ca6646,
    attr_health = 0x446b4174,
    attr_mind = 0xe54f80bb,
    attr_quickness = 0x9ab651ee,
    attr_stamina = 0x90c58d54,
    attr_strength = 0x95e993f1,
    attr_willpower = 0x684a8ca2,
    bio_comp_mask_scent = 0xf9c0d603,
    bleed_resist = 0x238eb421,
    blind_defense = 0xb72eead9,
    block = 0x43af11e1,
    burst_run = 0xfe163af5,
    camouflage = 0xb8fb7a65,
    creature_action = 0x27342c4f,
    creature_health = 0xf4f029c4,
    creature_mind = 0xb66b98fb,
    creature_tohit = 0xdc1852d2,
    dizzy_defense = 0xfc6f709a,
    dodge = 0xe430ff04,
    experiment_bonus = 0xaa67d80f,
    fire_resist = 0x46b79d91,
    food_reduce = 0x154a0b57,
    general_assembly = 0xd7c1e80d,
    harvesting = 0x2642fd4,
    healer_speed = 0xff5e125d,
    healing_dance_wound = 0xa184ab57,
    healing_music_wound = 0xca722642,
    healing_wound_treatment = 0x22e73423,
    incap_recovery = 0x1355b931,
    innate_regeneration = 0xefbe623a,
    innate_roar = 0x7b43c7cb,
    intimidate_defense = 0xeb958a6d,
    knockdown_defense = 0x26a24c7c,
    melee_defense = 0x3459f4a4,
    mind_heal = 0x331bad46,
    mitigate_damage = 0xd4dce9e6,
    poison_disease_resist = 0xb991e6d2,
    ranged_defense = 0xf4ecb0c2,
    reduce_clone_wounds = 0x430e1813,
    reduce_spice_downtime = 0x404403be,
    stun_defense = 0x89ccc3d6,
    surveying = 0x581fd626,
    tame_bonus = 0xdef504f2,
    trapping = 0x83437ec3,
    unarmed_damage = 0x929603d5,
    xp_increase = 0x619de2d4
};
