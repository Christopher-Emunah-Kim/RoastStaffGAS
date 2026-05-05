# Session State — Active
> PreCompact 훅이 자동 갱신. 압축 후 컨텍스트 복원에 사용.

## Current Task
(없음 — 세션 시작 시 업데이트)

## Progress
### PLAN_Hawkeye_Skills_v1.0 ✓ 전체 완료
- [x] MODULE-1: DT 스키마 + GA 헬퍼 (2026-04-27)
- [x] MODULE-2: 백스텝샷 LaunchCharacter (2026-04-27)
- [x] MODULE-3: 버스트애로우 RadialAoE (2026-04-30)
- [x] MODULE-4: 체인트랩 ChainTrapVortexActor (fbafe9985) 2026-05-03
- [x] MODULE-5: 애로우레인 (7baba80e8, 7a75bde6a) 2026-05-04
- [x] MODULE-6: 오토마톤 (8adcab3e1, 50bcf157e) 2026-05-04
- [x] MODULE-7: 스나이프 C++ GA_CharacterSkill_Charge (4efe08474) 2026-05-04
- [x] MODULE-8: ChargeGaugeWidget UI (0eac6728c) 2026-05-04
- [x] MODULE-9: 에디터 작업 DT 행 + GE BP (ca97eb737) 2026-05-04

### PLAN_CharacterMeshApply_v1.0
- [x] PLAN + CODE + COMMIT (3624c6080) — completed 이동 완료

## Key Decisions
### GA_CharacterSkill_Charge (2026-05-04)
- TargetingType: AimPreview+ChargeAndRelease 2단계 복합
  → Skill6 버튼 → 프리뷰 → LMB 누름(ConfirmSkillPreview) → GA 발동 → 차징
- 몽타주 섹션 구조: Loop(차징 중) → "Shoot"(JumpToSection, 발사 시)
- PC가 IA_Attack Completed/Canceled → SendGameplayEvent 브릿지, GA는 WaitGameplayEvent로 수신
- Skill_State_Charging(상태 태그) / Skill_Event_ChargeRelease(이벤트 태그) 명확히 분리

### ChargeGaugeWidget (2026-05-04)
- NativeTick 기반 ElapsedTime/MaxChargeTime → ProgressBar Fill 갱신
- Ratio >= 0.8f → PerfectZoneColor(황색) 전환으로 퍼펙트 존 표시

## Files In Progress
(없음 — 모두 커밋 완료)

## Next Session 추천
- 소서리스 스킬 6종 PLAN (P3) — 호크아이 완료 다음 단계
- 인게임 HUD 교체 + 입력 리바인딩 (P3)
- BP 투사체 NiagaraComp 회전 버그 (bRotationFollowsVelocity or NiagaraComp 로컬 회전) [P1]

## Open Questions
- BP 투사체 NiagaraComp 회전 — bRotationFollowsVelocity or NiagaraComp 로컬 회전 미확인 [P1]
