# 세션 핸드오프 — 2026-04-13

## Worktree 정보
main 브랜치 직접 작업. worktree 없음.

## 파이프라인 진행 상태
CODE 완료 (M-3, M-4) → 빌드 성공 → EXPLAIN_IMPL 완료 → 세션 종료
미커밋 상태. TEST / SR / COMMIT 미진행.

## 마지막 작업 내용
**[FEATURE] PHASE-1 인게임 루프 완성** 중 M-3, M-4 동시 구현.

### MODULE-3 — 무기 자동발사 전환
- `EquipmentSubsystem`: `RequestManualFire` 제거, `FindNearestEnemy(SearchRadius)` 추가
- `StartAutoFire` 타이머: 최근접 적 탐색 → 타겟 없으면 스킵, SUMMON 타입은 `ASC->LocalInputConfirm()` 자동 호출
- `RSPlayerController`: `IA_Slot1/2/3` 바인딩 제거, `IA_Attack` → `OnConfirm` 재활용(빈 구현, M-5 용도), `IA_SkillQ/E` UPROPERTY 추가

### MODULE-4 — ExecCalc 데미지 공식
- `RS_DamageExecCalc` 신규 (Public/Private/GAS/Calculations/)
  - Source 팀 태그 분기: 플레이어→에너미 `BaseDmg×(1+ATK/100)×CritMult`, 에너미→플레이어 `max(1, EnemyDmg-DEF)`
- 새 SetByCaller 태그: `Data.WeaponBaseDamage` / `Data.EnemyAttackDamage`
- 기존 `Data_Damage` 주입 → 각 태그로 교체 (BaseProjectile, BaseSummonObject, 에너미 4종)
- 에디터 작업(사용자 직접): GE_Damage에 ExecCalc 교체 + GE_EnemyDamage 신규 생성 + 에너미 BP AttackGEClass 교체

## 미완료 사항

### [P0] BUG — 시작 무기 슬롯 미등록
- 인게임 진입 시 캐릭터 시작 무기가 Slot[0]에 등록되지 않아 자동발사 미작동
- 재현: 게임 시작 → 무기 슬롯 비어있음
- 예상 원인: BeginPlay 순서 충돌 (EquipWeapon 호출 타이밍)
- **다음 세션 진입점: `/debug`**

### [P1] M-5 — 캐릭터 스킬 시스템
- 신규: `SkillManagerSubsystem.h/.cpp`, `GA_CharacterSkill.h/.cpp`
- `RSPlayerController::OnConfirm()` 분기 구현 (IsPreviewActive 체크)
- M-7 패시브 슬롯과 병렬 가능

### 미커밋 파일
- `EquipmentSubsystem.h/.cpp`
- `RSPlayerController.h/.cpp`
- `RSGameplayTags.h/.cpp`
- `RS_DamageExecCalc.h/.cpp` (신규)
- `BaseProjectile.cpp`, `BaseSummonObject.cpp`
- `MeleeEnemy.cpp`, `EliteEnemy.cpp`, `BossEnemy.cpp`, `EnemyProjectile.cpp`
- `_Design/TODO.md`, `_Design/Learning/KnowledgeGaps.md`

## ⭐ Main으로 전달할 내용 (Worktree 작업 시 필수)
해당 없음 (main 직접 작업)

## 최근 변경 파일
| 2026-04-09 14:20:29 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\Source\RoastStaffGAS\Public\Core\RSGameMode.h` |
| 2026-04-09 14:20:41 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\Source\RoastStaffGAS\Private\Core\RSGameMode.cpp` |
| 2026-04-09 14:20:51 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\Source\RoastStaffGAS\Private\Core\RSGameMode.cpp` |
| 2026-04-09 14:25:08 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\Source\RoastStaffGAS\Public\UI\Enemy\BossHPBarWidget.h` |
| 2026-04-09 14:25:28 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\Source\RoastStaffGAS\Private\UI\Enemy\BossHPBarWidget.cpp` |
| 2026-04-09 14:25:41 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 14:30:53 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 14:32:40 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\Changesets\CHANGESET.md` |
| 2026-04-09 14:32:52 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\References\Systems\PoolingSystem_변경리포트_v2.0.md` |
| 2026-04-09 14:33:05 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\References\Systems\PoolingSystem_변경리포트_v2.0.md` |
| 2026-04-09 14:33:18 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\References\Systems\PoolingSystem_변경리포트_v2.0.md` |
| 2026-04-09 14:33:46 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\Changesets\CHANGESET.md` |
| 2026-04-09 14:44:10 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 15:04:12 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 15:04:21 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 15:04:43 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 15:05:21 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 15:05:35 | Edit | `C:\Users\KGA\Projects\RoastStaffGAS\_Design\TODO.md` |
| 2026-04-09 16:28:59 | Write | `C:\Users\KGA\Projects\RoastStaffGAS\README.md` |
| 2026-04-09 17:06:27 | Write | `C:\Users\KGA\Projects\RoastStaffGAS\.git\hooks\pre-commit` |

## 토큰 사용 체감
M-3+M-4 동시 진행으로 파일 수가 많았음(12개 이상). ExecCalc 원리 설명 + EXPLAIN_IMPL 세션에서 컨텍스트 소진 빠름. 다음 세션은 단일 모듈(M-5 or BUG) 진입 권장.

## 참고사항
- `SLOT_COUNT = 3` 확정 (계획서 SD1은 2였으나 세션 중 기획 변경)
- `IA_MouseAim` 제거됨 — PC PlayerTick의 `HandleMouseAim()`이 매 틱 커서 방향 갱신하므로 별도 IA 불필요
- `IA_Attack` 유지 중 — 현재는 빈 `OnConfirm()` 핸들러만 있음, M-5에서 `IsPreviewActive()` 분기 채울 것
- GE_EnemyDamage는 에디터에서 신규 생성 필요 (에너미 공용 단일 GE, 각 에너미 BP AttackGEClass를 이걸로 교체)
- `PLAN_Phase1_InGame_v1.0.md` active 상태 유지 (M-5/M-6/M-7 미완료)
