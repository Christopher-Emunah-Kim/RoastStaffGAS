// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 프로젝트에서 사용할 모든 GameplayTags 정의
 * 
 * [네이티브 태그 사용목적]
 * - 컴파일 타임에 오타 검출
 * - 전역에서 일관된 참조
 * - 리팩토링 시 이름 변경 용이하게
 * 
 * [사용법]
 * #include "GameplayTags/K_GameplayTags.h"
 * 
 * //태그비교시
 * if(ASC->HasMatchingGameplayTag(KTags::State_Debuff_Burning))
 * {
 *		Damage *= 2.f;
 * }
 * 
 * //SetByCaller 데이터 전달 시
 * Spec.Data->SetByCallerMagnitude(KTags::Data_Damage, 10.f);
 */


namespace KTags
{
	//===============================================
	// 어빌리티 태그(Ability.*)
	//===============================================
	// GA 식별 용도 및 상호작용 목적
	
	//Fireball GA태그
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball);
	//Fireball 쿨다운 상태 태그
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball_Cooldown);
	
	//===============================================
	// 상태 태그(State.*)
	// State.Buff / State.Debuff / State.CC
	//===============================================
	// GE에서 GrantedTags 사용 목적. 
	// ASC->HasMatchingGameplayTag() 조회 목적
	
	//화상 상태(Burning) <- GE_Burn이 화상상태 부여
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Burning);
	//사망 상태(Dead) <- 체력 0이면 부여
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	
	//===============================================
	// 데이터 태그(Data.*)
	//===============================================
	// GE에 고정값이 아니라 동적값 전달 시 사용
	// SetByCaller로 전달 시 키값으로 사용
	// GE스펙 핸들 생성 후 데미지값 부여시 사용
	// 이때 동적값 전달받을 GE에는 "SetByCaller" 타입으로 Modifier에 세팅 후
	// Data Tag에 Data.*타입 세팅 잊지말기
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	
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
	// GE 태그(GameplayEffect.*)
	//===============================================
	
	//Hitcheck
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_HitCheck);
	
	//사망 시
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Death);
	
	//데미지 입을 시
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_TakeDamage);
	
	
}