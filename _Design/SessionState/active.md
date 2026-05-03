# Session State — Active
> PreCompact 훅이 자동 갱신. 압축 후 컨텍스트 복원에 사용.

## Current Task
(없음 — 세션 종료)

## Progress
### PLAN_Hawkeye_Skills_v1.0
- [x] MODULE-1: DT 스키마 + GA 헬퍼 (2026-04-27)
- [x] MODULE-2: 백스텝샷 LaunchCharacter (2026-04-27)
- [x] MODULE-3: 버스트애로우 RadialAoE (2026-04-30)
- [x] MODULE-4: 체인트랩 ChainTrapVortexActor (2026-05-03)
  - fbafe9985 feat(chain-trap)
  - 3ddca93d1 fix(backstep): DisableMovement+LaunchCharacter 충돌 버그
  - 718aca3ac data(hawkeye): DT + 에셋 + 몽타주
- [x] MODULE-5: 애로우레인 (7baba80e8, 7a75bde6a) 2026-05-04
- [x] MODULE-6: 오토마톤 (8adcab3e1, 50bcf157e) 2026-05-04
- [ ] MODULE-7/8: 스나이프 충전샷 + ChargeGaugeWidget (P1)
- [ ] MODULE-9: 에디터 작업 DT 행 + GE BP (P2)

### PLAN_CharacterMeshApply_v1.0
- [x] PLAN + CODE + COMMIT (3624c6080) — completed 이동 완료

## Key Decisions
### ChainTrapVortexActor (2026-05-03)
- SpawnFX 타이밍: OnSystemFinished 콜백 대신 SpawnFXDuration EditDefaultsOnly 딜레이 타이머
  → 루핑 FX 지원, 디자이너가 BP에서 직접 조정 가능
- 데미지+기절 GE 분리: Duration GE에 ExecCalc 혼재 금지 원칙 확립
  → GE_Hawkeye_ChainTrap_Damage(Instant) + GE_Stun(Duration) 분리
- PullTick: LaunchCharacter bZOverride=true, Z=0으로 바닥 뚫림 방지

### BackstepShot 버그 (2026-05-03)
- StartSkillWithMontage → DisableMovement → LaunchCharacter 무시
- 해결: ExecuteEffect_BackstepShot에서 LaunchCharacter 직전 SetMovementMode(MOVE_Falling)

### ApplyCharacterMesh (2026-05-03)
- DT_CharacterStatic Mesh/AnimBP SoftPtr → LoadSynchronous → GetMesh() 적용
- InitializePlayer: ApplyCharacterStats → ApplyCharacterMesh → InitDefaultWeapon 순서

## Files In Progress
(없음 — 모두 커밋 완료)

## Next Session 추천
- MODULE-7: 스나이프 C++ — GA_CharacterSkill_Charge + ARSPlayerController Input Released 바인딩
- MODULE-8: ChargeGaugeWidget + RSHUDWidget ShowChargeGauge/HideChargeGauge API

## Open Questions
- BP 투사체 NiagaraComp 회전 — bRotationFollowsVelocity or NiagaraComp 로컬 회전 미확인 [P1]
