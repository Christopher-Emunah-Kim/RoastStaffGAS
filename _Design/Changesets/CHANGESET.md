# CHANGESET — RoastStaffGAS
> 에이전트용 코드 변화 추적.
> 세션 시작 시: PENDING_COMMIT 항목만 확인 (COMMITTED는 건너뜀).
> 최신 항목이 위에.

## READ_STRATEGY
```
세션 시작: status=PENDING_COMMIT 항목만 읽기
compact 트리거: COMMITTED 항목 5개 초과 시
compact 방법: COMMITTED 항목 → 별도 확인 없이 제거 (Plans/completed/에 이미 반영)
```

## PENDING_COMMIT

- date: 2026-04-19
  plan: PLAN_CombatInfra_v1.0 MODULE-6 + PLAN-2 MODULE-1,2
  status: PENDING_COMMIT
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/SkillManagerSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/SkillManagerSubsystem.cpp
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
      - Source/RoastStaffGAS/Public/UI/InGame/SlotContainerWidget.h
      - Source/RoastStaffGAS/Private/UI/InGame/SlotContainerWidget.cpp
    modified:
      - Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp
      - Source/RoastStaffGAS/Public/Data/EnumTypes.h
      - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/Objects/Projectile/BaseProjectile.h
      - Source/RoastStaffGAS/Private/Objects/Projectile/BaseProjectile.cpp
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - ExternalSource/DT_Character_Skill_Static_Data.csv
      - ExternalSource/DT_Skill_Attack_Common_Param_Data.csv
      - ExternalSource/DT_Skill_Common_Param_Data.csv
      - ExternalSource/DT_Skill_Common_Resource_Data.csv
    editor:
      - Content/Input/IA_Skill1~6.uasset (신규)
      - Content/Input/IMC_Player.uasset (숫자키 1~6 매핑)
      - Content/Blueprint/Player/BP_RSPlayerController.uasset (IA_Skill1~6 할당)
      - Content/UI/Ingame/WBP_SlotContainer.uasset (SkillSlotWidget_2~5 추가)
      - Content/GAS/GA/Character/GA_CharacterSkill_SpawnProjectile.uasset (신규)
      - Content/Data/Skill/DT_CharacterSkill.uasset (SkillEffectID 입력)
      - Content/Data/DT_Skill_Attack_Common_Param_Data.uasset (HOMING_BOUNCE 설정)
      - Content/Data/DT_Skill_Common_Param_Data.uasset (PAINTER_04/06 Lifetime 추가)
      - Content/Data/DT_Skill_Common_Resource_Data.uasset (PAINTER_04/06 ProjectileClass 추가)
  summary: "feat(PainterSkills): 환영의 문 텔레포트 + 콩콩이 HOMING_BOUNCE + SkillSlot 2→6 + 숫자키 1~6 바인딩"

- date: 2026-04-18
  plan: PLAN_CombatInfra_v1.0 MODULE-2 + MODULE-3
  status: PENDING_COMMIT
  files:
    modified:
      - Source/RoastStaffGAS/Public/GAS/Attributes/EnemyAttributeSet.h
      - Source/RoastStaffGAS/Private/GAS/Attributes/EnemyAttributeSet.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/EnemyBaseCharacter.h
      - Source/RoastStaffGAS/Private/Character/Enemy/EnemyBaseCharacter.cpp
      - Source/RoastStaffGAS/Public/Data/EnumTypes.h
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/GAS/Abilities/GA_CharacterSkill.h
      - Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp
  summary: "feat(CombatInfra): 피격 반응(넉백+히트스탑+이미시브) + ProjectileSpawn 타입 추가 (SkillEffectID FK 복합 조회 방식)"

- date: 2026-04-17
  plan: PLAN_CombatInfra_v1.0 MODULE-1
  commits: ["75ba1d80b", "6ecf988c7"]
  status: COMMITTED
  files:
    modified:
      - Config/DefaultGame.ini
      - Content/Blueprint/Enemy/Boss/BP_BossEnemy.uasset
      - Content/UI/Ingame/Enemy/WBP_BossHPBar.uasset
      - Source/RoastStaffGAS/Private/System/EnemySpawner.cpp
  summary: "fix(EnemySpawn): NavMesh 투영 + LineTrace 바닥 검증 + Z 범위 제한 + 보스 캡슐 충돌 Block 수정"

- date: 2026-04-15
  plan: PLAN_PassiveSlotUI_v1.0
  commits: ["0a9533b9b", "7c36f7fd3", "ab9d95674", "c2a58ec1a", "5b45d6edd", "ccb38598c"]
  status: COMMITTED

## COMMITTED

- date: 2026-04-15
  plan: PLAN_Phase1_InGame_v1.0
  commits: ["cc4ab96c4", "f0efdb971", "fd502de74", "9d449833e", "5cea35082"]
  status: COMMITTED
  summary: "refactor(GAS-Delegate) + fix(HP-Behavior) + feat(CharacterStat) + data(Passive) + chore(에디터): ASC delegate 일원화, HP 버그 수정, 스탯팝업 Base+Bonus 표시, 패시브 magnitude 원복"
  bugs_fixed:
    - GAS Multiplicative 공식 오진 — magnitude 0.2x(80%감소)→1.2x(20%증가) 원복
    - MaxHP StatUpgrade 적용 시 CurHP=MaxHP 회복 전용 처리
    - MaxHP 패시브 적용 시 CurHP 델타만큼 연동
    - 레벨업 카드 선택 후 스탯 창 미갱신 — ASC attribute delegate로 교체

- date: 2026-04-14
  plan: PLAN_CharacterStatPopup_v1.0
  commits: ["a6b407a34", "6a35704d7", "58de4f52b", "d2be49f02", "5d1ba8d47", "8b1e18e42"]
  status: COMMITTED
  summary: "fix(FloatingDamage) + data(Phase1-Editor) + feat(CharacterStat): 스탯 팝업 HUD 구현 + Phase1 에디터 완료"
  bugs_fixed:
    - FloatingDamageWidget PreWarm/스폰 클래스 이중 관리 제거

- date: 2026-04-14
  plan: PLAN_LevelUpCardPool_v1.0
  commits: ["54c0698f", "4865e460", "8dc709e02"]
  status: COMMITTED
  summary: "feat(LevelUp): 카드풀 확장 — StatUpgrade/PassiveAdd/WeaponUpgrade/WeaponNew 혼합 + 4장 선택"

---
<!-- 이전 항목들은 compact됨 (2026-04-14) -->

## FORMAT
```yaml
- date: YYYY-MM-DD
  plan: PLAN_[시스템명]_vX.X
  commit: null | "abc1234"
  files:
    modified: []
    created:  []
    deleted:  []
  summary: "한 줄 요약"
  status: PENDING_COMMIT | COMMITTED | REVERTED
  bugs_found: []
  bugs_fixed: []
```
