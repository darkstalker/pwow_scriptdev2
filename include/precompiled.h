/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PRECOMPILED_H
#define SC_PRECOMPILED_H

#include "../ScriptMgr.h"
#include "sc_creature.h"
#include "sc_gossip.h"
#include "sc_grid_searchers.h"
#include "sc_instance.h"
#include "sc_utility.h"

#ifdef WIN32
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule,
DWORD  ul_reason_for_call,
LPVOID lpReserved
)
{
    return true;
}
#endif

#endif
