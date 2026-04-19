// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 
 */

/**
 * 프로젝트에서 사용할 모든 GameplayTags 정의
 * 
 * [네이티브 태그 사용목적]
 * - 컴파일 타임에 오타 검출
 * - 전역에서 일관된 참조
 * - 리팩토링 시 이름 변경 용이하게
 * 
* [사용법]
 * #include "AbilitySystem/Tags/RSGameplayTags.h"
 * 
 * // 태그 비교
 * if (ASC->HasMatchingGameplayTag(KTags::State_Debuff_Burn)) { ... }
 * 
 * // SetByCaller 데이터 전달
 * Spec.Data->SetByCallerMagnitude(KTags::Data_Damage, 10.f);
 */


namespace RSTags
{
	//===============================================
	// 어빌리티 태그(Ability.*)
	//===============================================
	// GA 식별 용도 및 상호작용 목적
	
	// 범용 GA 트리거 태그 — SendGameplayEvent로 발동
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Projectile);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Summon);
	
	//기본 공격 GA태그
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_BasicShoot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_BasicShoot_Cooldown);
	
	//스킬 GA태그
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Iceball);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Thunder);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Poison);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Shadow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Summon_Sword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Summon_Lightning);
	
	
	//Dash GA태그
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Dash_Cooldown);
	
	//===============================================
	// 팀 태그 (Team.*)
	//===============================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Player);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Enemy);
	
	//===============================================
	// 상태 태그(State.*)
	// State.Buff / State.Debuff / State.CC
	//===============================================
	// GE에서 GrantedTags 사용 목적. 
	// ASC->HasMatchingGameplayTag() 조회 목적
	
	// 사망
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	
	// 디버프 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Burn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Freeze);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Shock);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Slow);
	
	// 슬롯 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Slot_Active);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dashing);
	
	//===============================================
	// 데이터 태그(Data.*)
	//===============================================
	// GE에 고정값이 아니라 동적값 전달 시 사용
	// SetByCaller로 전달 시 키값으로 사용
	// GE스펙 핸들 생성 후 데미지값 부여시 사용
	// 이때 동적값 전달받을 GE에는 "SetByCaller" 타입으로 Modifier에 세팅 후
	// Data Tag에 Data.*타입 세팅 잊지말기
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_EXP);
	// SetByCaller — 플레이어 무기 기본 데미지 (ExecCalc 입력)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_WeaponBaseDamage);
	// SetByCaller — 에너미 공격 데미지 (ExecCalc 입력)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_EnemyAttackDamage);
	// SetByCaller — 패시브 GE 수치 주입 (TryAddPassive → GE Modifier)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_PassiveMagnitude);
	
	//===============================================
	// GC 태그(GameplayCue.*)
	//===============================================
	// GE의 GC 배열에 태그 추가. (자동 트리거)
	// ASC의 ExecuteGameplayCue(Tag, param)으로 발동 (수동 트리거)
	
	//화상 이펙트(Burn VFX)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_Burn);
	//Fireball 발사 이펙트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_Fireball_Launch);
	//Fireball 피격 이펙트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_Fireball_Impact);
	
	
	//===============================================
	// 속성 태그 (Element.*) — SpawnSkillFX 색상 분기용
	//===============================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Ice);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Thunder);

	//===============================================
	// CC 태그 (CC.*) — GE GrantedTags로 부여, PostGameplayEffectExecute에서 읽어 분기
	//===============================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CC_Knockdown); // 넉다운 — 래그돌 + 기립
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CC_Stun);      // 스턴   — 이동/공격 불가, Duration 동안 유지
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CC_Blind);     // 실명   — (연출 TODO) Duration 동안 유지

	//===============================================
	// 캐릭터 스킬 태그 (Skill.*)
	//===============================================
	// Q키 (슬롯1) 스킬 식별
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Character_Slot1);
	// E키 (슬롯2) 스킬 식별
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Character_Slot2);
	// 스킬 프리뷰 활성 상태 — ASC에 부여/제거로 IsPreviewActive() 대체 가능
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Character_Preview_Active);

	//===============================================
	// 패시브 태그 (Passive.*)
	//===============================================
	// 패시브 슬롯 포화 상태 — LevelUpSubsystem 카드 필터에 사용
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Passive_SlotFull);

	//===============================================
	// 이벤트 태그
	//===============================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Death);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_TakeDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Equipped);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Unequipped);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Fire_Projectile);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Fire_Summon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_LevelUp);
	
	//Hitcheck
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_HitCheck);
	// 에너미 이벤트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Spawned); 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Killed); 
	
	
}