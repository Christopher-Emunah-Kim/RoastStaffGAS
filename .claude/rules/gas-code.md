# GAS 코드 규칙
> 적용 경로: Source/**/GAS/**, Source/**/Abilities/**, Source/**/AttributeSets/**
> CODE 단계에서 GAS 파일 작업 시 반드시 준수

## 필수 규칙

### 속성(Attribute) 접근
- **속성 직접 수정 금지** — 반드시 Gameplay Effect 통해서만
- `PreAttributeChange()` → 클램핑 전용
- `PostGameplayEffectExecute()` → 반응 처리 (사망, 스탠 등)
- Base value vs Current value 구분: 수정자는 Current에 적용

### GA 트리거
- `SendGameplayEventToActor` 사용 — `TryActivateAbilityByHandle` 아님
- `ActivateAbility()` / `EndAbility()` 생명주기 완결 필수
  - EndAbility() 누락 시 다음 발동 차단
- 취소/중단 경로 반드시 처리 (OnCancelled 델리게이트)
- Cost/Cooldown은 `CommitAbility()` 으로 원자적 적용

### GAS 소유권 패턴 (프로젝트 고정)
- Player ASC → PlayerState 소유 (사망 후에도 유지)
- Enemy ASC → Enemy Actor 직접 소유 (사망 시 소멸)

### UPROPERTY GC 추적 (2회 지적 — 반드시 확인)
- UObject* / Actor* 멤버변수 → `UPROPERTY()` 강참조 필수
- `FGameplayAbilitySpec.SourceObject` → 약참조(Weak), 런타임 데이터 저장 불가
- 런타임 데이터 오브젝트는 `UPROPERTY()` 강참조로 유지

### 하드코딩 금지 (3회 연속 지적 — 최우선)
- 모든 수치(damage, health, cooldown, cost, duration, range) → DataTable 참조
- GE는 data-driven (Blueprint data-only subclass) — C++ 하드코딩 금지
- 예외: 0, 1, -1 같은 수학적 상수

### Gameplay Tags
- 계층 구조 준수: `State.Dead`, `Ability.Combat.Slash`, `Effect.Buff.Speed`
- Tag 문자열 직접 비교 금지 → FGameplayTag / FGameplayTagContainer 사용
- 새 Tag는 .ini 또는 데이터 에셋에 중앙 등록

### USTRUCT
- DataTable 연동 USTRUCT는 최소 동작 보장 기본값 필수 (SR 1회 지적)

### switch 문
- 모든 case 블록 끝: `break;` / `return;` / `[[fallthrough]]` 중 하나 명시 (SR 1회 지적)

## 코드 작성 전 체크리스트
- [ ] 새 속성 → DataTable에 row 추가했는가
- [ ] 새 Tag → .ini에 등록했는가
- [ ] EndAbility() 호출 경로 모두 커버했는가
- [ ] UPROPERTY() 누락 멤버 없는가
- [ ] 수치 하드코딩 없는가
