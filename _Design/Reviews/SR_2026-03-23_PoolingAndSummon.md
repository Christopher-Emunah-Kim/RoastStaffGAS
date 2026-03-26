# [SENIOR REVIEW 결과] - 풀링 시스템 및 소환형 스킬 통합

> 리뷰 일자: 2026-03-23
> 리뷰어: Claude Opus (Senior Reviewer)
> 대상 커밋: 7aa50ec (오브젝트 풀링 구현 및 기존 베이스에 적용) ~ 6066ef4 (소환형 스킬 공통 로직 추출)
> 관련 기획서: PLAN_POOLING_v1.0, PLAN_SUMMONSKILL_v1.0, 스킬 시스템 기획 v1.4

---

## 리뷰 대상 파일

| 파일 | 역할 |
|------|------|
| PoolableInterface.h | 풀링 인터페이스 (순수 가상) |
| PoolingSubsystem.h/.cpp | UWorldSubsystem 기반 오브젝트 풀 관리 |
| RSGameMode.h/.cpp | 풀 예열 호출 |
| BaseSummonObject.h/.cpp | 소환 장판 오브젝트 (IPoolableInterface 구현) |
| BaseProjectile.h/.cpp | 투사체 베이스 (IPoolableInterface 구현) |
| GA_SummonBase.h/.cpp | 소환형 GA 추상 베이스 |
| GA_SummonAutoTarget.h/.cpp | 자동 타겟팅 소환 자식 |
| GA_SummonAimedField.h/.cpp | 에임 장판형 소환 자식 |
| GA_Base.cpp | 모든 GA의 베이스 |

---

### 통과 항목

- IPoolableInterface 설계: OnPoolActivate/OnPoolDeactivate 순수 가상 2개만으로 미니멀하게 정의. ISP 원칙 준수.
- Object Pool 패턴 적용이 정확함. SpawnPooledActor의 풀-우선/폴백-스폰 구조가 명확.
- BeginPlay -> OnPoolDeactivate 워밍업 패턴: 예열 스폰된 액터가 자동 비활성 상태로 시작하는 설계가 올바름.
- ReturnToPool에서 ActiveActors.Contains 이중 반납 방어 로직 존재.
- ReturnAllActiveActors의 Snapshot 패턴: 순회 중 컬렉션 수정 방지를 위한 배열 복사가 적절.
- GA_SummonBase의 Template Method 패턴: DetermineSummonLocation PURE_VIRTUAL + HandleActiveMode 가상 함수 설계가 OCP를 잘 준수.
- GA_Base::ActivateAbility final 잠금으로 공통 캐싱을 강제하는 설계가 올바름.
- ProjectileMovementComponent 풀링 시 SetUpdatedComponent(GetRootComponent()) 복구 처리 올바름.
- VFXComp bAutoActivate=false 설정으로 풀링 라이프사이클과 정합.
- FTimerHandle + ReturnToPool 패턴이 SetLifeSpan 대비 풀링에 적합.
- LoadRequiredClass/LoadOptionalClass 템플릿 헬퍼로 SoftPtr 로딩 패턴을 일관화.
- 데이터 드리븐 아키텍처: GDS에서 모든 스킬 데이터를 조회하여 GA에서 사용. 하드코딩 최소화.
- 모든 if문 중괄호 규칙 준수.
- EndAbility에서 EQS->OnSummonAbilityEnded 호출로 쿨타임 타이머 재시작 보장.
- BaseSummonObject::ApplyGameplayEffectToArea의 HitActors TSet 중복 방지 처리.

---

### 수정 권고

| 파일 | 라인 | 관점 | 내용 | 심각도 |
|------|------|------|------|--------|
| PoolingSubsystem.h | 46 | 메모리 안전성 | **ActorPool TMap에 UPROPERTY() 누락.** 비활성 풀 액터에 대한 GC 강참조가 없어 가비지 컬렉터가 풀링된 액터를 수거할 수 있음. ActiveActors에는 UPROPERTY()가 있지만, ActorPool에 있는 비활성 액터는 GC 루트에서 도달 불가. PLAN_POOLING_v1.0에서도 "GC 강참조 필수"로 명시했으나 ActorPool 쪽은 누락됨. `UPROPERTY()` 추가 필수. | **Critical** |
| GA_Base.h | 60 | 컨벤션/하드코딩 | `SPAWN_OFFSET = 200.f`가 const 멤버로 하드코딩. CLAUDE.md "하드코딩 금지" 규칙 위반. DataTable 또는 최소한 UPROPERTY(EditDefaultsOnly)로 외부화 필요. | High |
| GA_SummonBase.cpp | 170-181 | 엣지 케이스 | `CheckIsActiveSlot`의 무한 for 루프 `for (int32 i = 0; ; ++i)`. GetSlotData가 null 반환으로 종료 조건을 삼고 있으나, 슬롯 데이터가 예상과 다르게 구성되면 무한 루프 위험. 명시적 상한 또는 컨테이너 크기 기반 루프로 변경 권고. | High |
| BaseSummonObject.cpp | 71-136 | 코드 가독성/계층 분리 | `ApplyGameplayEffectToArea`가 50줄 이상의 단일 함수로, 영역 탐색 + 적대 필터링 + DamageGE 적용 + StatusGE 적용 4개 책임이 혼재. Composed Method 패턴으로 하위 함수 분리 권고 (예: FilterEnemiesInRadius, ApplyDamageToTarget, ApplyStatusToTarget). | Medium |
| GA_SummonAutoTarget.cpp | 22 | 기획서 정합성 | 적 없을 때 `CachedInstigator->GetActorLocation() + Forward * 1000.f`로 캐릭터 전방 1000유닛에 소환. 기획서(스킬 시스템 기획 v1.4)에는 "사거리 내 적이 없는 경우 스킬 발동을 생략하고 쿨타임 타이머를 재시작"이라고 명시. 현재 코드는 발동을 생략하지 않고 전방에 소환하므로 기획서와 불일치. | High |
| BaseSummonObject.cpp | 54 | 풀링 정합성 | `OnPoolDeactivate`에서 `GetWorldTimerManager().ClearTimer(LifetimeHandle)`를 사용하지만, `ClearAndInvalidateTimerHandle`이 더 안전. ClearTimer는 핸들을 무효화하지 않아, 이미 클리어된 핸들에 대해 IsTimerActive 등의 후속 체크가 잘못된 결과를 줄 수 있음. BaseProjectile도 동일. | Medium |
| PoolingSubsystem.cpp | 8-31 | 엣지 케이스 | `InitializePool`에서 Count <= 0 체크 없음. 음수 Count가 들어오면 for 루프가 실행되지 않아 기능적 문제는 없지만, 의도하지 않은 호출을 경고할 방어 코드가 필요. | Low |
| BaseSummonObject.cpp | 71-136 | 인덴테이션 | ApplyGameplayEffectToArea 내부에서 탭/스페이스 혼용으로 들여쓰기 불일치. 75-76행은 6칸 스페이스, 90행은 탭 사용. 코드베이스 전체 일관성 필요. | Low |
| GA_SummonBase.h | 31 | 컨벤션 | `PURE_VIRTUAL` 매크로 뒤에 세미콜론이 없음. 컴파일은 되지만, PURE_VIRTUAL 매크로의 UE 사용 관례상 세미콜론을 붙이는 것이 일반적. | Low |

---

### 개선 제안

- **PoolingSubsystem - Pool Reserve**: `InitializePool`에서 `Pool.Reserve(Count)` 호출로 TArray 재할당 방지. 현재는 Add를 Count번 호출하면서 내부적으로 여러 번 재할당될 수 있음.

- **PoolingSubsystem - 풀 크기 모니터링 API**: 런타임에 풀 상태를 확인할 수 있는 `GetPoolStats()` 같은 디버그 함수 추가. 풀 고갈 빈도를 모니터링하면 적절한 예열 수량을 튜닝하기 좋음.

- **BaseProjectile::OnHit - 비활성 상태 방어**: OnPoolDeactivate 후에도 OnComponentHit 델리게이트가 바인딩된 상태로 남아있음. 충돌이 비활성화되어 실제로 호출될 가능성은 낮지만, OnHit 진입 시 `bIsActive` 같은 가드 추가를 고려.

- **BaseSummonObject - GE 적용과 OnPoolActivate 순서**: 현재 SpawnPooledActor 내부에서 OnPoolActivate(가시화) 후, GA에서 InitSummon(GE 적용)이 호출됨. PLAN_POOLING_v1.0에서는 "OnPoolActivate 시 check(bInitialized)"를 언급했으나 실제 코드에서는 bInitialized 체크가 OnPoolActivate에 없음. 의도적 생략이라면 기획서 업데이트 권고.

- **GA_SummonBase::SetSummonData - InstigatorASC 생명주기**: `InitData.InstigatorASC = GetOwnerASC()`로 ASC 포인터를 InitData 구조체에 저장. 소환 오브젝트가 Lifetime 동안 살아있는 동안 인스티게이터가 사망하면 dangling pointer 위험. TWeakObjectPtr 사용 또는 적용 시점에 IsValid 체크 추가 고려.

- **GA_SummonAimedField::DetermineSummonLocation - ZeroVector 반환 시**: AimLoc이 ZeroVector일 때 경고만 하고 그대로 반환. GA_SummonBase::OnAbilityActivated에서 ZeroVector면 EndAbility하므로 기능적 문제는 없지만, 기획서에서는 "마지막으로 캐싱된 AimLocation 기준으로 소환"이라고 명시. 현재는 그냥 종료됨.

- **ApplyGameplayEffectToArea - Context에 Instigator 설정 누락**: BaseSummonObject의 GE 적용 시 `MakeEffectContext()`만 호출하고 `Context.AddInstigator()` 미호출. BaseProjectile에서는 `Context.AddInstigator(GetInstigator(), this)`를 호출. Damage attribution이나 킬 크레딧 추적 시 문제될 수 있음.

---

### Gemini 리뷰 요약

Gemini 리뷰에서 Claude 리뷰와 중복되지 않는 추가 지적 사항:

- **OnHit 바인딩 관리**: BaseProjectile::BeginPlay에서 OnComponentHit.AddDynamic이 한 번만 호출되는데, OnPoolDeactivate 시 바인딩 해제 여부가 불명확. 풀 반환 상태에서 OnHit이 트리거될 가능성은 충돌 비활성화로 낮지만, 명시적 관리가 더 안전.

- **OverlapMulti 최적화**: ApplyGameplayEffectToArea에서 ECC_Pawn 채널로 OverlapMulti를 사용하는데, FCollisionQueryParams를 설정하여 자기 자신(소환 오브젝트)을 무시하도록 하면 불필요한 순회를 줄일 수 있음.

- **EQS 네이밍 혼란 가능성**: GA_SummonBase::EndAbility에서 사용하는 `EQS` 변수명이 UE의 Environment Query System(EQS)과 혼동될 수 있음. 실제로는 EquipmentSubsystem이므로 네이밍은 현재 `EQS`가 아닌 올바른 축약으로 보이나, 코드 리뷰에서 혼란 소지가 있음을 인지.

---

### 종합 평가

| 항목 | 점수 | 비고 |
|------|------|------|
| 패턴 적합도 | 4/5 | Object Pool + Template Method 적절. CheckIsActiveSlot 무한 루프 패턴은 개선 필요 |
| 코드 가독성 | 3/5 | 전반적으로 명확하나 ApplyGameplayEffectToArea의 50줄 단일 함수, 인덴테이션 불일치 |
| 메모리 안전성 | 2/5 | ActorPool UPROPERTY() 누락은 Critical. InstigatorASC dangling 위험도 존재 |
| 기획서 정합 | 3/5 | 대부분 일치하나 "적 없으면 발동 생략" 규칙 불일치, OnPoolActivate bInitialized 체크 누락 |
| 컨벤션 준수 | 3/5 | SPAWN_OFFSET 하드코딩, 인덴테이션 혼용. 나머지는 양호 |

**총평**: 풀링 시스템의 아키텍처 설계와 소환형 스킬 통합 구조는 견고합니다. Template Method 패턴을 통한 GA 계층 설계, IPoolableInterface의 미니멀한 정의, BeginPlay 워밍업 패턴 등은 교과서적으로 잘 구현되어 있습니다. 다만 **ActorPool의 UPROPERTY() 누락은 프로덕션 환경에서 GC에 의한 풀 액터 소실로 이어질 수 있는 Critical 이슈**이므로 즉시 수정이 필요합니다. 또한 자동 타겟팅 소환에서 적이 없을 때의 동작이 기획서와 불일치하므로 기획 담당자와 확인 후 코드 또는 기획서를 정정해야 합니다.
