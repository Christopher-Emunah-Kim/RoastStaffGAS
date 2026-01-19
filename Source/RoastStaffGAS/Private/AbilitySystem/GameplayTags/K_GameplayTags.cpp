// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayTags/K_GameplayTags.h"

namespace KTags
{
	//어빌리티 태그
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot, "Ability.Combat.BasicShoot");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_BasicShoot_Cooldown, "Ability.Combat.BasicShoot.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball, "Ability.Skill.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball_Cooldown, "Ability.Skill.Fireball.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash, "Ability.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash_Cooldown, "Ability.Movement.Dash.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Jump, "Ability.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Jump_Cooldown, "Ability.Movement.Jump.Cooldown");
	
	//상태 태그
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burning, "State.Debuff.Burning");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Dashing, "State.Dashing");
	UE_DEFINE_GAMEPLAY_TAG(State_Jumping, "State.Jumping");
	
	//데이터 태그
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	
	//GC 태그
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Burn, "GameplayCue.Combat.Burn");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Launch, "GameplayCue.Combat.Fireball.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Impact, "GameplayCue.Combat.Fireball.Impact");
	
	//GE 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_HitCheck, "Event.Montage.HitCheck");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Death, "Event.Combat.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_TakeDamage, "Event.Combat.TakeDamage");
	
	
	
	
	
	
}