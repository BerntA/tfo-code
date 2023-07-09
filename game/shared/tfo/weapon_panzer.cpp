//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Bernt - Modified to have proper physics & gravity affections.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_panzer.h"
#include "hl2_shareddefs.h"
#include "rumble_shared.h"

#ifdef GAME_DLL
#include "smoke_trail.h"
#include "te_effect_dispatch.h"
#include "triggers.h"
#include "explode.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RPG_SPEED	3000
#define ROCKET_MODEL "models/weapons/w_panzerschreck_rocket.mdl"

#ifdef GAME_DLL
BEGIN_DATADESC(CMissile)

DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_flAugerTime, FIELD_TIME),
DEFINE_FIELD(m_flMarkDeadTime, FIELD_TIME),
DEFINE_FIELD(m_flGracePeriodEndsAt, FIELD_TIME),
DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
DEFINE_FIELD(m_bCreateDangerSounds, FIELD_BOOLEAN),

// Function Pointers
DEFINE_FUNCTION(MissileTouch),
DEFINE_FUNCTION(AccelerateThink),
DEFINE_FUNCTION(AugerThink),
DEFINE_FUNCTION(IgniteThink),
DEFINE_FUNCTION(DumbThink), // Added dumb think method.

END_DATADESC()

LINK_ENTITY_TO_CLASS(rpg_missile, CMissile);

CMissile::CMissile()
{
	m_hRocketTrail = NULL;
	m_bCreateDangerSounds = false;
}

CMissile::~CMissile()
{
}

void CMissile::Precache(void)
{
	PrecacheModel(ROCKET_MODEL);
}

void CMissile::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);
	SetModel(ROCKET_MODEL);
	UTIL_SetSize(this, -Vector(4, 4, 4), Vector(4, 4, 4));

	SetTouch(&CMissile::MissileTouch);

	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetThink(&CMissile::IgniteThink);

	SetNextThink(gpGlobals->curtime + 0.3f);
	SetDamage(200.0f);

	// Test out grav.
	// We should definately have the rocket be modified by gravity.
	SetGravity(0.425);

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 100;
	m_bloodColor = DONT_BLEED;
	m_flGracePeriodEndsAt = 0;

	AddFlag(FL_OBJECT);
}

void CMissile::Event_Killed(const CTakeDamageInfo& info)
{
	m_takedamage = DAMAGE_NO;

	ShotDown();
}

unsigned int CMissile::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

int CMissile::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	if ((info.GetDamageType() & (DMG_MISSILEDEFENSE | DMG_AIRBOAT)) == false)
		return 0;

	bool bIsDamaged;
	if (m_iHealth <= AugerHealth())
	{
		// This missile is already damaged (i.e., already running AugerThink)
		bIsDamaged = true;
	}
	else
	{
		// This missile isn't damaged enough to wobble in flight yet
		bIsDamaged = false;
	}

	int nRetVal = BaseClass::OnTakeDamage_Alive(info);

	if (!bIsDamaged)
	{
		if (m_iHealth <= AugerHealth())
		{
			ShotDown();
		}
	}

	return nRetVal;
}

void CMissile::SetGracePeriod(float flGracePeriod)
{
	m_flGracePeriodEndsAt = gpGlobals->curtime + flGracePeriod;

	// Go non-solid until the grace period ends
	AddSolidFlags(FSOLID_NOT_SOLID);
}

void CMissile::AccelerateThink(void)
{
	Vector vecForward;

	// !!!UNDONE - make this work exactly the same as HL1 RPG, lest we have looping sound bugs again!
	EmitSound("Missile.Accelerate");

	// SetEffects( EF_LIGHT );

	AngleVectors(GetLocalAngles(), &vecForward);
	SetAbsVelocity(vecForward * RPG_SPEED);

	SetThink(&CMissile::DumbThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

#define AUGER_YDEVIANCE 20.0f
#define AUGER_XDEVIANCEUP 8.0f
#define AUGER_XDEVIANCEDOWN 1.0f

void CMissile::AugerThink(void)
{
	// If we've augered long enough, then just explode
	if (m_flAugerTime < gpGlobals->curtime)
	{
		Explode();
		return;
	}

	if (m_flMarkDeadTime < gpGlobals->curtime)
	{
		m_lifeState = LIFE_DYING;
	}

	QAngle angles = GetLocalAngles();

	angles.y += random->RandomFloat(-AUGER_YDEVIANCE, AUGER_YDEVIANCE);
	angles.x += random->RandomFloat(-AUGER_XDEVIANCEDOWN, AUGER_XDEVIANCEUP);

	SetLocalAngles(angles);

	Vector vecForward;

	AngleVectors(GetLocalAngles(), &vecForward);

	SetAbsVelocity(vecForward * 1000.0f);

	SetNextThink(gpGlobals->curtime + 0.05f);
}

void CMissile::ShotDown(void)
{
	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();

	DispatchEffect("RPGShotDown", data);

	if (m_hRocketTrail != NULL)
	{
		m_hRocketTrail->m_bDamaged = true;
	}

	SetThink(&CMissile::AugerThink);
	SetNextThink(gpGlobals->curtime);
	m_flAugerTime = gpGlobals->curtime + 1.5f;
	m_flMarkDeadTime = gpGlobals->curtime + 0.75;
}

void CMissile::DoExplosion(void)
{
	// Explode
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), GetDamage(), CMissile::EXPLOSION_RADIUS,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);
}

void CMissile::Explode(void)
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	Vector forward;

	GetVectors(&forward, NULL, NULL);

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	m_takedamage = DAMAGE_NO;
	SetSolid(SOLID_NONE);
	if (tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY))
	{
		DoExplosion();
	}

	if (m_hRocketTrail)
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	StopSound("Missile.Ignite");
	UTIL_Remove(this);
}

void CMissile::MissileTouch(CBaseEntity* pOther)
{
	Assert(pOther);

	// Don't touch triggers (but DO hit weapons)
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON)
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	Explode();
}

void CMissile::CreateSmokeTrail(void)
{
	if (m_hRocketTrail)
		return;

	// Smoke trail.
	if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f, 0.65f);
		m_hRocketTrail->m_EndColor.Init(0.0, 0.0, 0.0);
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 32;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;

		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
}

void CMissile::IgniteThink(void)
{
	// If it's MOVETYPE_FLY then gravity won't affect it.
	// SetMoveType( MOVETYPE_FLY );
	ViewPunch(QAngle(-2, 0, 0));
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetModel(ROCKET_MODEL);
	UTIL_SetSize(this, vec3_origin, vec3_origin);
	RemoveSolidFlags(FSOLID_NOT_SOLID);

	Vector vecForward;

	EmitSound("Missile.Ignite");

	AngleVectors(GetLocalAngles(), &vecForward);
	SetAbsVelocity(vecForward * RPG_SPEED);

	SetThink(&CMissile::DumbThink);
	SetNextThink(gpGlobals->curtime);

	CreateSmokeTrail();
}

void CMissile::DumbThink(void)
{
	// If we have a grace period, go solid when it ends
	if (m_flGracePeriodEndsAt)
	{
		if (m_flGracePeriodEndsAt < gpGlobals->curtime)
		{
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			m_flGracePeriodEndsAt = 0;
		}
	}

	// Correct the face of the rocket.
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

CMissile* CMissile::Create(const Vector& vecOrigin, const QAngle& vecAngles, edict_t* pentOwner = NULL)
{
	CMissile* pMissile = (CMissile*)CBaseEntity::Create("rpg_missile", vecOrigin, vecAngles, CBaseEntity::Instance(pentOwner));
	pMissile->SetOwnerEntity(Instance(pentOwner));
	pMissile->Spawn();
	pMissile->AddEffects(EF_NOSHADOW);

	Vector vecForward;
	AngleVectors(vecAngles, &vecForward);
	pMissile->SetAbsVelocity(vecForward * 300 + Vector(0, 0, 128));

	return pMissile;
}
#endif

//=============================================================================
// PANZER
//=============================================================================

#ifdef GAME_DLL
BEGIN_DATADESC(CWeaponRPG)
DEFINE_FIELD(m_bWantsReload, FIELD_BOOLEAN),
END_DATADESC()
#endif

IMPLEMENT_SERVERCLASS_ST(CWeaponRPG, DT_WeaponRPG)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_panzer, CWeaponRPG);
PRECACHE_WEAPON_REGISTER(weapon_panzer);

acttable_t	CWeaponRPG::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_RPG, true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_RPG_RELAXED,			true },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_RPG,				true },

	{ ACT_IDLE,						ACT_IDLE_RPG,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_WALK,						ACT_WALK_RPG,					true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RPG,			true },
	{ ACT_RUN,						ACT_RUN_RPG,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RPG,				true },
	{ ACT_COVER_LOW,				ACT_COVER_LOW_RPG,				true },

	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_RPG,                    false },
	{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_RPG,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_RPG,            false },
	{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_RPG,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_RPG,        false },
	{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_RPG,                    false },
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_RPG,                false },
};

IMPLEMENT_ACTTABLE(CWeaponRPG);

CWeaponRPG::CWeaponRPG()
{
	m_bReloadsSingly = true;
	m_bWantsReload = false;
	m_fMinRange1 = m_fMinRange2 = 40 * 12;
	m_fMaxRange1 = m_fMaxRange2 = 500 * 12;
}

CWeaponRPG::~CWeaponRPG()
{
}

void CWeaponRPG::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("Missile.Ignite");
	PrecacheScriptSound("Missile.Accelerate");

	UTIL_PrecacheOther("rpg_missile");
}

bool CWeaponRPG::WeaponShouldBeLowered(void)
{
	return (HasAnyAmmo() ? BaseClass::WeaponShouldBeLowered() : true);
}

void CWeaponRPG::PrimaryAttack(void)
{
	if (m_bWantsReload || (GetActivity() == ACT_VM_RELOAD))
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

#ifdef GAME_DLL
	Vector	vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
	QAngle vecAngles;
	VectorAngles(vForward, vecAngles);
	CMissile* pNewMissile = CMissile::Create(muzzlePoint, vecAngles, GetOwner()->edict());

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction == 1.0)
		pNewMissile->SetGracePeriod(0.3);
#endif

	DecrementAmmo(GetOwner());

#ifdef GAME_DLL
	// Register a muzzleflash for the AI
	pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
#endif

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	WeaponSound(SINGLE);

#ifdef GAME_DLL
	pOwner->RumbleEffect(RUMBLE_SHOTGUN_SINGLE, 0, RUMBLE_FLAG_RESTART);
#endif

	m_iPrimaryAttacks++;

#ifdef GAME_DLL
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// Check to see if we should trigger any RPG firing triggers
	int iCount = g_hWeaponFireTriggers.Count();
	for (int i = 0; i < iCount; i++)
	{
		if (g_hWeaponFireTriggers[i]->IsTouching(pOwner))
		{
			if (FClassnameIs(g_hWeaponFireTriggers[i], "trigger_rpgfire"))
			{
				g_hWeaponFireTriggers[i]->ActivateMultiTrigger(pOwner);
			}
		}
	}
#endif

	m_bWantsReload = true;
}

void CWeaponRPG::DecrementAmmo(CBaseCombatCharacter* pOwner)
{
	// Take away our primary ammo type
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

void CWeaponRPG::ItemPostFrame(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	CheckReload();
	BaseClass::ItemPostFrame();
}

void CWeaponRPG::CheckReload(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner || (m_flNextPrimaryAttack >= gpGlobals->curtime))
		return;

	if (m_bWantsReload)
	{
		Reload();
		return;
	}

	if (m_bInReload)
		m_bInReload = false;
}

bool CWeaponRPG::Reload(void)
{
	CBaseCombatCharacter* pOwner = GetOwner();
	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_bIsIronsighted)
		DisableIronsights();

	WeaponSound(RELOAD);
	SendWeaponAnim(ACT_VM_RELOAD);

	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_bInReload = true;
	m_bWantsReload = false;

	return true;
}

#ifdef GAME_DLL
bool CWeaponRPG::WeaponLOSCondition(const Vector& ownerPos, const Vector& targetPos, bool bSetConditions)
{
	bool bResult = BaseClass::WeaponLOSCondition(ownerPos, targetPos, bSetConditions);

	if (bResult)
	{
		CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

		if (npcOwner)
		{
			trace_t tr;

			Vector vecRelativeShootPosition;
			VectorSubtract(npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition);
			Vector vecMuzzle = ownerPos + vecRelativeShootPosition;
			Vector vecShootDir = npcOwner->GetActualShootTrajectory(vecMuzzle);

			// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
			AI_TraceHull(vecMuzzle, vecMuzzle + vecShootDir * (10.0f * 12.0f), Vector(-24, -24, -24), Vector(24, 24, 24), MASK_NPCSOLID, NULL, &tr);

			if (tr.fraction != 1.0f)
				bResult = false;
		}
	}

	return bResult;
}

int CWeaponRPG::WeaponRangeAttack1Condition(float flDot, float flDist)
{
	// Ignore vertical distance when doing our RPG distance calculations
	CAI_BaseNPC* pNPC = GetOwner()->MyNPCPointer();
	if (pNPC)
	{
		CBaseEntity* pEnemy = pNPC->GetEnemy();
		Vector vecToTarget = (pEnemy->GetAbsOrigin() - pNPC->GetAbsOrigin());
		vecToTarget.z = 0;
		flDist = vecToTarget.Length();
	}

	if (flDist < min(m_fMinRange1, m_fMinRange2))
		return COND_TOO_CLOSE_TO_ATTACK;

	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return 0;

	// See if there's anyone in the way!
	CAI_BaseNPC* pOwner = GetOwner()->MyNPCPointer();
	ASSERT(pOwner != NULL);

	if (pOwner)
	{
		// Make sure I don't shoot the world!
		trace_t tr;

		Vector vecMuzzle = pOwner->Weapon_ShootPosition();
		Vector vecShootDir = pOwner->GetActualShootTrajectory(vecMuzzle);

		// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
		AI_TraceHull(vecMuzzle, vecMuzzle + vecShootDir * (10.0f * 12.0f), Vector(-24, -24, -24), Vector(24, 24, 24), MASK_NPCSOLID, NULL, &tr);

		if (tr.fraction != 1.0)
		{
			return COND_WEAPON_SIGHT_OCCLUDED;
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}

void CWeaponRPG::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector	muzzlePoint;
		QAngle	vecAngles;

		muzzlePoint = GetOwner()->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		Vector vecShootDir = npc->GetActualShootTrajectory(muzzlePoint);

		// look for a better launch location
		Vector altLaunchPoint;
		if (GetAttachment("missile", altLaunchPoint))
		{
			// check to see if it's relativly free
			trace_t tr;
			AI_TraceHull(altLaunchPoint, altLaunchPoint + vecShootDir * (10.0f * 12.0f), Vector(-24, -24, -24), Vector(24, 24, 24), MASK_NPCSOLID, NULL, &tr);

			if (tr.fraction == 1.0)
			{
				muzzlePoint = altLaunchPoint;
			}
		}

		VectorAngles(vecShootDir, vecAngles);

		CMissile* pNewMissile = CMissile::Create(muzzlePoint, vecAngles, GetOwner()->edict());
		pNewMissile->SetGracePeriod(0.5);

		pOperator->DoMuzzleFlash();

		WeaponSound(SINGLE_NPC);
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif