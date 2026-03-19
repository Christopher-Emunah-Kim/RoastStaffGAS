// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tags/RSGameplayTags.h"


namespace RSTags
{
	//어빌리티 태그
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Projectile, "Ability.Skill.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Summon, "Ability.Skill.Summon");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot, "Ability.Combat.BasicShoot");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot_Cooldown, "Ability.Combat.BasicShoot.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash, "Ability.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash_Cooldown, "Ability.Movement.Dash.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball, "Ability.Skill.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball_Cooldown, "Ability.Skill.Fireball.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Iceball, "Ability.Skill.Iceball");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Thunder, "Ability.Skill.Thunder");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Poison, "Ability.Skill.Poison");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Shadow, "Ability.Skill.Shadow");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Summon_Sword, "Ability.Skill.Summon.Sword");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Summon_Lightning, "Ability.Skill.Summon.Lightning");
	
	//팀 태그
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, "Team.Player");
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy");
	
	//상태 태그
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burn, "State.Debuff.Burn");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Freeze, "State.Debuff.Freeze");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Shock, "State.Debuff.Shock");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Slow, "State.Debuff.Slow");
	UE_DEFINE_GAMEPLAY_TAG(State_Slot_Active, "State.Slot.Active");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burning, "State.Debuff.Burning");
	UE_DEFINE_GAMEPLAY_TAG(State_Dashing, "State.Dashing");
	
	//데이터 태그
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_EXP, "Data.EXP");
	
	//GC 태그
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Burn, "GameplayCue.Combat.Burn");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Launch, "GameplayCue.Combat.Fireball.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Impact, "GameplayCue.Combat.Fireball.Impact");
	
	//GE 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Death, "Event.Combat.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_TakeDamage, "Event.Combat.TakeDamage");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Equipped, "Event.Weapon.Equipped");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Unequipped, "Event.Weapon.Unequipped");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Fire, "Event.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Fire_Projectile, "Event.Weapon.Fire.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Fire_Summon, "Event.Weapon.Fire.Summon");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Defense, "Event.Weapon.Defense");
	UE_DEFINE_GAMEPLAY_TAG(Event_LevelUp, "Event.LevelUp");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_HitCheck, "Event.Montage.HitCheck");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Spawned, "Event.Enemy.Spawned");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Killed,  "Event.Enemy.Killed");
	
	
}