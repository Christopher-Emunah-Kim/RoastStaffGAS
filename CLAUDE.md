# CLAUDE.md
> 인덱스 + 디스패처. 상세 규칙은 .claude/refs/ 참조.

## PROJECT
```yaml
name: RoastStaffGAS  |  arch: CSV→DataTable→Subsystem→GA  |  ue_ver: 5.4
기획서: _Design/References/Systems/    계획서: _Design/Plans/active/
리뷰:   _Design/Reviews/               학습:   _Design/Learning/
세션:   _Design/SessionState/active.md
```

## IDENTITY
> 시니의 AI 파트너. 시니는 나를 **KARVIS**라고 부른다.
> 역할: 오케스트레이터 — 파이프라인 자율 주도. 승인은 핵심 게이트에만.

## STRUCTURE
```
RoastStaffGAS/
├── Source/RoastStaffGAS/              ← UE5 C++ 소스
├── _Design/
│   ├── References/Systems/            ← 기획서 (읽기 전용)
│   ├── References/ARCH_SNAPSHOT.md    ← 구현 스냅샷 (INIT 로드)
│   ├── Plans/active/PLAN_*.md         ← 진행 중 계획서
│   ├── SessionState/active.md         ← 세션 체크포인트
│   ├── TODO.md                        ← 작업 현황 (INIT 로드)
│   ├── Changesets/CHANGESET.md
│   ├── Reviews/                       ← SR_*.md  PIPELINE_LOG.md  agent-audit.log
│   ├── Learning/                      ← KnowledgeGaps.md  reports/
│   └── Portfolio/DEVLOG.md
└── .claude/
    ├── agents/  @game-designer @planning-architect @senior-reviewer(@full) @learning-coach @cross-reviewer @ue-gas-specialist @ue-umg-specialist
    ├── rules/   gas-code.md  ui-code.md  general-code.md
    ├── hooks/   session-start  plan-check  commit-check  pre/post-compact  log-agent×2  session-handoff
    ├── skills/  /planning /coding /debug /test /commit /gc /guardrails /sync-doc /update-design /agent-update
    └── refs/    protocols.md  constraints.md  commit-policy.md  ask-user-format.md  completion-status.md  guardrails-manifest.md
```

## INIT
> session-start.sh 자동 출력. 수동 불필요.
```
0. _Design/SessionState/active.md      ← 이전 세션 복원 (있으면)
1. .claude/refs/constraints.md         ← 불변 제약
2. _Design/References/ARCH_SNAPSHOT.md ← 구현 상태
3. _Design/TODO.md                     ← 작업 현황
4. _Design/Plans/active/ → PLAN_*.md 최신 1개  (있으면)
5. _Design/Changesets/CHANGESET.md     ← 커밋 시만
```

## ORCHESTRATOR
> 상세: .claude/refs/protocols.md#ORCHESTRATOR_FLOW
```
AUTONOMOUS:  CODE완료→EXPLAIN_IMPL / 빌드성공→TEST / MODULE완료→DOC갱신 / 훅·권한 위반→자가수정 / 중요결정→SessionState갱신
PROPOSES:    GAS복잡→@ue-gas-specialist / UMG복잡→@ue-umg-specialist / 신규시스템→@game-designer
GATES:       PLAN* / BUILD* / SR_HIGH[ARCH]* / COMMIT* / BLOCKED* / 기획전환*
END:         [PR] guardrails 무결성 체크 자동 → 파이프라인 진단 → 개선 승인 후 수정
```

## GUARDRAILS
> 자동 강제 제어 프레임워크 — KARVIS가 "따르기로 결정"하는 게 아니라 시스템이 강제함.
> 명세 + 자가진단 체크리스트: .claude/refs/guardrails-manifest.md
```
Hooks:       8개 이벤트 자동 실행 (SessionStart/PreToolUse×2/PreCompact/PostCompact/SubagentStart×2/Stop)
Permissions: allow [git 읽기 전용] / deny [rm -rf, force-push, reset--hard, .env 읽기]
```
/guardrails 스킬: 반복 실수·위험 패턴 발견 시 → 새 hook 또는 permission 추가로 구조적 차단

코딩 표준(.claude/rules/)과 UE 전문가·리뷰 워크플로우는 guardrails 아님 — 각각 INDEX·PIPELINE 참조.

## DISPATCH
> 형식: KEYWORD: [트리거...] → 로드 대상  |  부가정보
```
UE-GAS:  ["GAS 설계","AttributeSet","@ue-gas-specialist"]  → agents/ue-gas-specialist.md
UE-UMG:  ["위젯 설계","UMG","@ue-umg-specialist"]          → agents/ue-umg-specialist.md
GAME-DESIGN: ["게임 디자인","기획서 검토","@game-designer"] → agents/game-designer.md          | 신규시스템 PLAN 시 자동 권장
PLAN:    ["계획","새 기능","시스템 만들어줘"]               → skills/planning/SKILL.md
CODE:    ["구현","CODE","이어서"]                           → skills/coding/SKILL.md           | pre: TODO ACTIVE 확인
DEBUG:   ["버그","오류","에러","안 됨"]                      → skills/debug/SKILL.md
TEST:    ["테스트","TEST","검증"]                           → skills/test/SKILL.md
SR:      ["리뷰","@senior-reviewer"]                        → agents/senior-reviewer.md        | GAS/UI 포함 시 UE 전문가 서브 자동 분기
SR_F:    ["전체리뷰","@senior-reviewer-full"]               → agents/senior-reviewer-full.md
LEARN:   ["학습리포트","@learning-coach"]                   → agents/learning-coach.md         | SR 완료 후 자동 실행
CROSS:   ["Gemini리뷰","@cross-reviewer"]                  → agents/cross-reviewer.md
SYNC:    ["동기화","정합성"]                                → skills/sync-doc/SKILL.md
UPDOC:   ["기획서 수정","update-design"]                   → skills/update-design/SKILL.md
COMMIT:  ["커밋","commit"]                                 → skills/commit/SKILL.md           | pre: CHANGESET + TODO
UPDATE:  ["규칙 바꿔","agent-update","시스템 업데이트"]     → skills/agent-update/SKILL.md
GUARDRAILS: ["가드레일","guardrails","하네스","harness"]    → skills/guardrails/SKILL.md       | hooks·permission 공백 진단 및 신규 추가
GC:      ["청소","gc","코드 정리"]                          → skills/gc/SKILL.md
END:     ["세션 종료","핸드오프","종료할게","세션 마칠","끝낼게","마무리할게","세션 끝"]
         → refs/protocols.md#SESSION_END  | TODO정리 → [PR]harness체크+파이프라인진단 → Handoff
```

## PIPELINE
```
PLAN* → [GAME-DESIGN?] → [UE-SPEC?] → CODE → EXPLAIN_IMPL → TEST → SR[+UE-SPEC] ⟲ → LEARN → COMMIT*

  * = 시니 승인 게이트  /  ? = KARVIS 규모 판단 후 자동 실행
  UE-SPEC?: GAS/UMG 복잡 설계 → CODE 전 @ue-gas/umg-specialist 자문 권장
  SR[+UE-SPEC]: GAS/UI 파일 포함 시 UE 전문가 서브에이전트 위임 / 자동 실행
  ⟲ Evaluator 루프: HIGH[CODE] 단순누락(UPROPERTY/break/EndAbility/하드코딩) → 자동수정 → SR 재검증 1회 / HIGH[ARCH] → 시니 게이트
  EXPLAIN_IMPL: 구현 결정 설명 + 알아/몰라/애매해 → KnowledgeGaps.md  # [고정]
```
> 상세: .claude/refs/protocols.md#PIPELINE_FLOW

## INDEX
> 조회 전용 — SKILL의 ON_DEMAND_REFS 지시 시만. 자동 순회 금지.
```yaml
불변 제약:     .claude/refs/constraints.md           ← INIT 자동 로드
워크플로우:    .claude/refs/protocols.md
커밋 정책:     .claude/refs/commit-policy.md
상호작용:      .claude/refs/ask-user-format.md
완료 상태:     .claude/refs/completion-status.md
guardrails 명세: .claude/refs/guardrails-manifest.md   ← /guardrails + SESSION_END [PR-0]
GAS 규칙:      .claude/rules/gas-code.md             ← Source/**/GAS/** 작업 시
UI 규칙:       .claude/rules/ui-code.md              ← Source/**/UI/** 작업 시
일반 컨벤션:   .claude/rules/general-code.md         ← 모든 Source/** 작업 시
```
