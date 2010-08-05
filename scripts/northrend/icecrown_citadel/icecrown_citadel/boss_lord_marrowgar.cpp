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
SDName: boss_lord_marrowgar
SD%Complete: 80%
SDComment:
SDCategory: Icecrown Citadel
EndScriptData */

#include "precompiled.h"
#include "icecrown_citadel.h"

enum Spells
{
    SPELL_BERSERK               = 26662,
    SPELL_BONE_SLICE            = 69055,
    SPELL_BONE_SPIKE_GRAVEYARD  = 69057,
    SPELL_BONE_STORM            = 69076,
    SPELL_COLDFLAME_DAMAGE_AURA = 69145,
    SPELL_IMPALED               = 69065,
};

enum Summons
{
    NPC_COLDFLAME               = 36672,
    NPC_BONE_SPIKE              = 38711,
};

enum Says
{
    SAY_INTRO                   = -1300400,
    SAY_AGGRO                   = -1300401,
    SAY_BONE_STORM              = -1300402,
    SAY_BONE_SPIKE1             = -1300403,
    SAY_BONE_SPIKE2             = -1300404,
    SAY_BONE_SPIKE3             = -1300405,
    SAY_KILLED_PLAYER1          = -1300406,
    SAY_KILLED_PLAYER2          = -1300407,
    SAY_ENRAGE                  = -1300408,
    SAY_DEATH                   = -1300409,
};

enum Events
{
    EVENT_COLDFLAME = 1,
    EVENT_BONE_SLICE,
    EVENT_COLDFLAME_MOVE,
    EVENT_BONE_STORM,
    EVENT_BONE_STORM_MOVE,
    EVENT_BONE_STORM_STOP,
    EVENT_BONE_SPIKE_GRAVEYARD,
    EVENT_ENRAGE,
};

#define TIMER_BONE_SLICE            1*IN_MILLISECONDS
#define TIMER_COLDFLAME             4*IN_MILLISECONDS, 6*IN_MILLISECONDS
#define TIMER_COLDFLAME_MOVE        10  // creature is updated rougly every 500ms, so this might not work as expected
#define TIMER_BONE_STORM            50*IN_MILLISECONDS
#define TIMER_BONE_STORM_MOVE       5*IN_MILLISECONDS
#define TIMER_BONE_SPIKE_GRAVEYARD  20*IN_MILLISECONDS
#define TIMER_ENRAGE                10*MINUTE*IN_MILLISECONDS

struct MANGOS_DLL_DECL mob_bone_spikeAI: public Scripted_NoMovementAI
{
    Unit *pTarget;

    mob_bone_spikeAI(Creature *pCreature):
        Scripted_NoMovementAI(pCreature),
        pTarget(NULL)
    {
    }

    void Reset()
    {
    }

    void JustDied(Unit *pKiller)
    {
        RemoveImpale();
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (!pTarget)
            return;

        if (pTarget->isAlive())
            m_creature->SetTargetGUID(pTarget->GetGUID());
        else
            m_creature->ForcedDespawn();
    }

    void DoImpale(Unit* target)
    {
        if (!target)
            return;
        pTarget = target;
        DoCast(target, SPELL_IMPALED, true);
    }

    void RemoveImpale()
    {
        if (pTarget)
        {
            pTarget->RemoveAurasDueToSpell(SPELL_IMPALED);
            pTarget = NULL;
        }
    }
};

struct MANGOS_DLL_DECL boss_lord_marrowgarAI: public boss_icecrown_citadelAI
{
    SummonManager SummonMgr;
    bool m_bSaidBeginningStuff :1;
    bool m_bInBoneStorm :1;
    std::list<std::pair<WorldLocation/*start point*/, uint32/*dist*/> > ColdflameAttribs;

    boss_lord_marrowgarAI(Creature* pCreature):
        boss_icecrown_citadelAI(pCreature),
        SummonMgr(m_creature),
        m_bSaidBeginningStuff(false),
        m_bInBoneStorm(false)
    {
    }

    void Reset()
    {
        m_bInBoneStorm = false;
        SummonMgr.UnsummonAll();
        ColdflameAttribs.clear();

        Map::PlayerList const &Players = m_creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = Players.begin(); itr != Players.end(); ++itr)
        {
            Unit *pPlayer = itr->getSource();
            if (!pPlayer)
                continue;
            pPlayer->RemoveAurasDueToSpell(SPELL_IMPALED);
        }

        boss_icecrown_citadelAI::Reset();
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (!m_bSaidBeginningStuff && pWho && pWho->GetTypeId() == TYPEID_PLAYER)
        {
            m_bSaidBeginningStuff = true;
            DoScriptText(SAY_INTRO, m_creature);
        }
        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        SCHEDULE_EVENT_R(COLDFLAME);
        SCHEDULE_EVENT(BONE_SPIKE_GRAVEYARD);
        SCHEDULE_EVENT(BONE_STORM);
        Events.ScheduleEvent(EVENT_BONE_SLICE, 10*IN_MILLISECONDS, TIMER_BONE_SLICE);
        SCHEDULE_EVENT(ENRAGE);
        m_BossEncounter = IN_PROGRESS;
    }

    void KilledUnit(Unit* pWho)
    {
        if (pWho && pWho->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(urand(0,1) ? SAY_KILLED_PLAYER1 : SAY_KILLED_PLAYER2, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
        m_BossEncounter = DONE;
    }

    void SpellHitTarget(Unit* victim, SpellEntry const* pSpell)
    {
        SpellEntry const *CompareSpell = GetSpellStore()->LookupEntry(SPELL_BONE_SPIKE_GRAVEYARD);
        if (CompareSpell && pSpell->SpellDifficultyId == CompareSpell->SpellDifficultyId)
            if (Unit *pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM_PLAYER, 2)) //do not attack tanks.
                if (Creature *pSumm = SummonMgr.SummonCreatureAt(pTarget, NPC_BONE_SPIKE, TEMPSUMMON_TIMED_DESPAWN, 5*MINUTE*IN_MILLISECONDS))
                    if (mob_bone_spikeAI *SpikeAI = dynamic_cast<mob_bone_spikeAI*>(pSumm->AI()))
                        SpikeAI->DoImpale(pTarget);
    }

    void SummonedCreatureJustDied(Creature *pSumm)
    {
        SummonMgr.RemoveSummonFromList(pSumm->GetObjectGuid());
    }

    void GetPointOnCircle(float rad, float ang, float &x, float &y) const
    {
        x = rad * cos(ang);
        y = rad * sin(ang);
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim() || OutOfCombatAreaCheck())
            return;

        Events.Update(uiDiff);
        while (uint32 uiEventId = Events.ExecuteEvent())
            switch (uiEventId)
            {
                case EVENT_BONE_SLICE:
                    if (!m_bInBoneStorm)
                        DoCast(m_creature->getVictim(), SPELL_BONE_SLICE);
                    break;
                case EVENT_COLDFLAME:
                {
                    WorldLocation StartPos;
                    m_creature->GetPosition(StartPos);
                    if (m_bInBoneStorm)
                    {
                        for (uint32 i = 0; i < 4; i++)
                        {
                            StartPos.orientation += i*(M_PI/2);
                            ColdflameAttribs.push_back(std::make_pair(StartPos, 4));
                        }
                    }
                    else
                    {
                        if (Unit *target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        {
                            StartPos.orientation = m_creature->GetAngle(target);
                            ColdflameAttribs.push_back(std::make_pair(StartPos, 4));
                        }
                    }
                    // (no break)
                }
                case EVENT_COLDFLAME_MOVE:
                {
                    for (std::list<std::pair<WorldLocation, uint32> >::iterator i = ColdflameAttribs.begin(); i != ColdflameAttribs.end(); )
                    {
                        if (i->second <= 25)
                        {
                            float x, y;
                            i->second += 3;
                            GetPointOnCircle(i->second, i->first.orientation, x, y);
                            Creature *flame = SummonMgr.SummonCreature(NPC_COLDFLAME, i->first.coord_x+x, i->first.coord_y+y, i->first.coord_z, i->first.orientation, TEMPSUMMON_TIMED_DESPAWN, m_bIsHeroic ? 8000 : 3000);
                            if (flame)
                                flame->CastSpell(m_creature, SPELL_COLDFLAME_DAMAGE_AURA, false);
                            Events.ScheduleEvent(EVENT_COLDFLAME_MOVE, TIMER_COLDFLAME_MOVE);
                            ++i;
                        }
                        else
                            i = ColdflameAttribs.erase(i);
                    }
                    break;
                }
                case EVENT_BONE_SPIKE_GRAVEYARD:
                {
                    if (!m_bIsHeroic && m_bInBoneStorm)
                        break;
                    switch (urand(0, 2))
                    {
                        case 0: DoScriptText(SAY_BONE_SPIKE1, m_creature); break;
                        case 1: DoScriptText(SAY_BONE_SPIKE2, m_creature); break;
                        case 2: DoScriptText(SAY_BONE_SPIKE3, m_creature); break;
                    }
                    DoCast(m_creature, SPELL_BONE_SPIKE_GRAVEYARD);
                    break;
                }
                case EVENT_BONE_STORM:
                    DoScriptText(SAY_BONE_STORM, m_creature);
                    DoCast(m_creature, SPELL_BONE_STORM);
                    m_creature->SetSpeedRate(MOVE_WALK, 5.0f, true);
                    m_creature->SetSpeedRate(MOVE_RUN, 5.0f, true);
                    DoStartNoMovement(m_creature->getVictim());
                    m_bInBoneStorm = true;
                    for (uint32 i = 0; i < (m_bIsHeroic ? 4 : 3); i++)
                        Events.ScheduleEvent(EVENT_BONE_STORM_MOVE, 4000 + i*TIMER_BONE_STORM_MOVE);
                    Events.ScheduleEvent(EVENT_BONE_STORM_STOP, TIMER_BONE_STORM_MOVE*(m_bIsHeroic ? 5 : 4));
                    break;
                case EVENT_BONE_STORM_MOVE:
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM_PLAYER, 0))
                    {
                        float x, y, z;
                        pTarget->GetPosition(x, y, z);
                        m_creature->GetMotionMaster()->MovePoint(0, x, y, z);
                    }
                    break;
                }
                case EVENT_BONE_STORM_STOP:
                    m_creature->RemoveAurasDueToSpell(SPELL_BONE_STORM);
                    m_bInBoneStorm = false;
                    m_creature->SetSpeedRate(MOVE_WALK, 1.0f, true);
                    m_creature->SetSpeedRate(MOVE_RUN, 1.0f, true);
                    DoStartMovement(m_creature->getVictim());
                    Events.DelayEventsWithId(EVENT_BONE_SLICE, 10*IN_MILLISECONDS);
                    break;
                case EVENT_ENRAGE:  //you lose!
                    DoScriptText(SAY_ENRAGE, m_creature);
                    DoCast(m_creature, SPELL_BERSERK);
                    break;
                default:
                    break;
            }

        if (!m_bInBoneStorm) // prevent melee damage while in bonestorm
            DoMeleeAttackIfReady();
    }
};

void AddSC_boss_lord_marrowgar()
{
    Script *newscript;

    REGISTER_SCRIPT(boss_lord_marrowgar);
    REGISTER_SCRIPT(mob_bone_spike);
}
