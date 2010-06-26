#include "precompiled.h"
#include "icecrown_citadel.h"

typedef UNORDERED_MAP<uint32 /*entry*/, uint32 /*data_id*/> EntryTypeMap;

static EntryTypeMap const CreatureEntryToType = map_initializer<EntryTypeMap>
    (NPC_MARROWGAR,         TYPE_MARROWGAR)
    (NPC_DEATHWHISPER,      TYPE_DEATHWHISPER)
    (NPC_SAURFANG,          TYPE_SAURFANG)
    (NPC_FESTERGUT,         TYPE_FESTERGUT)
    (NPC_ROTFACE,           TYPE_ROTFACE)
    (NPC_PUTRICIDE,         TYPE_PUTRICIDE)
    (NPC_VALANAR,           TYPE_VALANAR)
    (NPC_KELESETH,          TYPE_KELESETH)
    (NPC_TALDARAM,          TYPE_TALDARAM)
    (NPC_LANATHEL,          TYPE_LANATHEL)
    (NPC_VALITHRIA,         TYPE_VALITHRIA)
    (NPC_SINDRAGOSA,        TYPE_SINDRAGOSA)
    (NPC_LICH_KING,         TYPE_LICH_KING);

static EntryTypeMap const GameObjectEntryToType = map_initializer<EntryTypeMap>
    (GO_MARROWGAR_DOOR_1,   DATA_MARROWGAR_DOOR_1)
    (GO_MARROWGAR_DOOR_2,   DATA_MARROWGAR_DOOR_2)
    (GO_DEATHWHISPER_ELEV,  DATA_DEATHWHISPER_ELEV)
    (GO_SAURFANG_CHEST_N10, DATA_SAURFANG_CHEST)
    (GO_SAURFANG_CHEST_N25, DATA_SAURFANG_CHEST)
    (GO_SAURFANG_CHEST_H10, DATA_SAURFANG_CHEST)
    (GO_SAURFANG_CHEST_H25, DATA_SAURFANG_CHEST)
    (GO_SAURFANG_DOOR,      DATA_SAURFANG_DOOR);

namespace icc {

uint32 GetType(Creature *pCreature)
{
    return map_find(CreatureEntryToType, pCreature->GetEntry(), DATA_MAX);
}

uint32 GetType(GameObject *pGO)
{
    return map_find(GameObjectEntryToType, pGO->GetEntry(), DATA_MAX);
}

} // namespace icc

boss_icecrown_citadelAI::boss_icecrown_citadelAI(Creature* pCreature):
    ScriptedAI(pCreature),
    m_pInstance(dynamic_cast<ScriptedInstance*>(pCreature->GetInstanceData())),
    m_BossEncounter(icc::GetType(pCreature), m_pInstance)
{
    Difficulty diff = pCreature->GetMap()->GetDifficulty();
    m_bIsHeroic = diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC;
    m_bIs10Man = diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_10MAN_HEROIC;
    m_BossEncounter = NOT_STARTED;
}

void boss_icecrown_citadelAI::Reset()
{
    Events.Reset();
    m_BossEncounter = NOT_STARTED;
}
