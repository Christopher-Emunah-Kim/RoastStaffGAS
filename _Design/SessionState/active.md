# Session State — Active
> PreCompact 훅이 자동 갱신. 압축 후 컨텍스트 복원에 사용.

## Current Task
[PLAN] SkillActivationType 2축 분리 리팩터링 + DT_CharacterSkill 통폐합

## Progress
### PLAN_CombatInfra_v1.0
- [x] PLAN
- [x] CODE (MODULE-1~CC + 스킬점검 ✓)
- [x] COMMIT (a7aff1033, 5ebab5d7d, b17d85a38, e958af9fc, 516e2b78e)
- [ ] SR / LEARN — PLAN_SkillSystemArch 이후로 연기

### PLAN_OutgameLobby3D_v1.0
- [x] PLAN
- [x] CODE
- [x] COMMIT (c6fd4228c, 9c6e1a738, 810436c81, a35bfc02b, 3a0265743)

### PLAN_SkillSystemArch_v1.0
- [x] PLAN (2026-04-21)
- [x] CODE (MODULE-1~6 ✓)
- [x] COMMIT (c74c3f3a3, 11407aa16, 9654d150e, 52a2610a4, 3e9b82688, ee0b6c1b2)
- [ ] 에디터 작업 (내일):
  - DT_CharacterSkill 각 스킬 행 SkillGEClass 열 GE 클래스 할당
  - BP_PullVortexActor 생성 + 도화가 5번 DT EffectActorClass 할당
  - 기존 스킬 + 도화가 5번 스킬 테스트
- [ ] SR / LEARN — 에디터 작업 + 테스트 완료 후

## Key Decisions
### CombatInfra
- EnemySpawner: NavMesh 투영 + LineTrace 바닥 검증 + Z ±150 QueryExtent 제한
- MODULE-3: ProjectileSpawn — SkillEffectID FK + GDS 복합 조회 (DRY)
- CC 시스템: GE GrantedTags (CC.Knockdown/Stun/Blind) → PostGameplayEffectExecute 분기
- SpawnPreview: bTeleportOnConfirm 플래그 + 스킬별 GA BP 분리 (Painter03/05)
  → EditDefaultsOnly는 BP 클래스 단위 공유 — 동작이 다른 스킬은 BP 분리 필수

### OutgameLobby3D
- LobbyCharInfoPanel: BindWidget + Show/Hide 패턴
- 아웃라인: CustomStencil 이진 마스크 (PostProcess)

### SkillSystemArch (2026-04-21)
- FCharacterSkillLevelData 삭제: 스킬레벨 시스템 미구현 — 필드 평탄화
- PullVortex 파라미터: EditDefaultsOnly on Actor BP (DT 컬럼화는 P3 TODO)
- SkillGEClass → DT_CharacterSkill 컬럼 이전 (MODULE-4 완료 후 에디터 할당)
- EffectActorClass 타입: TSoftClassPtr<AActor>로 확장 완료 (DataTableStructs + RuntimeDataStructs 모두)
- SD4 갱신 필요: DT_CharacterSkill이 TArray 제거로 이제 CSV 임포트 가능

## Files In Progress
(없음 — 모두 커밋 완료)

## Open Questions
- SM_Res_Sto_Chest_Wood_Worn_08.uasset (127MB) → .gitignore 처리 완료, 로컬에만 보관
- 클린 빌드 미완료 — 다음 세션 에디터 작업 전 확인 권장
