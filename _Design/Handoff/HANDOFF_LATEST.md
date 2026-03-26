# 세션 핸드오프 — 2026-03-25 16:10:05

## 파이프라인 진행 상태
> 완료 — PLAN → CROSS-REVIEW → CODE → TEST → SENIOR-REVIEW → LEARN → 커밋 (`323ebdd`)

## 마지막 작업 내용
**PIERCE HitType 구현** — 관통형 투사체 (최대 N회 관통, 회당 DamageDecay 감쇠, 중복 타격 방지)

### 구현 핵심
- `OnComponentBeginOverlap` + `ECR_Overlap` 기반 관통 감지 (OnHit 사용 시 ProjectileMovementComponent 정지)
- `SphereComp->IgnoreActorWhenMoving`으로 중복 타격 방지
- `PierceHitCount` 전용 카운터 — MoveIgnoreActors(발사자 포함)와 분리
- `bHasPierceFinished`로 ReturnToPool 이중 호출 방지
- EFFECT_THUNDER 스킬 HitType SINGLE → PIERCE 변경, 테스트 완료

### 디버깅 과정에서 발생한 주요 버그와 해결
1. **OnPoolDeactivate에 SetCollisionProfileName 호출** → BP 충돌 오버라이드 초기화로 SINGLE 타입 즉시 정지
   - 해결: 해당 라인 제거. 동일 클래스는 항상 동일 HitType이므로 리셋 불필요
2. **MoveIgnoreActors를 HitCount 카운터로 재활용** → 발사자 등록으로 오염, 첫 타격 30% 데미지
   - 해결: `PierceHitCount` 전용 멤버 분리

## 미완료 사항
### 하드코딩 수치 — 3회 연속 미해결 (최우선 교정 대상)
- `SPAWN_OFFSET = 200.f` (GA_Base.h)
- `HandleAreaHit` 거리 감쇠 분기값 (0.3f, 0.7f, 0.4f)
- `HandleArcType` LaunchAngle 클램프값 (-80, 80)
→ 다음 세션 초반에 DataTable/EditDefaultsOnly 이관 권장

### 다음 스프린트 후보
1. 하드코딩 수치 DataTable 이관
2. Stage 시스템 구현 (풀링 InitializePool 이관 대기 중)
3. 새 HitType 또는 MoveType 추가

## 최근 변경 파일
| 파일 | 내용 |
|------|------|
| BaseProjectile.h/cpp | PIERCE 완전 구현 + 버그 수정 |
| GA_ProjectileAttack.h/cpp | HandlePierceType 추가 |
| DataTableStructs.h | FSkillAttackHitTypeParamsPierce 기본값 추가 (PierceCount=1, DamageDecay=0.f) |
| RuntimeDataStructs.h | FProjectileInitData PierceCount/DamageDecay 추가 |
| ExternalSource/DT_Skill_Attack_HitType_Param_Pierce.csv | EFFECT_THUNDER 행 추가 |
| ExternalSource/DT_Skill_Attack_Common_Param_Data.csv | EFFECT_THUNDER HitType SINGLE→PIERCE |

## 토큰 사용 체감
세션이 컨텍스트 한도 초과로 요약 이어받기 형태로 시작됨. 디버깅 반복 과정(투사체 멈춤 현상 원인 분석)에서 컨텍스트 소비가 많았음.

## 참고사항
- 시니어 리뷰 기획서 정합 **2/5** — HitCount 오염이 원인. 다음 세션에서 데이터 구조 역할 혼용 주의
- 학습 리포트: `_Design/Learning/reports/LEARN_20260325_Pierce.md`
- 리뷰 파일: `_Design/Reviews/SR_2026-03-25_Pierce.md`
- CLAUDE.md 파이프라인 규칙 수정됨 (TEST/SENIOR-REVIEW/LEARN 단계 선택적으로 변경)
