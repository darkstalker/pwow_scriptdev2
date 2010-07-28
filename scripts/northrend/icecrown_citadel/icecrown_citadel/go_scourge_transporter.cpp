/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: go_scourge_transporter
SD%Complete: 0%
SDComment:
SDCategory: Icecrown Citadel
EndScriptData */

#include "precompiled.h"
#include "icecrown_citadel.h"

enum
{
    GOSSIP_TEXT_ID      = 15221,
};

static float const TeleportCoords[TP_MAX][4] =
{
    {-17.0711f, 2211.47f, 30.0546f, 0.0f},
    { -503.62f, 2211.47f, 62.8235f, 3.13002f},
    {-615.145f, 2211.47f, 199.972f, 6.27553f},
    {-549.131f, 2211.29f, 539.291f, 6.26765f},
    { 4199.35f, 2769.42f, 350.977f, 3.14159f},
    { 4356.58f, 2565.75f, 220.402f, -1.5708f},
    { 4356.93f, 2769.41f, 355.955f, -2.35619f}
};

static char const* const GossipStrings[TP_MAX] =
{
    "Light's Hammer",
    "Oratory of the Damned",
    "Rampart of Skulls",
    "Deathbringer's Rise",
    "Upper Spire",
    "Sindragosa's Lair",
    "Frozen Throne"
};

bool GOHello_scourge_transporter(Player *pPlayer, GameObject *pGo)
{
    ScriptedInstance *m_pInstance = dynamic_cast<ScriptedInstance*>(pGo->GetInstanceData());
    if (!m_pInstance)
        return false;
    bool skipCheck = pPlayer->isGameMaster();  // allow GM to teleport anywhere
    std::bitset<32> tpData = m_pInstance->GetData(DATA_TP_UNLOCKED);

    for (uint32 i = 0; i < TP_MAX; ++i)
        if (skipCheck || tpData[i])
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GossipStrings[i], GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + i);

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID, pGo->GetGUID());

    return true;
}

bool GOSelect_scourge_transporter(Player* pPlayer, GameObject* pGo, uint32 uiSender, uint32 uiAction)
{
    if (uiAction >= GOSSIP_ACTION_INFO_DEF && uiAction < GOSSIP_ACTION_INFO_DEF + TP_MAX)
    {
        uint32 i = uiAction - GOSSIP_ACTION_INFO_DEF;
        pPlayer->TeleportTo(pPlayer->GetMapId(), TeleportCoords[i][0], TeleportCoords[i][1], TeleportCoords[i][2], TeleportCoords[i][3], 0);
    }

    pPlayer->CLOSE_GOSSIP_MENU();

    return true;
}

void AddSC_scourge_transporter()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "go_scourge_transporter";
    newscript->pGOGossipHello = &GOHello_scourge_transporter;
    newscript->pGOGossipSelect = &GOSelect_scourge_transporter;
    newscript->RegisterSelf();
}
