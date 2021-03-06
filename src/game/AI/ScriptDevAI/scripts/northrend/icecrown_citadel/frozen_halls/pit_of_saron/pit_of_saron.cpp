/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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
SDName: instance_pit_of_saron
SD%Complete: 80%
SDComment:
SDCategory: Pit of Saron
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "pit_of_saron.h"

enum
{
    // Ambush and Gauntlet
    SAY_TYRANNUS_AMBUSH_1           = -1658047,
    SAY_TYRANNUS_AMBUSH_2           = -1658048,
    SAY_GAUNTLET                    = -1658049,

    // Gauntlet spells
    SPELL_ICICLE_SUMMON             = 69424,
    SPELL_ACHIEVEMENT_CHECK         = 72845,

    // Tyrannus intro
    SAY_PREFIGHT_1                  = -1658050,
    SAY_VICTUS_TRASH                = -1658051,
    SAY_IRONSKULL_TRASH             = -1658068,
    SAY_PREFIGHT_2                  = -1658052,

    SPELL_EJECT_ALL_PASSENGERS      = 50630,
    // SPELL_CSA_DUMMY_EFFECT_1     = 56685,                // What is this?

    // Sindragosa outro
    SAY_VICTUS_OUTRO_1              = -1658061,
    SAY_IRONSKULL_OUTRO_2           = -1658069,
    SAY_GENERAL_OUTRO_2             = -1658062,
    SAY_JAINA_OUTRO_1               = -1658063,
    SAY_SYLVANAS_OUTRO_1            = -1658064,
    SAY_JAINA_OUTRO_2               = -1658065,
    SAY_JAINA_OUTRO_3               = -1658066,
    SAY_SYLVANAS_OUTRO_2            = -1658067,

    SPELL_FROST_BOMB                = 70521,
    SPELL_FROZEN_AFTERMATH          = 70518,
    SPELL_ARCANE_FORM               = 70573,
    SPELL_CALL_OF_SYLVANAS_1        = 70636,                // triggers 70639
    SPELL_CALL_OF_SYLVANAS_2        = 70638,
    // SPELL_CALL_OF_SYLVANAS_3     = 70642,
    SPELL_JAINAS_CALL_1             = 70527,                // triggers 70525
    SPELL_JAINAS_CALL_2             = 70623,
};

static const DialogueEntryTwoSide aPoSDialogues[] =
{
    // Tyrannus intro
    {SAY_PREFIGHT_1,       NPC_TYRANNUS,       0,                    0,                  13000},
    {SAY_VICTUS_TRASH,     NPC_VICTUS_PART2,   SAY_IRONSKULL_TRASH,  NPC_IRONSKULL_PART2, 9000},
    {SAY_PREFIGHT_2,       NPC_TYRANNUS,       0,                    0,                  10000},
    {NPC_RIMEFANG,         0,                  0,                    0,                  0},

    // Tyrannus outro
    {NPC_SINDRAGOSA,       0,                  0,                    0,                  30000},
    {SAY_VICTUS_OUTRO_1,   NPC_VICTUS_PART2,   SAY_IRONSKULL_OUTRO_2, NPC_IRONSKULL_PART2, 17000},
    {SAY_GENERAL_OUTRO_2,  NPC_VICTUS_PART2,   SAY_GENERAL_OUTRO_2,  NPC_IRONSKULL_PART2, 14000},
    {SAY_JAINA_OUTRO_1,    NPC_JAINA_PART2,    SAY_SYLVANAS_OUTRO_1, NPC_SYLVANAS_PART2,  1000},
    {SPELL_FROST_BOMB,     0,                  0,                    0,                   7000},
    {NPC_JAINA_PART2,      0,                  0,                    0,                   8000},
    {SAY_JAINA_OUTRO_2,    NPC_JAINA_PART2,    SAY_SYLVANAS_OUTRO_2, NPC_SYLVANAS_PART2,  15000},
    {SAY_JAINA_OUTRO_3,    NPC_JAINA_PART2,    0,                    0,                   0},
    {0, 0, 0},
};

instance_pit_of_saron::instance_pit_of_saron(Map* pMap) : ScriptedInstance(pMap), DialogueHelper(aPoSDialogues),
    m_uiAmbushAggroCount(0),
    m_uiTeam(TEAM_NONE),
    m_uiSummonDelayTimer(0),
    m_uiIciclesTimer(0),
    m_uiEyeLichKingTimer(0)
{
    Initialize();
}

void instance_pit_of_saron::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    InitializeDialogueHelper(this);

    for (bool& i : m_abAchievCriteria)
        i = false;
}

void instance_pit_of_saron::OnPlayerEnter(Player* pPlayer)
{
    if (!m_uiTeam)                                          // very first player to enter
    {
        m_uiTeam = pPlayer->GetTeam();
        SetDialogueSide(m_uiTeam == ALLIANCE);

        if (GetData(TYPE_AMBUSH) == NOT_STARTED)
            m_uiEyeLichKingTimer = 5000;

        // dialogue starts on timer if any of the first two bosses are not already dead
        if (GetData(TYPE_GARFROST) == DONE || GetData(TYPE_KRICK) == DONE)
        {
            if (GetData(TYPE_AMBUSH) == NOT_STARTED)
                pPlayer->SummonCreature(NPC_TYRANNUS_INTRO, afTyrannusMovePos[2][0], afTyrannusMovePos[2][1], afTyrannusMovePos[2][2], 0, TEMPSPAWN_DEAD_DESPAWN, 0);
        }
        else
            m_uiSummonDelayTimer = 10000;
    }
}

void instance_pit_of_saron::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_TYRANNUS_INTRO:
        case NPC_JAINA_PART1:
        case NPC_SYLVANAS_PART1:
        case NPC_GARFROST:
        case NPC_KRICK:
        case NPC_ICK:
        case NPC_TYRANNUS:
        case NPC_RIMEFANG:
        case NPC_IRONSKULL_PART1:
        case NPC_VICTUS_PART1:
        case NPC_IRONSKULL_PART2:
        case NPC_VICTUS_PART2:
        case NPC_JAINA_PART2:
        case NPC_SYLVANAS_PART2:
        case NPC_SINDRAGOSA:
        case NPC_EYE_LICH_KING:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_STALKER:
            m_lTunnelStalkersGuidList.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_YMIRJAR_DEATHBRINGER:
        case NPC_YMIRJAR_WRATHBRINGER:
        case NPC_YMIRJAR_FLAMEBEARER:
        case NPC_FALLEN_WARRIOR:
        case NPC_COLDWRAITH:
            // Sort only the temporary summons
            if (pCreature->IsTemporarySummon())
                m_lAmbushNpcsGuidList.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_GENERAL_BUNNY:
            if (pCreature->GetPositionY() < 130.0f)
            {
                if (pCreature->GetOrientation() != 0)
                    m_lArcaneShieldBunniesGuidList.push_back(pCreature->GetObjectGuid());
                else
                    m_lFrozenAftermathBunniesGuidList.push_back(pCreature->GetObjectGuid());
            }
            break;
    }
}

void instance_pit_of_saron::OnCreatureRespawn(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        // following have passive behavior movement
        case NPC_FROSTBLADE:
            pCreature->AI()->SetReactState(REACT_PASSIVE);
            pCreature->SetCanEnterCombat(false);
            break;
        case NPC_TYRANNUS_INTRO:
            // temp summoned Tyrannus moves in a circle around the instance
            if (pCreature->IsTemporarySummon())
                pCreature->GetMotionMaster()->MoveWaypoint(1);
            break;
    }
}

void instance_pit_of_saron::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_ICEWALL:
            if (m_auiEncounter[TYPE_GARFROST] == DONE && m_auiEncounter[TYPE_KRICK] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_HALLS_OF_REFLECT_PORT:
            break;
        case GO_SARONITE_ROCK:
            m_lSaroniteRockGuidList.push_back(pGo->GetObjectGuid());
            break;

        default:
            return;
    }
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_pit_of_saron::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_GARFROST:
            if (uiData == DONE && m_auiEncounter[TYPE_KRICK] == DONE)
                DoUseDoorOrButton(GO_ICEWALL);
            if (uiData == DONE || uiData == FAIL)
            {
                for (const auto& guid : m_lSaroniteRockGuidList)
                    if (GameObject* pRock = instance->GetGameObject(guid))
                        pRock->SetLootState(GO_JUST_DEACTIVATED);

                m_lSaroniteRockGuidList.clear();
            }
            if (uiData == IN_PROGRESS)
                SetSpecialAchievementCriteria(TYPE_ACHIEV_DOESNT_GO_ELEVEN, true);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_KRICK:
            if (uiData == DONE && m_auiEncounter[TYPE_GARFROST] == DONE)
                DoUseDoorOrButton(GO_ICEWALL);
            if (uiData == DONE)
            {
                for (const auto guid : m_lIntroCreaturesGuidList)
                    if (Creature* creature = instance->GetCreature(guid))
                        creature->ForcedDespawn();
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_TYRANNUS:
            if (uiData == DONE)
            {
                StartNextDialogueText(NPC_SINDRAGOSA);

                for (const auto guid : m_lEndingCreaturesGuidList)
                    if (Creature* creature = instance->GetCreature(guid))
                        creature->GetMotionMaster()->UnpauseWaypoints();
            }
            else if (uiData == SPECIAL)
                DoStartTyrannusEvent();
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_AMBUSH:
            if (uiData == DONE)
            {
                // Complete tunnel achievement
                if (Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS))
                    pTyrannus->CastSpell(pTyrannus, SPELL_ACHIEVEMENT_CHECK, TRIGGERED_OLD_TRIGGERED);

                // Despawn the Tyrannus at the entrance
                if (Creature* creature = GetSingleCreatureFromStorage(NPC_TYRANNUS_INTRO))
                    creature->ForcedDespawn();

                // Despawn Jaina / Sylvanas
                if (Creature* creature = GetSingleCreatureFromStorage(GetPlayerTeam() == HORDE ? NPC_SYLVANAS_PART1 : NPC_JAINA_PART1))
                    creature->ForcedDespawn();

                m_uiIciclesTimer = 0;
                m_uiEyeLichKingTimer = 0;
            }
            else if (uiData == IN_PROGRESS)
                DoStartAmbushEvent();
            m_auiEncounter[uiType] = uiData;
            break;
        default:
            return;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_pit_of_saron::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_pit_of_saron::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_pit_of_saron::OnCreatureEnterCombat(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_YMIRJAR_DEATHBRINGER)
    {
        ++m_uiAmbushAggroCount;

        // Summon the rest of the mobs at the 2nd ambush
        if (m_uiAmbushAggroCount == 2)
        {
            Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS_INTRO);
            if (!pTyrannus)
                return;

            DoScriptText(SAY_TYRANNUS_AMBUSH_2, pTyrannus);
            pTyrannus->SetWalk(false);
            pTyrannus->GetMotionMaster()->Clear(false, true);
            pTyrannus->GetMotionMaster()->MovePoint(0, afTyrannusMovePos[3][0], afTyrannusMovePos[3][1], afTyrannusMovePos[3][2]);

            // Spawn Mobs
            for (const auto& aEventSecondAmbushLocation : aEventSecondAmbushLocations)
            {
                if (Creature* pSummon = pTyrannus->SummonCreature(aEventSecondAmbushLocation.uiEntryHorde, aEventSecondAmbushLocation.fX, aEventSecondAmbushLocation.fY,
                    aEventSecondAmbushLocation.fZ, aEventSecondAmbushLocation.fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                {
                    pSummon->SetWalk(false);
                    pSummon->GetMotionMaster()->MoveWaypoint(aEventSecondAmbushLocation.pathId);
                }
            }
        }
    }
}

void instance_pit_of_saron::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_YMIRJAR_DEATHBRINGER:
        case NPC_YMIRJAR_WRATHBRINGER:
        case NPC_YMIRJAR_FLAMEBEARER:
        case NPC_FALLEN_WARRIOR:
        case NPC_COLDWRAITH:
            // Check for tunnel event end - these mobs are not summoned
            if (pCreature->IsTemporarySummon())
            {
                m_lAmbushNpcsGuidList.remove(pCreature->GetObjectGuid());

                // If empty start tunnel event
                if (m_lAmbushNpcsGuidList.empty())
                {
                    Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS_INTRO);
                    if (!pTyrannus)
                        return;

                    DoScriptText(SAY_GAUNTLET, pTyrannus);

                    pTyrannus->SetWalk(false);
                    pTyrannus->GetMotionMaster()->Clear(false, true);
                    pTyrannus->GetMotionMaster()->MovePoint(0, afTyrannusMovePos[0][0], afTyrannusMovePos[0][1], afTyrannusMovePos[0][2]);

                    m_uiIciclesTimer = urand(3000, 5000);
                    SetSpecialAchievementCriteria(TYPE_ACHIEV_DONT_LOOK_UP, true);
                }
            }
            break;
    }
}

void instance_pit_of_saron::SetSpecialAchievementCriteria(uint32 uiType, bool bIsMet)
{
    if (uiType < MAX_SPECIAL_ACHIEV_CRITS)
        m_abAchievCriteria[uiType] = bIsMet;
}

bool instance_pit_of_saron::CheckAchievementCriteriaMeet(uint32 uiCriteriaId, Player const* /*pSource*/, Unit const* /*pTarget*/, uint32 /*uiMiscValue1 = 0*/) const
{
    switch (uiCriteriaId)
    {
        case ACHIEV_CRIT_DOESNT_GO_ELEVEN:
            return m_abAchievCriteria[TYPE_ACHIEV_DOESNT_GO_ELEVEN];
        case ACHIEV_CRIT_DONT_LOOK_UP:
            return m_abAchievCriteria[TYPE_ACHIEV_DONT_LOOK_UP];

        default:
            return false;
    }
}

void instance_pit_of_saron::JustDidDialogueStep(int32 iEntry)
{
    switch (iEntry)
    {
        case NPC_RIMEFANG:
            // Eject Tyrannus and prepare for combat
            if (Creature* pRimefang = GetSingleCreatureFromStorage(NPC_RIMEFANG))
            {
                pRimefang->CastSpell(pRimefang, SPELL_EJECT_ALL_PASSENGERS, TRIGGERED_OLD_TRIGGERED);
                pRimefang->SetWalk(false);
                pRimefang->GetMotionMaster()->MovePoint(0, afTyrannusMovePos[1][0], afTyrannusMovePos[1][1], afTyrannusMovePos[1][2]);
            }
            if (Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS))
                pTyrannus->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            break;
        case SAY_VICTUS_OUTRO_1:
        {
            Player* pPlayer = GetPlayerInMap();
            if (!pPlayer)
                return;

            // Spawn Sindragosa
            if (Creature* pSummon = pPlayer->SummonCreature(aEventOutroLocations[0].uiEntryHorde, aEventOutroLocations[0].fX, aEventOutroLocations[0].fY,
                                    aEventOutroLocations[0].fZ, aEventOutroLocations[0].fO, TEMPSPAWN_TIMED_DESPAWN, 2 * MINUTE * IN_MILLISECONDS))
            {
                pSummon->SetWalk(false);
                pSummon->GetMotionMaster()->MovePoint(0, aEventOutroLocations[0].fMoveX, aEventOutroLocations[0].fMoveY, aEventOutroLocations[0].fMoveZ);
            }
            // Spawn Jaina or Sylvanas
            if (Creature* pSummon = pPlayer->SummonCreature(m_uiTeam == HORDE ? aEventOutroLocations[1].uiEntryHorde : aEventOutroLocations[1].uiEntryAlliance,
                                    aEventOutroLocations[1].fX, aEventOutroLocations[1].fY, aEventOutroLocations[1].fZ, aEventOutroLocations[1].fO, TEMPSPAWN_TIMED_DESPAWN, 24 * HOUR * IN_MILLISECONDS))
            {
                pSummon->SetWalk(false);
                pSummon->GetMotionMaster()->MovePoint(0, aEventOutroLocations[1].fMoveX, aEventOutroLocations[1].fMoveY, aEventOutroLocations[1].fMoveZ);
            }
            break;
        }
        case SAY_JAINA_OUTRO_1:
            // Visual effect
            for (GuidList::const_iterator itr = m_lArcaneShieldBunniesGuidList.begin(); itr != m_lArcaneShieldBunniesGuidList.end(); ++itr)
            {
                if (Creature* pBunny = instance->GetCreature(*itr))
                    pBunny->CastSpell(pBunny, SPELL_ARCANE_FORM, TRIGGERED_OLD_TRIGGERED);
            }
            // Teleport players
            if (Creature* pTemp = GetSingleCreatureFromStorage(m_uiTeam == HORDE ? NPC_SYLVANAS_PART2 : NPC_JAINA_PART2))
            {
                pTemp->CastSpell(pTemp, m_uiTeam == HORDE ? SPELL_CALL_OF_SYLVANAS_2 : SPELL_JAINAS_CALL_2, TRIGGERED_OLD_TRIGGERED);
                pTemp->CastSpell(pTemp, m_uiTeam == HORDE ? SPELL_CALL_OF_SYLVANAS_2 : SPELL_JAINAS_CALL_2, TRIGGERED_OLD_TRIGGERED);
            }
            break;
        case SPELL_FROST_BOMB:
            // Frost bomb on the platform
            if (Creature* pSindragosa = GetSingleCreatureFromStorage(NPC_SINDRAGOSA))
                pSindragosa->CastSpell(pSindragosa, SPELL_FROST_BOMB, TRIGGERED_OLD_TRIGGERED);
            // Visual effect
            for (GuidList::const_iterator itr = m_lFrozenAftermathBunniesGuidList.begin(); itr != m_lFrozenAftermathBunniesGuidList.end(); ++itr)
            {
                if (Creature* pBunny = instance->GetCreature(*itr))
                    pBunny->CastSpell(pBunny, SPELL_FROZEN_AFTERMATH, TRIGGERED_OLD_TRIGGERED);
            }
            break;
        case NPC_JAINA_PART2:
            // Visual effect remove
            for (GuidList::const_iterator itr = m_lArcaneShieldBunniesGuidList.begin(); itr != m_lArcaneShieldBunniesGuidList.end(); ++itr)
            {
                if (Creature* pBunny = instance->GetCreature(*itr))
                    pBunny->RemoveAurasDueToSpell(SPELL_ARCANE_FORM);
            }
            // Sindragosa exit
            if (Creature* pSindragosa = GetSingleCreatureFromStorage(NPC_SINDRAGOSA))
                pSindragosa->GetMotionMaster()->MovePoint(0, 759.148f, 199.955f, 720.857f);
            // Jaina / Sylvanas starts moving (should use wp)
            if (Creature* pTemp = GetSingleCreatureFromStorage(m_uiTeam == HORDE ? NPC_SYLVANAS_PART2 : NPC_JAINA_PART2))
            {
                pTemp->SetWalk(true);
                pTemp->GetMotionMaster()->MovePoint(0, 1057.76f, 111.927f, 628.4123f);
            }
            break;
        case SAY_JAINA_OUTRO_2:
            if (Creature* pTemp = GetSingleCreatureFromStorage(m_uiTeam == HORDE ? NPC_SYLVANAS_PART2 : NPC_JAINA_PART2))
                pTemp->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

            // ToDo: Jaina / Sylvanas should have some waypoint movement here and the door should be opened only when they get in front of it.
            DoUseDoorOrButton(GO_HALLS_OF_REFLECT_PORT);
            break;
    }
}

// Method to start the intro event
void instance_pit_of_saron::DoStartIntroEvent()
{
    Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS_INTRO);
    if (!pTyrannus)
    {
        script_error_log("instance_pit_of_saron: Error: cannot find creature %u in instance", NPC_TYRANNUS_INTRO);
        return;
    }

    // Spawn Begin Mobs; script handled in DB; Dialogue handled in DB
    for (const auto& aEventBeginLocation : aEventBeginLocations)
    {
        if (Creature* pSummon = pTyrannus->SummonCreature(m_uiTeam == HORDE ? aEventBeginLocation.uiEntryHorde : aEventBeginLocation.uiEntryAlliance,
            aEventBeginLocation.fX, aEventBeginLocation.fY, aEventBeginLocation.fZ, aEventBeginLocation.fO, TEMPSPAWN_TIMED_DESPAWN, 24 * HOUR * IN_MILLISECONDS))
        {
            pSummon->SetWalk(false);
            pSummon->GetMotionMaster()->MoveWaypoint(aEventBeginLocation.pathId);

            m_lIntroCreaturesGuidList.push_back(pSummon->GetObjectGuid());
        }
    }
}

// Method to start the ambush event
void instance_pit_of_saron::DoStartAmbushEvent()
{
    Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS_INTRO);
    if (!pTyrannus)
    {
        script_error_log("instance_pit_of_saron: Error: cannot find creature %u in instance", NPC_TYRANNUS_INTRO);
        return;
    }

    DoScriptText(SAY_TYRANNUS_AMBUSH_1, pTyrannus);

    // Spawn Mobs
    for (const auto& aEventFirstAmbushLocation : aEventFirstAmbushLocations)
    {
        if (Creature* pSummon = pTyrannus->SummonCreature(aEventFirstAmbushLocation.uiEntryHorde, aEventFirstAmbushLocation.fX, aEventFirstAmbushLocation.fY,
            aEventFirstAmbushLocation.fZ, aEventFirstAmbushLocation.fO, TEMPSPAWN_DEAD_DESPAWN, 0))
        {
            pSummon->SetWalk(false);
            pSummon->GetMotionMaster()->MoveWaypoint(aEventFirstAmbushLocation.pathId);
        }
    }
}

// Function to spawn the Tyrannus event mobs
void instance_pit_of_saron::DoStartTyrannusEvent()
{
    Creature* pTyrannus = GetSingleCreatureFromStorage(NPC_TYRANNUS);
    if (!pTyrannus)
    {
        script_error_log("instance_pit_of_saron: Error: cannot find creature %u in instance", NPC_TYRANNUS);
        return;
    }

    // Handle dialogue
    StartNextDialogueText(SAY_PREFIGHT_1);

    // Spawn Mobs
    for (const auto& aEventTunnelEndLocation : aEventTunnelEndLocations)
    {
        if (Creature* pSummon = pTyrannus->SummonCreature(m_uiTeam == HORDE ? aEventTunnelEndLocation.uiEntryHorde : aEventTunnelEndLocation.uiEntryAlliance,
            aEventTunnelEndLocation.fX, aEventTunnelEndLocation.fY, aEventTunnelEndLocation.fZ, aEventTunnelEndLocation.fO, TEMPSPAWN_DEAD_DESPAWN, 0))
        {
            pSummon->SetWalk(false);
            pSummon->GetMotionMaster()->MoveWaypoint(aEventTunnelEndLocation.pathId);

            m_lEndingCreaturesGuidList.push_back(pSummon->GetObjectGuid());
        }
    }
}

void instance_pit_of_saron::Update(uint32 uiDiff)
{
    DialogueUpdate(uiDiff);

    if (m_uiSummonDelayTimer)
    {
        if (m_uiSummonDelayTimer <= uiDiff)
        {
            DoStartIntroEvent();
            m_uiSummonDelayTimer = 0;
        }
        else
            m_uiSummonDelayTimer -= uiDiff;
    }

    if (m_uiIciclesTimer)
    {
        if (m_uiIciclesTimer <= uiDiff)
        {
            for (const auto guid : m_lTunnelStalkersGuidList)
            {
                // Only 25% of the stalkers will actually spawn an icicle
                if (roll_chance_i(50))
                    continue;

                if (Creature* pStalker = instance->GetCreature(guid))
                    pStalker->CastSpell(pStalker, SPELL_ICICLE_SUMMON, TRIGGERED_OLD_TRIGGERED);
            }
            m_uiIciclesTimer = urand(3000, 5000);
        }
        else
            m_uiIciclesTimer -= uiDiff;
    }

    if (m_uiEyeLichKingTimer)
    {
        if (m_uiEyeLichKingTimer <= uiDiff)
        {
            // the eye will randomly face the players
            if (Creature* pEye = GetSingleCreatureFromStorage(NPC_EYE_LICH_KING))
            {
                Player* pPlayer = GetPlayerInMap();
                if (!pPlayer)
                {
                    script_error_log("instance_pit_of_saron: Error: couldn't find any player in instance");
                    m_uiEyeLichKingTimer = 0;
                }

                pEye->SetFacingToObject(pPlayer);

                m_uiEyeLichKingTimer = urand(10000, 30000);
            }
        }
        else
            m_uiEyeLichKingTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_pit_of_saron(Map* pMap)
{
    return new instance_pit_of_saron(pMap);
}

void AddSC_instance_pit_of_saron()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_pit_of_saron";
    pNewScript->GetInstanceData = &GetInstanceData_instance_pit_of_saron;
    pNewScript->RegisterSelf();
}
