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
	UE_DEFINE_GAMEPLAY_TAG(Data_WeaponBaseDamage, "Data.WeaponBaseDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_EnemyAttackDamage, "Data.EnemyAttackDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_PassiveMagnitude, "Data.PassiveMagnitude");
	
	//GC 태그
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Burn, "GameplayCue.Combat.Burn");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Launch, "GameplayCue.Combat.Fireball.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Fireball_Impact, "GameplayCue.Combat.Fireball.Impact");
	
	//캐릭터 스킬 태그
	UE_DEFINE_GAMEPLAY_TAG(Skill_Character_Slot1, "Skill.Character.Slot1");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Character_Slot2, "Skill.Character.Slot2");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Character_Preview_Active, "Skill.Character.Preview.Active");

	//패시브 태그
	UE_DEFINE_GAMEPLAY_TAG(Passive_SlotFull, "Passive.SlotFull");

	//이벤트 태그
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

	// 속성 태그 — SpawnSkillFX 색상 분기
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire,    "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice,     "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Thunder, "Element.Thunder");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ancient, "Element.Ancient");

	// CC 태그 — GE GrantedTags 부여, PostGameplayEffectExecute 분기
	UE_DEFINE_GAMEPLAY_TAG(CC_Knockdown, "CC.Knockdown");
	UE_DEFINE_GAMEPLAY_TAG(CC_Stun,      "CC.Stun");
	UE_DEFINE_GAMEPLAY_TAG(CC_Blind,     "CC.Blind");
}