/////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright � 2015 Daniel Allendorf                                        //
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
#include "PacketSwitch.h"
#include "RecvOpcodes.h"
#include "Handlers\CommonHandlers.h"
#include "Handlers\LoginHandlers.h"
#include "Handlers\SetfieldHandlers.h"
#include "Handlers\PlayerHandlers.h"
#include "Handlers\MapobjectHandlers.h"
#include "Handlers\InventoryHandlers.h"
#include "Handlers\MessagingHandlers.h"
#include <iostream>

namespace Net
{
	PacketSwitch::PacketSwitch()
	{
		// Common handlers
		handlers[PING] = unique_ptr<PacketHandler>(new PingHandler());

		// Login handlers
		handlers[LOGIN_RESULT] = unique_ptr<PacketHandler>(new LoginResultHandler());
		handlers[SERVERLIST] = unique_ptr<PacketHandler>(new ServerlistHandler());
		handlers[CHARLIST] = unique_ptr<PacketHandler>(new CharlistHandler());
		handlers[CHARNAME_RESPONSE] = unique_ptr<PacketHandler>(new CharnameResponseHandler());
		handlers[ADD_NEWCHAR_ENTRY] = unique_ptr<PacketHandler>(new AddNewCharEntryHandler());
		handlers[DELCHAR_RESPONSE] = unique_ptr<PacketHandler>(new DeleteCharResponseHandler());
		handlers[SERVER_IP] = unique_ptr<PacketHandler>(new ServerIPHandler());

		// 'Setfield' handlers
		handlers[SET_FIELD] = unique_ptr<PacketHandler>(new SetfieldHandler());

		// MapObject handlers
		handlers[SPAWN_NPC] = unique_ptr<PacketHandler>(new SpawnNpcHandler());
		handlers[SPAWN_NPC_C] = unique_ptr<PacketHandler>(new SpawnNpcControllerHandler());
		handlers[SPAWN_MOB] = unique_ptr<PacketHandler>(new SpawnMobHandler());
		handlers[SPAWN_MOB_C] = unique_ptr<PacketHandler>(new SpawnMobControllerHandler());
		handlers[MOVE_MOB_RESPONSE] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[SHOW_MOB_HP] = unique_ptr<PacketHandler>(new ShowMobHpHandler());
		handlers[KILL_MOB] = unique_ptr<PacketHandler>(new KillMobHandler());
		handlers[SPAWN_PLAYER] = unique_ptr<PacketHandler>(new SpawnCharHandler());
		handlers[PLAYER_MOVED] = unique_ptr<PacketHandler>(new MoveCharHandler());
		handlers[REMOVE_PLAYER] = unique_ptr<PacketHandler>(new RemoveCharHandler());
		handlers[SPAWN_PET] = unique_ptr<PacketHandler>(new SpawnPetHandler());
		handlers[DROP_ITEM_FROMOBJECT] = unique_ptr<PacketHandler>(new DropItemHandler());
		handlers[REMOVE_MAPITEM] = unique_ptr<PacketHandler>(new RemoveDropHandler());

		// Player handlers
		handlers[KEYMAP] = unique_ptr<PacketHandler>(new KeymapHandler());
		handlers[SKILL_MACROS] = unique_ptr<PacketHandler>(new SkillMacrosHandler());
		handlers[CHANGE_STATS] = unique_ptr<PacketHandler>(new ChangeStatsHandler());
		handlers[GIVE_BUFF] = unique_ptr<PacketHandler>(new ApplyBuffHandler());
		handlers[RECALCULATE_STATS] = unique_ptr<PacketHandler>(new RecalculateStatsHandler());
		handlers[UPDATE_SKILLS] = unique_ptr<PacketHandler>(new UpdateskillsHandler());

		// Messaging handlers
		handlers[SHOW_STATUS_INFO] = unique_ptr<PacketHandler>(new ShowStatusInfoHandler());
		handlers[CHAT_RECEIVED] = unique_ptr<PacketHandler>(new ChatReceivedHandler());

		// Inventory Handlers
		handlers[MODIFY_INVENTORY] = unique_ptr<PacketHandler>(new ModifyInventoryHandler());
		handlers[GATHER_RESULT] = unique_ptr<PacketHandler>(new GatherResultHandler());
		handlers[SORT_RESULT] = unique_ptr<PacketHandler>(new SortResultHandler());

		// Todo
		handlers[MEMO_RESULT] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[ENABLE_REPORT] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[BUDDY_LIST] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[GUILD_OPERATION] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[SERVER_MESSAGE] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[WEEK_EVENT_MESSAGE] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[FAMILY_PRIV_LIST] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[SCRIPT_PROGRESS_MESSAGE] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[RECEIVE_POLICE] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[MAKE_NPC_SCRIPTED] = unique_ptr<PacketHandler>(new NullHandler());

		// Ignored
		handlers[SELECT_WORLD] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[RECOMMENDED_WORLDS] = unique_ptr<PacketHandler>(new NullHandler());
		handlers[UPDATE_GENDER] = unique_ptr<PacketHandler>(new NullHandler());
	}

	PacketSwitch::~PacketSwitch() {}

	void PacketSwitch::forward(int8_t* buffer, size_t length) const
	{
		// Read the opcode to determine handler responsible.
		InPacket recv = InPacket(buffer, length);
		uint16_t opcode = recv.readshort();
		if (opcode < NUM_HANDLERS)
		{
			const PacketHandler* handler = handlers[opcode].get();
			if (handler)
			{
				// Handler ok. Packet is passed on.
				handler->handle(recv);
			}
			else
			{
				// Notice about unhandled packet.
				std::cout << "Received Unhandled Packet. Opcode: " << opcode << std::endl;
			}
		}
	}
}