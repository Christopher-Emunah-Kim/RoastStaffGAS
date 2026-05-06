# Session State — Active
> PreCompact 훅이 자동 갱신. 압축 후 컨텍스트 복원에 사용.

## Current Task
(없음 — 세션 시작 시 업데이트)

## Progress
### PLAN_EnemyHitMontage_v1.0 ✓ 전체 완료
- [x] HitMontage EditDefaultsOnly 방식 전환 + ABP 구성 (6c2406d1a, 1f8fa07f9, bd76571ee)
- [x] AttackMontage 타입별 자식 클래스 추가 (ShockwaveMontage, SpreadProjectileMontage, ProjectileMontage, ChargeMontage, AttackMontage)
- [x] AnimTick 거리 임계값 조정 (Near 3000, Far 5000, FarAnim 0.05s)
- [x] ABP_Melee/Ranged/Elite/Boss + AM_*_Hit + AM_*_Attack 에셋 14개 (bd76571ee)

## Key Decisions
### HitMontage/AttackMontage (2026-05-06)
- DT data-driven(TSoftObjectPtr) 방식 → EditDefaultsOnly(BP 직접 할당)로 전환
  이유: 에너미 타입별 DT row 구분 불필요, KnockdownMontage와 일관성
- 히트스탑(CustomTimeDilation=0) 주석처리 유지 — 몽타주 재생 테스트 후 제거 여부 결정
  이유: 히트스탑 활성 시 DeltaTime=0 → 몽타주 동결 충돌
- AttackMontage: base class가 아닌 각 자식 클래스에 배치
  이유: MeleeEnemy는 투사체 없고, EliteEnemy는 공격 2종(투사체+돌진)으로 타입별 상이

## Files In Progress
(없음 — 모두 커밋 완료)

## Next Session 추천
- 소서리스 스킬 6종 PLAN (P3) — 호크아이 완료 다음 단계
- 인게임 HUD 교체 + 입력 리바인딩 (P3)
- 에너미 BP별 KnockdownMontage 할당 [P2] — 에디터 작업만
- 히트스탑 코드 최종 제거 여부 결정 (EnemyBaseCharacter.cpp 주석처리 구간)

## Open Questions
(없음)
