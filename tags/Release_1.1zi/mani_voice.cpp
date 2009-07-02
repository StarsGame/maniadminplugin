//
// Mani Admin Plugin
//
// Copyright (c) 2009 Giles Millward (Mani). All rights reserved.
//
// This file is part of ManiAdminPlugin.
//
// Mani Admin Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Mani Admin Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Mani Admin Plugin.  If not, see <http://www.gnu.org/licenses/>.
//






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "dlls/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "mrecipientfilter.h" 
#include "bitbuf.h"
#include "engine/IEngineSound.h"
#include "inetchannelinfo.h"
#include "ivoiceserver.h"
#include "networkstringtabledefs.h"
#include "mani_main.h"
#include "mani_convar.h"
#include "mani_parser.h"
#include "mani_player.h"
#include "mani_language.h"
#include "mani_menu.h"
#include "mani_memory.h"
#include "mani_output.h"
#include "mani_admin_flags.h"
#include "mani_admin.h"
#include "mani_gametype.h"
#include "mani_effects.h"
#include "mani_voice.h"

extern	IVEngineServer *engine;
extern	IVoiceServer *voiceserver;
extern	IPlayerInfoManager *playerinfomanager;
extern	int	max_players;
extern	CGlobalVars *gpGlobals;
extern	bool war_mode;
extern	ConVar	*sv_alltalk;

static	bool IsPlayerValid(player_t *player_ptr);

inline bool FStruEq(const char *sz1, const char *sz2)
{
	return(Q_strcmp(sz1, sz2) == 0);
}

inline bool FStrEq(const char *sz1, const char *sz2)
{
	return(Q_stricmp(sz1, sz2) == 0);
}

static void ManiDeadAllTalk ( ConVar *var, char const *pOldString );

ConVar mani_dead_alltalk ("mani_dead_alltalk", "0", 0, "0 = Dont let dead players from each team talk, 1 = Let dead players from each team talk", true, 0, true, 1, ManiDeadAllTalk); 


//---------------------------------------------------------------------------------
// Purpose: Process if the player can talk when dead or not
//---------------------------------------------------------------------------------
bool	ProcessDeadAllTalk
(
 int	receiver_index,
 int	sender_index,
 bool	*new_listen
)
{
//	Msg("Receiver [%i] Sender [%i]\n", receiver_index, sender_index);
	if (!voiceserver) return false;
	if (war_mode) return false;
	if (!gpManiGameType->IsTeamPlayAllowed()) return false;

	if (punish_mode_list[sender_index - 1].muted != 0||
		punish_mode_list[sender_index - 1].gimped != 0)
	{
		*new_listen = false;
		return true;
	}

	if (!gpManiGameType->IsTeamPlayAllowed()) return false;
	if (mani_dead_alltalk.GetInt() == 0) return false;
	if (sv_alltalk && sv_alltalk->GetInt() == 1) return false;

	player_t player_receiver;
	player_t player_sender;

	player_sender.index = sender_index;
	player_receiver.index = receiver_index;

	if (!IsPlayerValid(&player_sender)) return false;
	if (!IsPlayerValid(&player_receiver)) return false;

	if (gpManiGameType->IsSpectatorAllowed())
	{
		if (player_sender.team == gpManiGameType->GetSpectatorIndex()) return false;
		if (player_receiver.team == gpManiGameType->GetSpectatorIndex()) return false;
	}

	if (gpManiGameType->IsValidActiveTeam(player_sender.team) && gpManiGameType->IsValidActiveTeam(player_receiver.team))
	{
		if (player_sender.team != player_receiver.team)
		{
			*new_listen = true;
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------------
// Purpose: Faster version of FindPlayerByIndex
//---------------------------------------------------------------------------------

static
bool IsPlayerValid(player_t *player_ptr)
{

	if (player_ptr->index < 1 || player_ptr->index > max_players)
	{
		return false;
	}

	edict_t *pEntity = engine->PEntityOfEntIndex(player_ptr->index);
	if(pEntity && !pEntity->IsFree() )
	{
		IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( pEntity );
		if (playerinfo && playerinfo->IsConnected())
		{
			if (strcmp(playerinfo->GetNetworkIDString(), "BOT") == 0) return false;
			if (playerinfo->IsHLTV()) return false;
			player_ptr->team = playerinfo->GetTeamIndex();
			if (playerinfo->IsDead()) return true;
			return false;
		}
	}

	return false;
}

static void ManiDeadAllTalk ( ConVar *var, char const *pOldString )
{
	if (!FStrEq(pOldString, var->GetString()))
	{
		if (atoi(var->GetString()) == 0)
		{
			SayToAll(true, "DeadAllTalk mode off");
		}
		else
		{
			SayToAll(true, "DeadAllTalk mode on");
		}
	}
}