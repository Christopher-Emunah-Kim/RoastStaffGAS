// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tags/K_GameplayTags.h"

namespace KTags
{
	//어빌리티 태그
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Projectile1, "Ability.Skill.Projectile001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Summon1, "Ability.Skill.Summon001");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot1, "Ability.Combat.BasicShoot001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot_Cooldown1, "Ability.Combat.BasicShoot.Cooldown001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball1, "Ability.Skill.Fireball001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball_Cooldown1, "Ability.Skill.Fireball.Cooldown001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash1, "Ability.Movement.Dash001");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash_Cooldown1, "Ability.Movement.Dash.Cooldown001");
	
	
	//상태 태그
	UE_DEFINE_GAMEPLAY_TAG(State_Dead001, "State.Dead001");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burn001, "State.Debuff.Burn001");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Freeze001, "State.Debuff.Freeze001");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Shock001, "State.Debuff.Shock001");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Slow001Slow, "State.Debuff.Slow001");
	UE_DEFINE_GAMEPLAY_TAG(State_Slot_Active001, "State.Slot.Active001");
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burning001, "State.Debuff.Burning001");
	UE_DEFINE_GAMEPLAY_TAG(State_Dashing001, "State.Dashing001");
	
	//데이터 태그
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage001, "Data.Damage001");
	
	//GC 태그
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Burn001Burn, "GameplayCue.Combat.Burn001");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Launch001, "GameplayCue.Combat.Fireball.Launch001");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Impact001, "GameplayCue.Combat.Fireball.Impact001");
	
	//GE 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Death001, "Event.Combat.Death001");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_TakeDamage001, "Event.Combat.TakeDamage001");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Equipped001, "Event.Weapon.Equipped001");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Unequipped001, "Event.Weapon.Unequipped001");
	UE_DEFINE_GAMEPLAY_TAG(Event_LevelUp001, "Event.LevelUp001");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_HitCheck001, "Event.Montage.HitCheck001");
	
	
	
	
}