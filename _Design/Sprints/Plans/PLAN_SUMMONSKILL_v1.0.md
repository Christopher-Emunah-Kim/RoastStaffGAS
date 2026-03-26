# PLAN_SUMMONSKILL_v1.0

> 작성일: 2026-03-17
> 관련 기획서: 스킬 시스템 기획 v1.4.md, PLAN_SPRINT1_v1.3.md
> 스프린트: SPRINT 1

---

## 구현 목표

소환형 스킬(MoveType == Summon) 2종을 구현한다.

| 종류 | 클래스 | 동작 |
|------|--------|------|
| 즉발 소환형 | GA_SummonAutoTarget | 자동/수동 모드 모두 SearchRange 내 가장 가까운 적 위치에 즉시 소환 |
| 에임 장판형 | GA_SummonAimedField | 자동 모드: 즉발 소환형과 동일. 수동(액티브) 모드: 마우스 에임을 따라 SummonPreviewObject가 이동, 좌클릭 시 에임 위치에 소환 |

---

## 영향 범위

### 수정/생성이 필요한 C++ 클래스

| 구분 | 클래스 | 파일 위치 |
|------|--------|-----------|
| 신규 | AGA_SummonBase | GAS/Abilities/Summon/GA_SummonBase.h/.cpp |
| 신규 | AGA_SummonAutoTarget | GAS/Abilities/Summon/GA_SummonAutoTarget.h/.cpp |
| 신규 | AGA_SummonAimedField | GAS/Abilities/Summon/GA_SummonAimedField.h/.cpp |
| 신규 | ASummonObject | Actors/SummonObject.h/.cpp |
| 신규 | ASummonPreviewObject | Actors/SummonPreviewObject.h/.cpp |

### 수정이 필요한 기존 파일

| 파일 | 변경 내용 |
|------|-----------|
| Data/EnumTypes.h | ESkillType: Projectile/Summon → Attack/Util. EMoveType에 Summon 값 추가 |
| Data/DataTableStructs.h | FSkillCommonParamsData.SkillType enum 수정. FSkillAttackCommonParamsData.Damage → Amount. FSkillAttackMoveTypeParamsSummon 신규 구조체 추가 |
| Data/RuntimeDataStructs.h | FWeaponSlotInstanceData 내 EquipData에 StatusEffectClass SoftRef 추가 |

### 수정/생성이 필요한 DataTable

| 테이블 | 변경 내용 |
|--------|-----------|
| Skill_Common_Resource_Data | StatusEffectClass 컬럼 추가 |
| Skill_Attack_Common_Params_Data | Damage 컬럼 → Amount 로 변경 |
| Skill_Attack_MoveType_Params_Summon | 신규 테이블 생성 |

### 추가 필요한 Gameplay Tag

없음 (기존 태그 구조 활용)

### 에디터 설정(BP/에셋) 필요 여부

- BP_SummonObject (ASummonObject 기반 Blueprint)
- BP_SummonPreviewObject (ASummonPreviewObject 기반 Blueprint)
- GA_SummonAutoTarget BP, GA_SummonAimedField BP
- Skill_Common_Resource_Data, Skill_Attack_MoveType_Params_Summon DataTable에 새 행 추가

---

## 함수 호출 흐름

### Q. 즉발 소환형은 어떤 순서로 실행되는가?

```
EquipmentSubsystem::RequestManualFire(AimLocation)         [수동] OR
EquipmentSubsystem::AutoFireTimer 만료                      [자동]
    │
    ▼
SendGameplayEventToActor → GA_SummonBase::ActivateAbility
    │
    ▼
GA_SummonBase::DetermineSummonLocation()  ← pure virtual
    │   GA_SummonAutoTarget에서 오버라이드
    ▼
내부 타겟 탐색: GetWorld()->OverlapMultiByChannel(SearchRange 내 Enemy 탐색)
    ├─ 적 발견 → 가장 가까운 적 위치 반환
    └─ 적 없음 → 발동 중단, EndAbility
    │
    ▼
ASummonObject::SpawnSummonObject(Location)
    │
    ▼
ASummonObject::BeginPlay
    ├─ DamageGE 즉시 적용 (ApplyGameplayEffectToTarget) ← 필수
    └─ StatusGE 적용 (StatusEffectClass != null이면 실행) ← 선택
    │
    ▼
Lifetime 경과 후 DestroyActor
    │
    ▼
GA_SummonBase::EndAbility → 쿨타임 타이머 재시작
```

### Q. 에임 장판형 수동 모드는 어떻게 다른가?

```
Num1/2/3 키 → 슬롯 액티브 전환 (GA_SummonAimedField가 장착된 슬롯)
    │
    ▼
GA_SummonAimedField::ActivateAbility
    │
    ▼
[수동(액티브) 모드 분기]
ASummonPreviewObject 스폰 (마우스 에임 위치에)
    │
    ▼
PlayerTick 매 프레임:
    PlayerController::CachedAimLocation → FGameplayAbilityTargetData_LocationInfo 페이로드
    → GA_SummonAimedField에 AimLocation 갱신 이벤트 전달
    → ASummonPreviewObject::SetActorLocation(NewLocation)
    │
    ▼
마우스 좌클릭 (OnShootStart)
    │
    ▼
ASummonPreviewObject 제거
GA_SummonAimedField::DetermineSummonLocation() → CachedAimLocation 반환
    │
    ▼
ASummonObject 스폰 → GE 적용 (즉발 소환형과 동일 이후 흐름)
```

### Q. AimLocation은 어떻게 GA에 전달되는가?

- `PlayerController::CachedAimLocation`은 `HandleMouseAim()`이 매 프레임 갱신.
- 수동 발사 시 `EquipmentSubsystem::RequestManualFire(CachedAimLocation)` 으로 전달.
- GA 내부에서 `FGameplayAbilityTargetData_LocationInfo`를 활용하여 Location 페이로드를 수신.

---

## 예외처리 목록

| 상황 | 처리 방식 |
|------|-----------|
| 자동 모드 소환형 — SearchRange 내 적 없음 | EndAbility 호출. 쿨타임 타이머 재시작. 로그 출력 |
| SummonObjectClass 로드 실패 | 로그 출력 후 EndAbility. 슬롯 유지 |
| 에임 장판형 수동 모드 — 마우스 에임 게임 화면 밖 | 마지막으로 캐싱된 AimLocation 기준으로 소환 + 경고 로그 |
| 수동 발사 중 슬롯 해제 | 진행 중인 GA 강제 종료. SummonPreviewObject 즉시 제거 |
| StatusEffectClass == null | StatusGE 적용 단계 생략 (선택 적용이므로 정상 흐름) |
| ASummonObject DamageGE 적용 실패 (타겟 ASC null) | 로그 출력. 오브젝트는 Lifetime 만료까지 유지 |

---

## GA 클래스 설계

### GA_SummonBase (추상 베이스)

```
ActivateAbility()
{
    FVector Location = DetermineSummonLocation();  ← pure virtual
    if (!Location.IsValid) { EndAbility(); return; }
    SpawnSummonObject(Location);
    EndAbility();
}

virtual FVector DetermineSummonLocation() = 0;  // 자식에서 구현
```

### GA_SummonAutoTarget

```
DetermineSummonLocation() override
{
    // SearchRange 내 Enemy 탐색
    // 적 없으면 FVector::ZeroVector (invalid 판단용 플래그)
    return NearestEnemyLocation;
}
```

### GA_SummonAimedField

```
ActivateAbility() override
{
    if (bIsActiveSlot)
    {
        // 프리뷰 오브젝트 스폰, 클릭 이벤트 대기
        SpawnPreview();
        WaitForConfirmCancel();
        // 확인 시 → DetermineSummonLocation 호출 후 소환
    }
    else
    {
        Super::ActivateAbility();  // 즉발 소환형과 동일
    }
}

DetermineSummonLocation() override
{
    return CachedAimLocation;  // PlayerController에서 가져옴
}
```

---

## ASummonObject 설계

```
BeginPlay()
{
    // 1. DamageGE 즉시 1회 적용 (필수)
    ApplyGameplayEffectToTarget(Target, DamageGEClass);

    // 2. StatusGE 지속 적용 (선택 — StatusEffectClass != null)
    if (StatusEffectClass)
    {
        ApplyGameplayEffectToTarget(Target, StatusEffectClass);
    }

    // 3. Lifetime 후 소멸
    SetLifeSpan(Lifetime);
}
```

## ASummonPreviewObject 설계

- 시각 전용 오브젝트. GE 없음. 충돌 없음.
- `SetActorLocation(FVector NewLocation)` 호출로 매 프레임 에임 위치 추적.
- `DestroyActor()` 호출로 즉시 제거 가능.

---

## [검토 결과]

- **기획서 일관성**: 스킬 시스템 기획 v1.4, PLAN_SPRINT1_v1.3과 일치.
- **누락된 예외처리**: 없음.
- **기획서 정정 필요 사항**: 스킬 시스템 기획 v1.3 → v1.4, PLAN_SPRINT1_v1.2 → v1.3 정정 완료.
