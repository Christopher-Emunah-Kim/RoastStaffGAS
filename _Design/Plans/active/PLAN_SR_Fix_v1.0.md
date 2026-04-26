# PLAN_SR_Fix_v1.0
> SR_SkillRefactor_v1.md HIGH+MED 항목 수정
> 생성: 2026-04-26 | 임시 플랜

## GOAL
SR 지적 HIGH/MED 항목 수정

## FILES
- Source/RoastStaffGAS/Private/GAS/Abilities/GA_Base.cpp
- Source/RoastStaffGAS/Private/Objects/GroundEffect/GroundEffectActor.cpp
- Source/RoastStaffGAS/Private/Objects/GroundEffect/PullVortexActor.cpp
- Source/RoastStaffGAS/Private/Objects/Projectile/BaseProjectile.cpp
- Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
- Source/RoastStaffGAS/Public/GAS/Abilities/GA_CharacterSkill.h

## MODULES
- MODULE-1: HIGH-1 CommitAbility (GA_Base.cpp)
- MODULE-2: MED-1 KHS_DEBUG → KHS_INFO (GroundEffectActor.cpp, PullVortexActor.cpp)
- MODULE-3: MED-3 FSkillEffectInitData UPROPERTY (RuntimeDataStructs.h)
- MODULE-4: MED-4 GA_CharacterSkill 헤더 주석 (GA_CharacterSkill.h)
