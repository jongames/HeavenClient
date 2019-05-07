//////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright © 2015-2016 Daniel Allendorf                                   //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////
#include "EquipTooltip.h"

#include "../../Gameplay/Stage.h"
#include "../../Data/EquipData.h"
#include "../../Data/WeaponData.h"

#include "nlnx/nx.hpp"
#include "nlnx/node.hpp"

namespace jrc
{
	EquipTooltip::EquipTooltip()
	{
		nl::node Item = nl::nx::ui["UIToolTip.img"]["Item"];
		nl::node Frame = Item["Frame"];
		nl::node ItemIcon = Item["ItemIcon"];
		nl::node Equip = Item["Equip"];
		nl::node EquipCan = Equip["Can"];
		nl::node EquipCannot = Equip["Cannot"];

		top = Frame["top"];
		mid = Frame["line"];
		line = Frame["dotline"];
		bot = Frame["bottom"];
		base = ItemIcon["base"];
		cover = Frame["cover"];
		itemcover = ItemIcon["cover"];
		type[true] = ItemIcon["new"];
		type[false] = ItemIcon["old"];

		potential[Equip::Potential::POT_NONE] = {};
		potential[Equip::Potential::POT_HIDDEN] = ItemIcon["0"];
		potential[Equip::Potential::POT_RARE] = ItemIcon["1"];
		potential[Equip::Potential::POT_EPIC] = ItemIcon["2"];
		potential[Equip::Potential::POT_UNIQUE] = ItemIcon["3"];
		potential[Equip::Potential::POT_LEGENDARY] = ItemIcon["4"];

		requirements.push_back(Maplestat::Id::LEVEL);
		requirements.push_back(Maplestat::Id::STR);
		requirements.push_back(Maplestat::Id::DEX);
		requirements.push_back(Maplestat::Id::INT);
		requirements.push_back(Maplestat::Id::LUK);

		reqstattextures[Maplestat::Id::LEVEL][false] = EquipCannot["reqLEV"];
		reqstattextures[Maplestat::Id::LEVEL][true] = EquipCan["reqLEV"];
		reqstattextures[Maplestat::Id::FAME][false] = EquipCannot["reqPOP"];
		reqstattextures[Maplestat::Id::FAME][true] = EquipCan["reqPOP"];
		reqstattextures[Maplestat::Id::STR][false] = EquipCannot["reqSTR"];
		reqstattextures[Maplestat::Id::STR][true] = EquipCan["reqSTR"];
		reqstattextures[Maplestat::Id::DEX][false] = EquipCannot["reqDEX"];
		reqstattextures[Maplestat::Id::DEX][true] = EquipCan["reqDEX"];
		reqstattextures[Maplestat::Id::INT][false] = EquipCannot["reqINT"];
		reqstattextures[Maplestat::Id::INT][true] = EquipCan["reqINT"];
		reqstattextures[Maplestat::Id::LUK][false] = EquipCannot["reqLUK"];
		reqstattextures[Maplestat::Id::LUK][true] = EquipCan["reqLUK"];

		reqstatpositions[Maplestat::Id::LEVEL] = Point<int16_t>(97, 47);
		reqstatpositions[Maplestat::Id::STR] = Point<int16_t>(97, 62);
		reqstatpositions[Maplestat::Id::LUK] = Point<int16_t>(177, 62);
		reqstatpositions[Maplestat::Id::DEX] = Point<int16_t>(97, 71);
		reqstatpositions[Maplestat::Id::INT] = Point<int16_t>(177, 71);

		reqset[false] = Charset(EquipCannot, Charset::Alignment::LEFT);
		reqset[true] = Charset(EquipCan, Charset::Alignment::LEFT);

		lvset[false] = Charset(EquipCannot, Charset::Alignment::LEFT);
		lvset[true] = Charset(Equip["YellowNumber"], Charset::Alignment::LEFT);

		atkincset[false] = Charset(Equip["Summary"]["decline"], Charset::Alignment::RIGHT);
		atkincset[true] = Charset(Equip["Summary"]["incline"], Charset::Alignment::RIGHT);

		jobsback = Equip["Job"]["normal"];
		jobs[false][0] = Equip["Job"]["disable"]["0"];
		jobs[false][1] = Equip["Job"]["disable"]["1"];
		jobs[false][2] = Equip["Job"]["disable"]["2"];
		jobs[false][3] = Equip["Job"]["disable"]["3"];
		jobs[false][4] = Equip["Job"]["disable"]["4"];
		jobs[false][5] = Equip["Job"]["disable"]["5"];
		jobs[true][0] = Equip["Job"]["enable"]["0"];
		jobs[true][1] = Equip["Job"]["enable"]["1"];
		jobs[true][2] = Equip["Job"]["enable"]["2"];
		jobs[true][3] = Equip["Job"]["enable"]["3"];
		jobs[true][4] = Equip["Job"]["enable"]["4"];
		jobs[true][5] = Equip["Job"]["enable"]["5"];

		invpos = 0;
	}

	void EquipTooltip::set_equip(Tooltip::Parent parent, int16_t ivp)
	{
		if (ivp == invpos)
			return;

		invpos = ivp;

		const Player& player = Stage::get().get_player();

		InventoryType::Id invtype;

		switch (parent)
		{
		case Tooltip::Parent::ITEMINVENTORY:
		case Tooltip::Parent::SHOP:
			invtype = InventoryType::Id::EQUIP;
			break;
		case Tooltip::Parent::EQUIPINVENTORY:
			invtype = InventoryType::Id::EQUIPPED;
			break;
		default:
			invtype = InventoryType::Id::NONE;
		}

		auto oequip = player.get_inventory().get_equip(invtype, ivp);

		if (!oequip)
			return;

		const Equip& equip = *oequip;

		int32_t item_id = equip.get_item_id();

		const EquipData& equipdata = EquipData::get(item_id);
		const ItemData& itemdata = equipdata.get_itemdata();
		const CharStats& stats = player.get_stats();

		height = 540;

		itemicon = itemdata.get_icon(false);

		for (auto& ms : requirements)
		{
			canequip[ms] = stats.get_stat(ms) >= equipdata.get_reqstat(ms);
			std::string reqstr = std::to_string(equipdata.get_reqstat(ms));

			if (ms != Maplestat::Id::LEVEL)
				reqstr.insert(0, 3 - reqstr.size(), '0');

			reqstatstrings[ms] = reqstr;
		}

		okjobs.clear();

		switch (equipdata.get_reqstat(Maplestat::Id::JOB))
		{
		case 0:
			okjobs.push_back(0);
			okjobs.push_back(1);
			okjobs.push_back(2);
			okjobs.push_back(3);
			okjobs.push_back(4);
			okjobs.push_back(5);
			canequip[Maplestat::Id::JOB] = true;
			break;
		case 1:
			okjobs.push_back(1);
			canequip[Maplestat::Id::JOB] = (stats.get_stat(Maplestat::Id::JOB) / 100 == 1) || (stats.get_stat(Maplestat::Id::JOB) / 100 >= 20);
			break;
		case 2:
			okjobs.push_back(2);
			canequip[Maplestat::Id::JOB] = stats.get_stat(Maplestat::Id::JOB) / 100 == 2;
			break;
		case 4:
			okjobs.push_back(3);
			canequip[Maplestat::Id::JOB] = stats.get_stat(Maplestat::Id::JOB) / 100 == 3;
			break;
		case 8:
			okjobs.push_back(4);
			canequip[Maplestat::Id::JOB] = stats.get_stat(Maplestat::Id::JOB) / 100 == 4;
			break;
		case 16:
			okjobs.push_back(5);
			canequip[Maplestat::Id::JOB] = stats.get_stat(Maplestat::Id::JOB) / 100 == 5;
			break;
		default:
			canequip[Maplestat::Id::JOB] = false;
			break;
		}

		prank = equip.get_potrank();

		switch (prank)
		{
		case Equip::Potential::POT_HIDDEN:
			potflag = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::RED);
			potflag.change_text("(Hidden Potential)");
			break;
		case Equip::Potential::POT_RARE:
			potflag = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE);
			potflag.change_text("(Rare Item)");
			break;
		case Equip::Potential::POT_EPIC:
			potflag = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE);
			potflag.change_text("(Epic Item)");
			break;
		case Equip::Potential::POT_UNIQUE:
			potflag = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE);
			potflag.change_text("(Unique Item)");
			break;
		case Equip::Potential::POT_LEGENDARY:
			potflag = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE);
			potflag.change_text("(Legendary Item)");
			break;
		default:
			height -= 16;
			break;
		}

		Text::Color namecolor;

		switch (equip.get_quality())
		{
		case EquipQuality::Id::GREY:
			namecolor = Text::Color::LIGHTGREY;
			break;
		case EquipQuality::Id::ORANGE:
			namecolor = Text::Color::ORANGE;
			break;
		case EquipQuality::Id::BLUE:
			namecolor = Text::Color::MEDIUMBLUE;
			break;
		case EquipQuality::Id::VIOLET:
			namecolor = Text::Color::VIOLET;
			break;
		case EquipQuality::Id::GOLD:
			namecolor = Text::Color::YELLOW;
			break;
		default:
			namecolor = Text::Color::WHITE;
			break;
		}

		std::string namestr = itemdata.get_name();

		if (equip.get_level() > 0)
		{
			namestr.append(" (+");
			namestr.append(std::to_string(equip.get_level()));
			namestr.append(")");
		}

		name = Text(Text::Font::A12B, Text::Alignment::LEFT, namecolor, namestr, 400);
		atkinc = Text(Text::Font::A11M, Text::Alignment::RIGHT, Text::Color::DUSTYGRAY, "ATT INCREASE");

		std::string desctext = itemdata.get_desc();
		hasdesc = desctext.size() > 0;

		if (hasdesc)
		{
			desc = Text(Text::Font::A12M, Text::Alignment::LEFT, Text::Color::WHITE, desctext, 250);
			height += desc.height() + 10;
		}

		category = Text(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::WHITE, "Type: " + equipdata.get_type());

		is_weapon = equipdata.is_weapon();

		if (is_weapon)
		{
			const WeaponData& weapon = WeaponData::get(item_id);
			wepspeed = Text(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::WHITE, "Attack Speed: " + weapon.getspeedstring());
		}
		else
		{
			height -= 18;
		}

		hasslots = (equip.get_slots() > 0) || (equip.get_level() > 0);

		if (hasslots)
		{
			slots = Text(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::WHITE, "Remaining Enhancements: " + std::to_string(equip.get_slots()));

			std::string vicious = std::to_string(equip.get_vicious());

			if (equip.get_vicious() > 1)
				vicious.append(" (MAX) ");

			hammers = Text(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::WHITE, "Hammers Applied: " + vicious);
		}
		else
		{
			height -= 36;
		}

		statlabels.clear();

		for (Equipstat::Id es = Equipstat::Id::STR; es <= Equipstat::Id::JUMP; es = static_cast<Equipstat::Id>(es + 1))
		{
			if (equip.get_stat(es) > 0)
			{
				int16_t delta = equip.get_stat(es) - equipdata.get_defstat(es);
				std::string statstr = std::to_string(equip.get_stat(es));

				if (delta != 0)
				{
					statstr.append(" (");
					statstr.append((delta < 0) ? "-" : "+");
					statstr.append(std::to_string(abs(delta)) + ")");
				}

				statlabels[es] = Text(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::WHITE, Equipstat::names[es] + std::string(": ") + statstr);
			}
			else
			{
				height -= 18;
			}
		}
	}

	void EquipTooltip::draw(Point<int16_t> pos) const
	{
		if (invpos == 0)
			return;

		int16_t max_width = Constants::Constants::get().get_viewwidth();
		int16_t max_height = Constants::Constants::get().get_viewheight();
		int16_t cur_width = pos.x() + top.width();
		int16_t cur_height = pos.y() + height + 36;

		int16_t adj_x = cur_width - max_width;
		int16_t adj_y = cur_height - max_height;

		if (adj_x > 0)
			pos.shift_x(adj_x * -1);

		if (adj_y > 0)
			pos.shift_y(adj_y * -1);

		top.draw(pos);
		mid.draw(DrawArgument(pos + Point<int16_t>(0, 13), Point<int16_t>(0, height)));
		bot.draw(pos + Point<int16_t>(0, height + 13));
		cover.draw(pos);

		name.draw(pos + Point<int16_t>(17, 7));

		if (prank != Equip::Potential::POT_NONE)
		{
			potflag.draw(pos + Point<int16_t>(130, 20));
			pos.shift_y(16);
		}

		pos.shift_y(44);

		line.draw(pos);

		atkinc.draw(pos + Point<int16_t>(248, 4));
		base.draw(pos + Point<int16_t>(12, 10));
		type[false].draw(pos + Point<int16_t>(12, 10));
		itemicon.draw(DrawArgument(pos + Point<int16_t>(18, 82), 2.0f, 2.0f));
		potential[prank].draw(pos + Point<int16_t>(12, 10));
		itemcover.draw(pos + Point<int16_t>(12, 10));

		int16_t atkincnum = 0;
		std::string atkincstr = std::to_string(atkincnum);
		bool atkinc_pos = true;

		if (canequip[Maplestat::Id::JOB])
		{
			if (atkincnum < 0)
			{
				atkincstr = "m" + atkincstr;
				atkinc_pos = false;
			}
			else if (atkincnum > 0)
			{
				atkincstr = "p" + atkincstr;
				atkinc_pos = true;
			}
			else
			{
				atkinc_pos = true;
			}
		}
		else
		{
			atkincstr = "m";
			atkinc_pos = false;
		}

		atkincset[atkinc_pos].draw(atkincstr, 11, pos + Point<int16_t>(239, 26));

		pos.shift_y(12);

		for (Maplestat::Id ms : requirements)
		{
			Point<int16_t> reqpos = reqstatpositions[ms];
			bool reqok = canequip[ms];
			reqstattextures[ms][reqok].draw(pos + reqpos);

			if (ms != Maplestat::Id::LEVEL)
				reqset[reqok].draw(reqstatstrings[ms], 6, pos + reqpos + Point<int16_t>(54, 0));
			else
				lvset[reqok].draw(reqstatstrings[ms], 6, pos + reqpos + Point<int16_t>(54, 0));
		}

		pos.shift_y(88);

		Point<int16_t> job_position(pos + Point<int16_t>(10, 14));
		jobsback.draw(job_position);

		for (auto& jbit : okjobs)
			jobs[canequip[Maplestat::Id::JOB]].at(jbit).draw(job_position);

		line.draw(pos + Point<int16_t>(0, 47));

		pos.shift_y(49);

		int16_t stat_x = 13;
		int16_t stat_y = 15;

		category.draw(pos + Point<int16_t>(stat_x, 0));

		pos.shift_y(stat_y);

		if (is_weapon)
		{
			wepspeed.draw(pos + Point<int16_t>(stat_x, 0));
			pos.shift_y(stat_y);
		}

		for (const Text& label : statlabels.values())
		{
			if (label.empty())
				continue;

			label.draw(pos + Point<int16_t>(stat_x, 0));
			pos.shift_y(stat_y);
		}

		if (hasslots)
		{
			slots.draw(pos + Point<int16_t>(stat_x, 0));
			pos.shift_y(stat_y);
			hammers.draw(pos + Point<int16_t>(stat_x, 0));
			pos.shift_y(stat_y);
		}

		if (hasdesc)
		{
			pos.shift_y(-4);
			line.draw(pos);
			desc.draw(pos + Point<int16_t>(9, 8));
		}
	}
}