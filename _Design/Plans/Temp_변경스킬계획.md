# RoastStaffGAS — 캐릭터 스킬 구현 스펙
> Claude Code 전달용 / 작성일: 2026-04-17
> 마감: 2026-04-23 (6일)

---

## 0. 현재 코드베이스 전제

```
기존 스킬 타입 (ESkillActivationType):
  InstantAoE   — 캐릭터 위치 기준 즉발 AoE
  SelfBuff     — 자기 자신에게 GE 적용
  SpawnPreview — 프리뷰 액터 배치 후 확정

기존 투사체 계층 (ALRProjectileBase 파생):
  LRLinearProjectile   — 직선, 최초 충돌 즉시 처리
  LRExplodeProjectile  — 착탄 범위 폭발
  LRPierceProjectile   — 관통 N회
  LRArcProjectile      — 포물선 궤적
  LRHomingProjectile   — 타겟 추적

기존 GAS 패턴:
  - SetByCaller(Data.WeaponBaseDamage) 주입
  - ExecCalc에서 Source 팀태그로 방향 분기
  - SpawnSkillFX(FXClass, Location, Radius) — Niagara SetVariableFloat("Radius")

신규 추가 필요 항목 (3캐릭터 공통):
  (1) ESkillActivationType에 ProjectileSpawn 타입 추가
  (2) ESkillActivationType에 ChargeAndRelease 타입 추가 (스나이프 전용)
  (3) DT_CharacterSkill에 ElementTag(FGameplayTag) 컬럼 추가
      → SpawnSkillFX() 내에서 태그별 Niagara SetVariableLinearColor() 분기
  (4) 피격 반응 공통 구조:
      GE 수신 → PostExecute → GameplayEvent 발행
      → [피격 몽타주] + [LaunchCharacter() 넉백] + [TimeDilation 히트스탑 0.05초]
```

---

## 1. 도화가 (Painter / 동양풍 서포터 계열 딜러)

### 컨셉
동양풍 붓·묵법 FX. 스킬별 연출 키워드는 **붓·먹물·달·해·구슬**. 
기존 `SpawnPreview` + `SelfBuff` 패턴 비중이 높아 신규 로직 추가 없이 구현 가능한 캐릭터.

### 스킬 6개

#### Skill_Painter_01 — 흩뿌리기 (AoE Burst)
```yaml
타입: InstantAoE
설명: 주변 반경에 묵법 에너지 방사형 방출, 다수 에너미 넉백
연출:
  - 안에서 밖으로 방사형 확산 Niagara FX (먹빛 파티클)
  - 피격 에너미에게 먹물 튀김 Hit 파티클
로직:
  - ActivateAbility → SphereOverlap(캐릭터 위치, Radius=DT값)
  - 충돌 에너미 → ApplyGameplayEffect(데미지 GE)
  - 충돌 에너미 → LaunchCharacter(방사 방향, Force=DT값)
  - SpawnSkillFX(FX_Inkburst, Location, Radius)
재활용: 기존 InstantAoE 패턴 그대로
구현 난이도: 낮음
```

#### Skill_Painter_02 — 먹물세례 (Debuff Puddle)
```yaml
타입: SpawnPreview
설명: 전방에 먹물 장판 배치, 밟은 적에게 이동속도 감소 GE 지속 적용
연출:
  - 프리뷰: 반투명 원형 먹물 데칼
  - 확정 시: 바닥에 번지는 먹물 텍스처 Niagara 장판 생성
  - 적 Overlap 시: 발자국 파티클
로직:
  - SpawnPreview → LMB 확정 / RMB 취소
  - 확정 위치에 장판 Actor 스폰 (수명 DT값 초)
  - 장판 Actor OnComponentBeginOverlap → ApplyGameplayEffect(이속감소 GE)
  - OnComponentEndOverlap → RemoveGameplayEffect
재활용: 기존 SpawnPreview 패턴 + 장판 Actor (신규, 크레모아와 구조 공유 가능)
구현 난이도: 낮음~중간
```

#### Skill_Painter_03 — 범가르기 (Crescent Slash)
```yaml
타입: ProjectileSpawn (신규 타입)
설명: 전방 초승달 형태 참격 투사체 발사, 단일 타겟 즉시 폭발
연출:
  - 초승달 궤적 Niagara FX (흰빛 + 먹빛 혼합)
  - 착탄 시 파열 FX
로직:
  - SpawnProjectile(LRLinearProjectile 파생, 전방 방향)
  - Hit → ApplyGameplayEffect(데미지 GE, SetByCaller)
  - Hit → SpawnSkillFX(FX_SlashBurst, HitLocation)
재활용: LRLinearProjectile — 비주얼만 초승달 Niagara로 교체
구현 난이도: 낮음
```

#### Skill_Painter_04 — 해그리기 (Sun Buff)
```yaml
타입: SelfBuff
설명: 자신에게 공격력 증가 버프 GE 적용, 해 모양 황금빛 오라 연출
연출:
  - 캐릭터 주변 황금빛 원형 Niagara FX 루프 재생
  - 버프 만료 시 오라 소멸 FX
로직:
  - ApplyGameplayEffectToSelf(ATK증가 GE, Duration=DT값)
  - SpawnAttachedFX(FX_SunAura, 캐릭터 루트)
  - GE 만료 OnGameplayEffectRemoved → FX 제거
재활용: 기존 SelfBuff 패턴 그대로
구현 난이도: 낮음
```

#### Skill_Painter_05 — 환영의 문 (Blink Portal)
```yaml
타입: SpawnPreview
설명: 지정 위치로 즉발 순간이동, 출발·도착 지점 각각 FX
연출:
  - 프리뷰: 문 모양 게이트 데칼
  - 출발: 소멸 파티클 (먹빛)
  - 도착: 소환 파티클 (빛 번짐)
로직:
  - SpawnPreview → 위치 확정
  - SpawnSkillFX(FX_Disappear, CurrentLocation)
  - SetActorLocation(TargetLocation) (즉발)
  - SpawnSkillFX(FX_Appear, TargetLocation)
재활용: 기존 SpawnPreview 패턴 — SetActorLocation() 이동만 추가
구현 난이도: 낮음
```

#### Skill_Painter_06 — 콩콩이 (Triple Bounce Orb)
```yaml
타입: ProjectileSpawn (신규 타입)
설명: 발광 구슬이 적 사이를 최대 3회 튀어다니며 연쇄 공격
연출:
  - 발광 구슬 Niagara 투사체
  - 충돌 후 다음 타겟으로 호밍 이동 시 궤적 FX
  - 각 충돌마다 Hit 파티클
로직:
  - SpawnProjectile(LRHomingProjectile 파생)
  - Hit → ApplyGameplayEffect(데미지)
  - Hit → 현재 타겟 제외 반경 내 가장 가까운 적 탐색
  - 타겟 존재 && BounceCount < 3 → 새 HomingProjectile 스폰
  - BounceCount >= 3 → 투사체 풀 반환
재활용: LRHomingProjectile + 충돌 시 리다이렉트 로직 추가
구현 난이도: 중간
```

### 도화가 구현 우선순위
```
P0: 흩뿌리기 → 해그리기 (기존 타입, FX 연동 검증)
P1: 환영의 문 → 범가르기 (SpawnPreview+이동 / ProjectileSpawn 타입 검증)
P2: 먹물세례 → 콩콩이 (장판 Actor / 연쇄 호밍)
```

---

## 2. 소서리스 (Sorceress / 3속성 마법사)

### 컨셉
화염·냉기·번개 3속성. 스킬별 `ElementTag`로 FX 컬러 + GE 상태이상이 분기되는 구조.
**속성 태그 기반 분기가 GAS 설계 역량을 보여주는 핵심.**

### 속성 태그 → FX 컬러 매핑
```cpp
// SpawnSkillFX() 내 분기 예시
if (ElementTag == TAG_Element_Fire)   Color = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);  // 주황
if (ElementTag == TAG_Element_Ice)    Color = FLinearColor(0.3f, 0.8f, 1.0f, 1.0f);  // 청백
if (ElementTag == TAG_Element_Thunder)Color = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);  // 노랑
NiagaraComponent->SetVariableLinearColor("ElementColor", Color);
```

### 스킬 6개

#### Skill_Sorc_01 — 인페르노 (Fire DoT Zone)
```yaml
타입: SpawnPreview
속성: Fire (TAG_Element_Fire)
설명: 지정 위치에 화염 장판 생성, 밟은 적에게 지속 화염 데미지
연출:
  - 프리뷰: 붉은 원형 범위 데칼
  - 확정: 화염 Niagara 장판 (루프, ElementColor=주황)
  - 적 접촉 시: 불꽃 도트 파티클 Attach
로직:
  - SpawnPreview → 장판 Actor 스폰
  - OnComponentBeginOverlap → ApplyGameplayEffect(DoT GE, Period=1초, Duration=DT값)
  - ApplyGE는 중첩 없이 갱신 (EGameplayEffectStackingType::AggregateByTarget)
재활용: SpawnPreview + 장판 Actor (먹물세례와 동일 구조, 속성만 다름)
구현 난이도: 낮음~중간
```

#### Skill_Sorc_02 — 혹한의 부름 (Ice AoE Cast)
```yaml
타입: InstantAoE (캐스팅 딜레이 포함)
속성: Ice (TAG_Element_Ice)
설명: 1.5초 캐스팅 후 광역 냉기 폭발, 이동속도 감소 + 넉백
연출:
  - 캐스팅 중: 발 아래 서리 결정 성장 Niagara (점점 커짐)
  - 폭발: 빙결 파편 방사 FX (ElementColor=청백)
로직:
  - WaitDelay(1.5f) → SphereOverlap
  - 충돌 에너미 → ApplyGameplayEffect(데미지 + 이속감소 GE)
  - 충돌 에너미 → LaunchCharacter(방사 방향, Force=DT값)
  - SpawnSkillFX(FX_IceExplosion, Location, Radius)
재활용: InstantAoE + WaitDelay 선딜 추가
구현 난이도: 낮음
```

#### Skill_Sorc_03 — 라이트닝 볼텍스 (Chain Lightning)
```yaml
타입: ProjectileSpawn (신규 타입)
속성: Thunder (TAG_Element_Thunder)
설명: 번개 투사체 발사 → 착탄 시 주변 적에게 연쇄 번개 (최대 2회)
연출:
  - 투사체: 번개 궤적 Niagara LinTrail (ElementColor=노랑)
  - 착탄: 전기 방전 FX
  - 연쇄: 착탄 지점에서 2번째 적으로 번개 빔 Niagara
로직:
  - SpawnProjectile(LRLinearProjectile 파생)
  - Hit → ApplyGameplayEffect(데미지 + 감전 GE 1초)
  - Hit → 반경 내 다음 타겟 탐색 → ChainCount < 2 → 다시 SpawnProjectile
재활용: LRLinearProjectile + 연쇄 로직 (콩콩이와 구조 유사, 유틸 함수 공유 가능)
구현 난이도: 중간
```

#### Skill_Sorc_04 — 아이스 애로우 (Piercing Ice)
```yaml
타입: ProjectileSpawn (신규 타입)
속성: Ice (TAG_Element_Ice)
설명: 냉기 화살 직선 관통, 피격 적에게 이동속도 감소 스택 누적
연출:
  - 투사체: 얼음 결정 형태 Niagara (ElementColor=청백)
  - 관통 시: 빙결 파편 FX
로직:
  - SpawnProjectile(LRPierceProjectile, 관통횟수=DT값)
  - 각 Hit → ApplyGameplayEffect(이속감소 GE, 중첩 +1, Max=3)
재활용: LRPierceProjectile — 기존 관통 로직 그대로, GE만 교체
구현 난이도: 낮음
```

#### Skill_Sorc_05 — 종말의 날 (Meteor Drop)
```yaml
타입: SpawnPreview
속성: Fire (TAG_Element_Fire)
설명: 지정 위치에 대형 메테오 낙하, 고데미지 + 강넉백 (포트폴리오 핵심 연출)
연출:
  - 프리뷰: 화염 낙하 예고 원형 데칼 (깜빡임)
  - 낙하: 상공에서 화염 궤적 Niagara LinTrail
  - 착탄: 대형 충격파 FX + 화염 잔불 루프
로직:
  - SpawnPreview → WaitDelay(2.0f) 낙하 딜레이
  - 착탄 → SphereOverlap(Radius=DT값)
  - 충돌 에너미 → ApplyGameplayEffect(고데미지 GE)
  - 충돌 에너미 → LaunchCharacter(방사 방향, Force=강) + 히트스탑 0.05초
  - SpawnSkillFX(FX_MeteorImpact, Location, Radius)
재활용: SpawnPreview + WaitDelay + LaunchCharacter (혹한의 부름과 동일 구조, 스케일만 다름)
구현 난이도: 중간 (연출 완성도가 관건)
```

#### Skill_Sorc_06 — 점멸 (Blink Dash)
```yaml
타입: SelfBuff (이동 포함)
설명: 마우스 방향으로 즉발 이동, 이동 궤적에 전기 잔상
연출:
  - 출발 지점: 전기 소멸 파티클
  - 궤적: 번개 잔상 Niagara LinTrail (ElementColor=노랑)
  - 도착 지점: 전기 폭발 소형 FX
로직:
  - 마우스 방향 벡터 계산 → Normalize → * DashDistance(DT값)
  - SetActorLocation(TargetLocation) 즉발
  - SpawnSkillFX(FX_BlinkTrail, 궤적 중간점)
재활용: SelfBuff + SetActorLocation() — 환영의 문과 동일 이동 처리
구현 난이도: 낮음
```

### 소서리스 구현 우선순위
```
P0: 라이트닝 볼텍스 → 혹한의 부름 (속성 태그+FX 분기 구조 검증)
P1: 아이스 애로우 → 인페르노 (관통 GE / DoT 장판)
P2: 종말의 날 → 점멸 (핵심 연출 / 이동)
```

---

## 3. 호크아이 (Hawkeye / 원거리 궁수·투척)

### 컨셉
활·단검·폭발물 복합. **투사체 다양성이 가장 풍부한 캐릭터** — 기존 ProjectileBase 계층을 가장 잘 활용할 수 있음.
매(실버호크) 소환 아이덴티티 → `HawkGauge(0~100f)` 게이지 관리 로직 추가.

### 호크 게이지 설계
```cpp
// UPlayerAttributeSet 또는 USkillManagerSubsystem에 추가
float HawkGauge = 0.f;  // 0~100
// 래피드샷·아토믹 Hit 시 +GaugePerHit(DT값)
// 호크샷 사용 시 -100 (풀게이지 필요)
// 호크 게이지 UI: 기존 스킬 슬롯 옆 별도 게이지 위젯
```

### 스킬 6개

#### Skill_Hawk_01 — 래피드 샷 (Rapid Shot) ★ P0 최우선
```yaml
타입: ProjectileSpawn (신규 타입)
설명: 전방에 화살 5발 빠르게 연속 발사, 관통 가능
연출:
  - 화살 5발 0.1초 간격 연속 발사
  - 각 화살 궤적 Niagara LinTrail (흰빛)
  - 피격 시 Impact 스파크 파티클
로직:
  - for i in 0..4:
      WaitDelay(0.1f * i)
      SpawnProjectile(LRPierceProjectile, 전방 방향)
  - 각 Hit → ApplyGameplayEffect(데미지 GE, SetByCaller)
  - Hit → HawkGauge += GaugePerHit
재활용: LRPierceProjectile 그대로, 반복 발사 루프만 추가
구현 난이도: 낮음 ← 시작점으로 적합
```

#### Skill_Hawk_02 — 아토믹 애로우 (Atomic Arrow) ★ P0
```yaml
타입: ProjectileSpawn (신규 타입)
속성: Thunder (TAG_Element_Thunder)
설명: 번개 속성 투사체 발사, 착탄 시 폭발 + 감전 CC 부여
연출:
  - 투사체: 번개 궤적 Niagara (ElementColor=노랑)
  - 착탄: 전기 방전 폭발 FX
  - 피격 적: 감전 파티클 Overlay (적 위에 Attach)
로직:
  - SpawnProjectile(LRExplodeProjectile, 전방 방향)
  - Hit → SphereOverlap(Radius=ExplosionRadius)
  - 범위 내 에너미 → ApplyGameplayEffect(데미지 + 감전 GE, 2초)
  - Hit → HawkGauge += GaugePerHit
재활용: LRExplodeProjectile — 폭발 로직 그대로, 감전 GE + FX 색상 분기 추가
구현 난이도: 낮음
```

#### Skill_Hawk_03 — 호크 샷 (Hawk Shot / 실버호크)
```yaml
타입: ProjectileSpawn (신규 타입) + 게이지 조건
설명: HawkGauge 100 소모 → 매 Actor가 타겟 추적 후 고데미지 타격
연출:
  - 호크 게이지 100 미만 시 사용 불가 (UI 피드백)
  - 매 스켈레탈 메쉬 or StaticMesh Actor 호밍 이동
  - 타격 시 날개 충격파 FX + 히트스탑 0.05초
로직:
  - ActivateAbility 진입 시 HawkGauge >= 100 체크 → 미달 시 EndAbility
  - HawkGauge = 0 차감
  - 가장 가까운 에너미 타겟 탐색
  - SpawnProjectile(LRHomingProjectile 파생, HawkMeshActor 비주얼)
  - Hit → ApplyGameplayEffect(고데미지 GE) + LaunchCharacter(강넉백)
  - Hit → 풀 반환
재활용: LRHomingProjectile — Actor 비주얼 연동 + 게이지 조건만 추가
구현 난이도: 중간
```

#### Skill_Hawk_04 — 애로우 해일 (Arrow Shower)
```yaml
타입: SpawnPreview
설명: 지정 범위에 다수 화살 낙하, 화상 DoT 부여
연출:
  - 프리뷰: 원형 범위 데칼 (빨간 점선)
  - 확정: 상공에서 화살 Niagara 낙하 (SpawnRate, 방향 하향)
  - 피격 적: 화상 파티클 Overlay
로직:
  - SpawnPreview → WaitDelay(0.3f) → SphereOverlap
  - 충돌 에너미 → ApplyGameplayEffect(데미지 + 화상 DoT GE, Period=1초)
  - SpawnSkillFX(FX_ArrowRain, Location, Radius)
    → Niagara SpawnRate, Radius 파라미터 런타임 주입
재활용: SpawnPreview + WaitDelay (인페르노·종말의 날과 동일 구조)
구현 난이도: 중간
```

#### Skill_Hawk_05 — 크레모아 (Claymore Mine)
```yaml
타입: SpawnPreview
설명: 지뢰 Actor 설치, 에너미 접근 시 자동 폭발 + AoE
연출:
  - 설치: 지뢰 소형 StaticMesh Actor 스폰 + 금속 반짝임 Niagara
  - 폭발: 연기+화염 FX
로직:
  - SpawnPreview → 지뢰 Actor 스폰 (풀링 연동)
  - 지뢰 Actor: 대기 상태 (Tick 없음, 충돌만 감지)
  - OnComponentBeginOverlap (에너미만 감지) → 폭발 처리
    → SphereOverlap(폭발반경) → ApplyGameplayEffect(데미지 GE)
    → SpawnSkillFX(FX_Explosion) → 풀 반환
재활용: SpawnPreview + 대기 Actor (먹물세례 장판 Actor와 구조 유사, 클래스 분리)
구현 난이도: 중간
```

#### Skill_Hawk_06 — 스나이프 (Snipe)
```yaml
타입: ChargeAndRelease (신규 타입)
설명: 차징 후 장거리 관통 화살 발사, 차징 단계별 데미지 배율
연출:
  - 차징 중: 조준 UI 오버레이 (HUD 레이어) + 차징 게이지 바 표시
  - 차징 완료: 화면 약간 줌인 연출 (FOV 감소)
  - 발사: 긴 궤적 Niagara LinTrail (흰빛, 빠른 속도)
로직:
  - WaitInputPress(Q/E) → 차징 시작, 타이머 시작
  - WaitInputRelease 또는 MaxChargeTime 도달 → 발사
  - ChargeRatio = Clamp(ElapsedTime / MaxChargeTime, 0, 1)
  - DamageMultiplier = Lerp(0.5f, 2.0f, ChargeRatio)  ← DT값으로 조정
  - SpawnProjectile(LRPierceProjectile, 관통=3)
  - SetByCaller 주입 시 BaseDamage * DamageMultiplier 적용
  - 차징 중 TAG_Skill_State_Charging 태그 활성 → 타 스킬 사용 불가
재활용: LRPierceProjectile — 차징 로직(ChargeAndRelease 타입)만 신규
구현 난이도: 높음 ← 마지막에 착수
```

### 호크아이 구현 우선순위
```
P0: 래피드샷 → 아토믹 애로우 (ProjectileSpawn 타입 검증 + 속성 FX)
P1: 호크샷 → 애로우해일 (게이지 조건 / SpawnPreview+낙하)
P2: 크레모아 → 스나이프 (대기 Actor / ChargeAndRelease 신규 타입)
```

---

## 4. 전체 코드베이스 재활용 매핑

```
스킬                    재활용 투사체/패턴              신규 추가
─────────────────────────────────────────────────────────────────
[도화가]
흩뿌리기               InstantAoE                      없음
먹물세례               SpawnPreview + 장판Actor         장판Actor 클래스
범가르기               LRLinearProjectile               ProjectileSpawn 타입
해그리기               SelfBuff                         없음
환영의 문              SpawnPreview + SetActorLocation  없음
콩콩이                 LRHomingProjectile               충돌 후 리다이렉트 로직

[소서리스]
라이트닝 볼텍스        LRLinearProjectile               연쇄 타겟 탐색 로직
혹한의 부름            InstantAoE + WaitDelay           없음
아이스 애로우          LRPierceProjectile               없음
인페르노               SpawnPreview + 장판Actor         (먹물세례 공유)
종말의 날              SpawnPreview + WaitDelay         없음
점멸                   SelfBuff + SetActorLocation      (환영의 문 공유)

[호크아이]
래피드샷               LRPierceProjectile               반복 발사 루프
아토믹 애로우          LRExplodeProjectile              ElementTag FX 분기
호크샷                 LRHomingProjectile               HawkGauge 조건
애로우해일             SpawnPreview + WaitDelay         낙하 Niagara 파라미터
크레모아               SpawnPreview + 대기Actor          대기Actor 충돌 처리
스나이프               LRPierceProjectile               ChargeAndRelease 타입
```

---

## 5. 공통 신규 구현 항목 (우선순위 순)

```
[1] ProjectileSpawn 타입 추가 — ESkillActivationType
    → GA_CharacterSkill에 SpawnProjectile() 분기 추가

[2] ElementTag 컬럼 추가 — DT_CharacterSkill
    → SpawnSkillFX() 내 SetVariableLinearColor() 분기

[3] 피격 반응 공통 구조 구현 — EnemyBaseCharacter
    → GE PostExecute → GameplayEvent
    → 피격 몽타주 + LaunchCharacter 넉백 + TimeDilation 히트스탑

[4] 장판 Actor 공통 클래스 — ARS_GroundEffectActor
    → Duration, OverlapGEClass, FXClass를 DataTable에서 주입
    → 먹물세례·인페르노·애로우해일이 동일 클래스 재사용

[5] HawkGauge 상태 추가 — UPlayerAttributeSet 또는 USkillManagerSubsystem
    → Hit 시 누적 / 호크샷 사용 시 소모 / UI 연동

[6] ChargeAndRelease 타입 추가 — 스나이프 전용 (우선순위 낮음)
    → WaitInputPress → ChargeTimer → WaitInputRelease → DamageMultiplier
```

---

*작성: JARVIS / 기반 서치: 나무위키(도화가·소서리스·호크아이/스킬·운용 문서)*