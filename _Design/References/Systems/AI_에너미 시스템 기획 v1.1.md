# [🟩 Solo Project] (9) AI/에너미 시스템 기획 v1.1

No: 428
주제: C++, Self
난이도: ⭐⭐
최종수정: 2026년 3월 2일 오후 1:53
작성일: 2026년 3월 2일 오전 10:35
Keyword: 기획

# 💡 INDEX

# 💡 AI/에너미 시스템 상세 기획서

## 핵심 역할

> 
> 
> 
> 
> - 에너미 캐릭터의 AI 행동 패턴을 정의하고, 타입별 공격 처리 로직을 담당한다.
> - 스폰 매니저로부터 초기화 요청을 받아 에너미 인스턴스를 설정하고, 전투 상태에 진입시킨다.
> - 에너미는 GE를 수신하는 쪽(피격)으로서 ASC와 BaseAttributeSet을 보유하며,
> 공격하는 쪽으로서는 GA 없이 충돌 또는 투사체 액터를 통해 직접 플레이어 ASC에 GE를 적용한다.
> - 보스 에너미는 일반 에너미 구조를 상속하되, 다단계 페이즈와 강화된 행동 패턴을 추가로 정의한다.
> 
> **핵심 책임**
> 
> - 적 캐릭터: AI 이동, 공격 처리(근접/원거리), 사망 처리, GE 수신을 담당한다.
> - 적 투사체 : 원거리 타입 에너미가 발사하는 단순 이동 액터. GAS와 무관하며, 플레이어 충돌 시 직접 GE를 적용한다.
> - 보스 캐릭터 : 페이즈 전환, 특수 패턴을 추가로 담당한다.
> - AI컨트롤러 : BehaviorTree를 통해 에너미의 이동·공격·상태 전환을 제어한다.
> 

---

## 다른 시스템과 관계

| 연관 시스템 | 관계 방향 | 설명 |
| --- | --- | --- |
| 게임데이터 서브시스템 | 에너미 → GDS | 초기화 시 DT_Enemy에서 스탯 조회 |
| 캐릭터 시스템 | 에너미 ← 캐릭터 | 적 캐릭터는 플레이어/적 공통 베이스 캐릭터 상속.  공통 초기화·사망 처리·GE 수신 로직을 공유 |
| 스폰 매니저 | 스폰 → 에너미 | 에너미 스폰 시 EnemyID와 초기 이동 방향을 전달. 에너미는 이를 받아 초기화 |
| 스테이지 시스템 | 에너미 → 스테이지 | 사망 시 EnemyID와 처치 이벤트를 스테이지 시스템에 전달 |
| 스킬 시스템(플레이어) | 스킬 → 에너미 | 플레이어 투사체가 충돌 시 에너미 ASC에 데미지 GE를 적용 |
| 캐릭터 시스템(플레이어) | 에너미 → 플레이어 | 충돌 또는 적 투사체가 플레이어 ASC에 직접 GE를 적용 |

---

# 💡 기능 명세 및 상세 규칙

| 기능 구분 | 상세 기능 |
| --- | --- |
| 에너미 초기화 | 스폰 시 EnemyID로 GDS 조회 → AS 기본값 설정 → AI 행동 시작 |
| Chase AI 행동 | 플레이어를 향해 지속 추격 → 공격 범위 진입 시 근접 충돌 데미지 적용 |
| Ranged AI 행동 | 일정 거리 유지 → 쿨타임마다 투사체 발사 |
| Elite AI 행동 | 케이스 기반. 광역 충격파 특수 공격 추가 |
| Boss AI 행동 | 다단계 페이즈 전환, 페이즈별 강화 패턴 |
| 에너미 투사체 처리 | 투사체가 플레이어와 충돌 시 GE 직접 적용 후 소멸 |
| 사망 처리 | 공통 베이스 캐릭터가 공통 사망 프로세스 실행. 이후 스테이지 시스템에 이벤트 전달 |

---

### 에너미 초기화 상세 규칙

> 
> 
> 
> 
> - 스폰 매니저가 에너미 액터를 스폰한 직후, EnemyID를 전달하여 초기화를 요청한다.
> - 초기화 프로세스
>     
>     1. ABaseCharacter 공통 초기화 프로세스를 실행한다. (ASC 초기화 → GDS 스탯 조회 → AS 기본값 설정 → 패시브 GE 적용)
>     
>     2. DT_Enemy의 AIType을 기반으로 BehaviorTree 에셋을 결정하고 UEnemyAIController에 설정한다.
>     
>     3. 스폰 매니저로부터 전달받은 초기 이동 방향(플레이어 위치)을 Blackboard에 저장한다.
>     
>     4. AI 행동을 시작한다.
>     
> - 예외처리
>     - EnemyID가 DT_Enemy에 존재하지 않는 경우 → 로그 출력 후 크래시
>     - AIType에 해당하는 BehaviorTree 에셋 로드 실패 시 → 로그 출력 후 크래시
>     - 초기 이동 방향 데이터가 전달되지 않은 경우 → 스폰 위치에서 가장 가까운 방향으로 플레이어를 탐색하여 이동 시작
>     

---

### Chase AI 행동 상세 규칙

> 
> 
> 
> 
> - Chase 타입은 플레이어를 향해 직선으로 추격하다가, 공격 범위 내에 진입하면 충돌 데미지를 적용하는 가장 단순한 패턴이다.
> - 행동 루프
>     
>     1. 처음 스폰 시 플레이어 위치(Blackboard)를 목표로 이동을 시작한다.
>     
>     2. 실시간으로 플레이어의 현재 위치를 갱신하여 추격 방향을 보정한다.
>     
>     3. 플레이어와의 거리가 공격범위(AttackRange) 이하로 진입하면 충돌 판정을 수행한다.
>     
>     4. 충돌 판정 성공 시 플레이어에게 데미지를 적용한다.
>     
>     - ASC에 AttackDamage 값을 담은 데미지 GE를 직접 적용한다.
>     - GE적용시 DT_Enemy_StatusEffect에서 해당 ENemyID와 ApplyContext(Melee)에 해당하는 행 조회
>     - 조회된 GE가 있는경우 ASC에 추가적용. 조회결과가 없을 경우엔 데미지만 적용
>     
>     5. 쿨다운 타임 동안 동일 플레이어에게 중복 데미지를 적용하지 않는다.(AttackCooldown)
>     
>     6. 쿨타임 종료 후 3번부터 반복한다.
>     
> - 예외처리
>     - 플레이어가 사망한 경우 → 이동을 중단하고 Idle 상태로 전환한다.
>     - 플레이어 ASC가 유효하지 않은 경우 → 데미지 적용을 무시하고 경고 로그를 출력한다.
>     - 에너미가 맵 경계에 도달한 경우 → 이동을 중단하고 제자리에서 대기한다.
>     

---

### 원거리 AI 행동 상세 규칙

> 
> 
> 
> 
> - 원거리(Ranged) 타입은 플레이어와 일정 거리를 유지하면서 투사체를 발사하는 패턴이다.
> - 투사체는 GAS와 무관한 단순 이동 액터이며, 플레이어 충돌 시 직접 GE를 적용한다.
>     - GE적용시 DT_Enemy_StatusEffect에서 해당 ENemyID와 ApplyContext(Projectile)에 해당하는 행 조회
>     - 조회된 GE가 있는경우 ASC에 추가적용. 조회결과가 없을 경우엔 데미지만 적용
>     
> - 행동 루프
>     
>     1. 플레이어와의 거리가 사격 가능 범위(PreferredRange)미만이면 플레이어 반대 방향으로 후퇴한다.
>     
>     2. 플레이어와의 거리가 사격 가능 범위 이상 최대 공격거리(MaxAttackRange) 미만이면 이동을 멈추고 공격 준비 상태가 된다.
>     
>     3. 플레이어와의 거리가 최대 공격 거리 이상이면 플레이어 방향으로 이동하여 범위 내로 진입한다.
>     
>     4. 공격 준비 상태에서 공격 쿨다운 타이머가 만료되면 플레이어 방향으로 투사체를 발사한다.
>     
>     5. 4번부터 반복한다.
>     
> 
> - 적 발사 투사체(AEnemyProjectile) 동작 규칙
>     - 발사 시 플레이어의 현재 위치를 기준으로 방향 벡터를 계산하여 직선으로 이동한다. (유도 없음)
>     - 지정된 속도(ProjectileSpeed)로 이동하며, 생명주기(ProjectileLifetime) 경과 시 소멸한다.
>     - 플레이어와 충돌하면 플레이어 ASC에 AttackDamage 값을 담은 데미지 GE를 직접 적용하고 소멸한다.
>     - 맵 경계를 벗어나면 즉시 소멸한다.
>     - 소멸 시 오브젝트 풀로 복귀한다.
> 
> - 예외처리
>     - 플레이어가 사망한 경우 → 이동을 중단하고 Idle 상태로 전환한다. 날아가던 투사체는 Lifetime 만료까지 유지된다.
>     - 플레이어 ASC가 유효하지 않은 경우 → GE 적용을 무시하고 경고 로그를 출력한다.
>     - 투사체 스폰 실패 시(오브젝트 풀 고갈) → 경고 로그를 출력하고 해당 발사 사이클을 스킵한다.
>     

---

### Elite AI 행동 상세 규칙

> 
> 
> 
> 
> - Elite 타입은 Chase 행동을 기반으로 하되, 주기적으로 광역 충격파 특수 공격을 추가로 사용한다.
> - 충격파는 에너미 위치를 중심으로 일정 반경 내의 플레이어에게 피해를 주는 즉발 범위 공격이다.
> 
> - 행동 루프
>     1. 일반 적의 Chase 행동 루프와 동일하게 플레이어를 추격하며 근접 데미지를 적용한다.
>     2. 특수 공격 타이머(ShockwaveCooldown)가 별도로 동작한다.
>     3. 특수 공격 타이머가 만료되면 Chase 이동을 일시 중단하고 충격파 선딜레이(PrepareTime) 동안 대기한다.
>         - 선딜레이 동안 경고 이펙트를 에너미 주변에 표시하여 플레이어에게 범위를 예고한다.
>     
>     4. 선딜레이 종료 후 충격 범위(ShockwaveRadius) 내의 플레이어에게 데미지 GE(ShockwaveDamage)를 직접 적용한다.
>     
>     - GE적용시 DT_Enemy_StatusEffect에서 해당 ENemyID와 ApplyContext(ShockWave)에 해당하는 행 조회
>     - 조회된 GE가 있는경우 ASC에 추가적용. 조회결과가 없을 경우엔 데미지만 적용
>     
>     5. Chase 행동을 재개하고  특수공격 타이머를 초기화한다.
>     
> - 예외처리
>     - 충격파 선딜레이 중 에너미가 사망하는 경우 → 충격파를 취소하고 사망 처리를 진행한다.
>     - 충격파 발동 시 플레이어 ASC가 유효하지 않은 경우 → GE 적용을 무시하고 경고 로그를 출력한다.
>     

---

### Boss AI 행동 상세 규칙

> 
> 
> 
> 
> - 보스는 적 공통 클래스를 상속한다.
>     - GE적용시 DT_Enemy_StatusEffect에서 해당 ENemyID와 ApplyContext(ShockWave)에 해당하는 행 조회
>     - 조회된 GE가 있는경우 ASC에 추가적용. 조회결과가 없을 경우엔 데미지만 적용
> - 보스는 체력(HP) 비율에 따라 2단계 페이즈로 나뉘며, 페이즈 전환 시 행동 패턴이 강화된다.
> - 보스는 AIType이 별도로 정의되지 않으며, DT_Enemy의 IsBoss 플래그로 구분한다.
> 
> - **페이즈 구조**
> 
> | 페이즈 | 진입 조건 | 행동 패턴 |
> | --- | --- | --- |
> | Phase 1 | 스폰 ~ HP 50% 초과 | Chase 기반 근접 공격 + 주기적 충격파 (Elite와 동일) |
> | Phase 2 | HP 50% 이하 | 이동속도 증가 + 충격파 쿨타임 단축 + 투사체 추가 발사 |
> - **페이즈 전환 규칙**
>     - CurrentHP가 MaxHP의 50% 이하로 감소하는 순간 페이즈 전환 트리거가 발동된다.
>     - 페이즈 전환 시 이동을 일시 중단하고 전환 연출(이펙트 + 로어 애니메이션)을 재생한다.
>     - 전환 연출 완료 후 Phase 2 행동 파라미터(MoveSpeed 증가, ShockwaveCooldown 감소)를 적용하고 행동을 재개한다.
>     - 페이즈 전환은 전투 중 1회만 발생한다. HP가 다시 50%를 초과하더라도 Phase 1으로 복귀하지 않는다.
>     
> - **Phase 2 투사체 발사 규칙**
>     - Phase 2 진입 후 별도의 투사체 발사 타이머가 추가로 동작한다.
>     - 타이머 만료 시 플레이어 방향으로 3방향 확산형 투사체를 발사한다. (중앙 + 좌우 15도)
>     - 투사체 동작 규칙은 원거리 AI의 투사체 발사 규칙을 따른다.
>     
> - **보스 처치 처리**
>     - 보스 사망 시 공통 베이스(ABaseCharacter)의 공통 사망 프로세스를 실행한다.
>     - 스테이지 시스템에 보스 처치 이벤트를 전달한다.
>     - 스테이지 시스템은 보스 처치를 수신하여 즉시 클리어 판정을 내린다. (스테이지 기획서 규칙 참조)
>     
> - **예외처리**
>     - 페이즈 전환 연출 중 추가 데미지로 사망 조건이 충족되는 경우 → 연출을 중단하고 즉시 사망 처리를 진행한다.
>     - 페이즈 전환 연출 중 플레이어가 사망하는 경우 → 연출을 중단하고 Idle 상태로 전환한다.
>     

---

# 💡 데이터 설계

## 필요 테이블 목록 및 관계

> 
> 
> - DT_Enemy → DT_Enemy_Ranged 참조 (EnemyID FK, Ranged 타입만 존재)
> - DT_Enemy → DT_Enemy_Elite 참조 (EnemyID FK, Elite/Boss 타입만 존재)
> - DT_Enemy → DT_Enemy_Boss 참조 (EnemyID FK, IsBoss == true만 존재)
> - DT_Enemy ← DT_Enemy_StatusEffect 참조 (1:N, ApplyContext로 공격 유형 분기)
> - DT_Enemy → DT_EnemyResource 참조
> - DT_Enemy → DT_String 참조

### DT_Enemy (기본 에너미 테이블)

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| EnemyID | STRING | PK, NOT NULL | 에너미 고유 ID |
| ResourceID | STRING | FK, NOT NULL | DT_EnemyResource 참조 |
| NameString | STRING | FK, NOT NULL | DT_String 참조 |
| MaxHP | FLOAT | NOT NULL | 최대 체력 |
| MoveSpeed | FLOAT | NOT NULL | 이동 속도 |
| AttackDamage | FLOAT | NOT NULL | 근접 공격 데미지 |
| AttackCooldown | FLOAT | NOT NULL | 공격 후 재공격까지 대기 시간 (초) |
| AttackRange | FLOAT | NOT NULL | 근접 공격 판정 범위 |
| AIType | ENUM | NOT NULL | Chase / Ranged / Elite (AI 행동 패턴 타입) |
| bIsBoss | BOOL | NOT NULL | 보스 여부  |
| BehaviorTree | STRING | NOT NULL | BT 에셋 경로 |
| SpawnWeight | FLOAT | NOT NULL | 웨이브 스폰 가중치 |
| DropEXP | INT | NOT NULL | 처치 시 지급 경험치 |
| DefaultEffects | ARRAY<STRING> |  | 에너미 고유 패시브 GE경로 목록 |

### DT_EnemyResource (에너미 리소스 테이블)

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| ResourceID | STRING | PK, NOT NULL | 리소스 고유 ID |
| Mesh | STRING | NOT NULL | 에너미 스켈레탈 메시 경로 |
| AnimBlueprint | STRING | NOT NULL | 에너미 애님 블루프린트 경로 |
| Effect | STRING |  | 에너미 전용 이펙트 경로 |
| DeathMontage | STRING | NOT NULL | 사망 애님 몽타주 경로 |
| HitMontage | STRING |  | 피격 애님 몽타주 경로 |

### **DT_Enemy_Ranged**

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| EnemyID | STRING | PK, NOT NULL | DT_Enemy 참조 |
| PreferredRange | FLOAT | NOT NULL | 사격 가능 범위 |
| MaxAttackRange | FLOAT | NOT NULL | 투사체 발사 최대 거리 |
| ProjectileSpeed | FLOAT | NOT NULL | AEnemyProjectile 이동 속도 |
| ProjectileLifetime | FLOAT | NOT NULL | AEnemyProjectile 최대 생존 시간 (초) |

### **DT_Enemy_Elite**

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| EnemyID | STRING | PK, NOT NULL | DT_Enemy 참조. 
Elite 타입과 Boss 에너미 모두 참조 |
| ShockwaveRadius | FLOAT | NOT NULL | 충격파 피해 반경 |
| ShockwaveDamage | FLOAT | NOT NULL | 충격파 기본 피해량 |
| ShockwaveCooldown | FLOAT | NOT NULL | 충격파 발동 간격 (초) |
| ShockwavePrepareTime | FLOAT | NOT NULL | 충격파 선딜레이 시간 (초) |

### **DT_Enemy_Boss**

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| EnemyID | STRING | PK, NOT NULL | DT_Enemy 참조. 
IsBoss == true인 에너미만 존재 |
| Phase2HPRatio | FLOAT | NOT NULL | 페이즈 전환 HP 비율 (기본 0.5) |
| Phase2MoveSpeedMult | FLOAT | NOT NULL | Phase 2 이동속도 배율 |
| Phase2ShockwaveCooldown | FLOAT | NOT NULL | Phase 2 충격파 쿨타임 (초) |
| Phase2ProjectileCooldown | FLOAT | NOT NULL | Phase 2 3방향 투사체 발사 간격 (초) |
| Phase2ProjectileSpeed | FLOAT | NOT NULL | Phase 2 투사체 이동 속도 |
| Phase2ProjectileLifetime | FLOAT | NOT NULL | Phase 2 투사체 최대 생존 시간 (초) |

### **DT_Enemy_StatusEffect** (1:N, 전 타입 공통)

| 컬럼명 | 타입 | 제약 | 설명 |
| --- | --- | --- | --- |
| RecordID | STRING | PK, NOT NULL | 행 고유 ID |
| EnemyID | STRING | FK, NOT NULL | DT_Enemy 참조 |
| ApplyContext | ENUM | NOT NULL | Melee / Projectile / Shockwave — 어떤 공격에 이 GE를 부여할지 |
| StatusEffectGE | STRING | NOT NULL | 적용할 GE 에셋 경로 |

## 런타임 사용 데이터

에너미 시스템이 인스턴스별로 관리하는 런타임 데이터 목록이다. 아래 데이터는 저장되지 않고, 해당 에너미 액터가 살아있는 동안에만 유지된다.

| **데이터 항목** | **타입** | **설명** |
| --- | --- | --- |
| 현재 AI 상태 | ENUM | Idle / Chase / Retreat / Attack / Shockwave / Dead |
| 공격 쿨타임 잔여 시간 | FLOAT | 마지막 공격 후 경과 시간. AttackCooldown과 비교하여 공격 가능 여부 판단 |
| 충격파 쿨타임 잔여 시간 | FLOAT | Elite/Boss 전용. 마지막 충격파 후 경과 시간 |
| 투사체 쿨타임 잔여 시간 | FLOAT | Ranged/Boss Phase2 전용. 마지막 투사체 발사 후 경과 시간 |
| 페이즈 전환 완료 여부 | BOOL | Boss 전용. Phase 2 전환 완료 여부. 중복 전환 방지 |
| 사망 처리 완료 여부 | BOOL | bIsDead 플래그. 중복 사망 이벤트 방지 (캐릭터 시스템 공통 규칙 참조) |

---

# 💡 UI/UX 상세

- 모든 적 클래스는 공통으로 체력바 UI 컴포넌트를 보유하고있다.
    - 적 체력바는 CurrentHP/MaxHP 비율로 실시간 갱신된다.
- 보스는 스폰되는 경우 HUD 화면 상단 중앙에 보스 체력바 UI를 표시한다.
    - 보스 체력바는 CurrentHP/MaxHP 비율로 실시간 갱신되며, 보스 50% 도달 하여 2페이즈 시작 시 색상이 변경되어 페이즈 전환에 대한 시각적 피드백을 전달한다.
    - 보스 사망후 체력바는 페이드 아웃 애니메이션과 함께 사라진다.

# 💡 **Base클래스 설계 아이디어**

> 
> 
> 
> 
> - 
> 

```cpp

```

```cpp

```

# 💡코드구현(code)