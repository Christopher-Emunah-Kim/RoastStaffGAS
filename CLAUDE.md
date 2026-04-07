# CLAUDE.md
> 라우터 + 목차. 규칙 상세는 .claude/references/ 참조.

## PROJECT
> 상세: _Design/References/README.md
```yaml
name: RoastStaffGAS
arch: CSV → DataTable → Subsystem → GA
refs:
  기획서:   _Design/References/Systems/
  계획서:   _Design/Plans/active/
  리뷰:     _Design/Reviews/
  학습:     _Design/Learning/
  핸드오프: _Design/Handoff/
```

## ABSOLUTE_RULES
```
1. 파이프라인 강제: PLAN → CODE → [TEST] → [SR] → [LEARN] → COMMIT
2. 기획서(_Design/References/Systems/) 또는 계획서(_Design/Plans/active/) 없이 코드 작성 금지
3. 기획서 충돌 시: 중단 → "기획서 ○○의 ○○ 규칙과 충돌" → 선택지 제시
4. 코딩 규칙: .claude/skills/coding/references/conventions.md 준수
5. [TEST][SR][LEARN]은 사용자 승인 후 실행
6. 3회 실패 → BLOCKED 자동 선언
7. 무거운 작업 시작 전 반드시 사용자 확인 (HEAVY_OP_POLICY 참조)
8. git commit 실행 전 반드시 스테이지 구성 + 커밋 메시지를 제안하고 사용자 승인 대기
   - "커밋해줘" 발언 없이 git commit 실행 절대 금지
   - 커밋은 기능 완성 + 테스트 완료 상태에서만 실행
   - 승인 없이 git commit 실행 = ABSOLUTE 위반
9. 작업 완료 후 판단 구조 공개: 이 결정에서 고려한 선택지들과 각각을 선택/버린 이유를 간략히 설명할 것
10. 세션 마무리 시 역질문 1개: "이 부분에서 시니라면 어떻게 했을 것 같아요?" — 시니의 사고를 자극하는 질문으로 마무리
```


## SESSION_START
> 상세 프로토콜: .claude/references/protocols.md
```
1. _Design/TODO.md 읽기                                          ← 최우선
2. _Design/Plans/active/ → Glob("PLAN_*.md")으로 파일 목록 확인  ← 있으면 최신 1개만 Read
3. _Design/Changesets/CHANGESET.md 읽기                         ← 커밋 관련 작업 시만
```

## ROUTING_TABLE
```yaml
PLAN:   trigger: ["계획","PLAN","구현하고 싶어","새 기능","시스템 만들어줘"]
        load:    [.claude/skills/planning/SKILL.md]

CODE:   trigger: ["코드 작성","CODE","구현해줘","만들어줘","이어서"]
        load:    [.claude/skills/coding/SKILL.md]
        pre:     _Design/TODO.md ACTIVE 항목 확인

TEST:   trigger: ["테스트","TEST","검증","확인해줘"]
        load:    [.claude/skills/test/SKILL.md]

SR:     trigger: ["시니어리뷰","@senior-reviewer","리뷰해줘"]
        load:    [.claude/agents/senior-reviewer.md]

SR_F:   trigger: ["전체리뷰","@senior-reviewer-full"]
        load:    [.claude/agents/senior-reviewer-full.md]

LEARN:  trigger: ["학습리포트","@learning-coach"]
        load:    [.claude/agents/learning-coach.md]
        note: LEARN은 세션 마다 자동 권장 — 시니의 성장 추적이 목적

CROSS:  trigger: ["Gemini리뷰","@cross-reviewer"]
        load:    [.claude/agents/cross-reviewer.md]

SYNC:   trigger: ["동기화","sync","정합성","기획서랑 코드 맞아"]
        load:    [.claude/skills/sync-doc/SKILL.md]

UPDOC:  trigger: ["기획서 수정","기획서 업데이트","update-design"]
        load:    [.claude/skills/update-design/SKILL.md]

COMMIT: trigger: ["커밋해줘","커밋하자","commit"]
        load:    [.claude/skills/commit/SKILL.md]
        pre:     _Design/Changesets/CHANGESET.md + _Design/TODO.md 읽기 (SESSION_START에서 읽힌 경우 생략)

UPDATE: trigger: ["플로우 개선","스킬 수정","규칙 바꿔","agent-update","시스템 업데이트"]
        load:    [.claude/skills/agent-update/SKILL.md]

HARNESS: trigger: ["하네스","워크플로 강화","실수 막아","구조적 차단","harness"]
         load:    [.claude/skills/harness/SKILL.md]

END:    trigger: ["세션 종료","핸드오프","종료할게"]
        flow:    .claude/references/protocols.md#SESSION_END 실행
        note:    TODO 정리 → [PR] 파이프라인 자가 진단 → 승인 후 파이프라인 파일 수정 → Handoff

GC:     trigger: ["청소","gc","전체 점검","가비지","안티패턴 정리","코드 정리"]
        load:    [.claude/skills/gc/SKILL.md]
```

## PIPELINE_FLOW
> 상세: .claude/references/protocols.md#pipeline
```
[PLAN] → [CODE] → [EXPLAIN_IMPL]* → [TEST]* → [SR]* → [LEARN]* → COMMIT
  * = 사용자 승인 필요
  EXPLAIN_IMPL: CODE 완료 후 구현 결정 설명 + 알아/몰라/애매해 응답 → KnowledgeGaps.md 축적
```

## LOAD_STRATEGY
```yaml
always:     [CLAUDE.md]
session:    [_Design/TODO.md]
on_route:   해당 SKILL.md 또는 agent.md 만
on_demand:  각 SKILL의 ON_DEMAND_REFS 명시 시만
never_auto: _Design/References/Systems/ 전체 순회 금지

# 중복 읽기 방지
no_reread:  SESSION_START에서 읽힌 파일(TODO.md, PLAN_*.md)은 같은 세션 내 재읽기 금지

# Grep-first 원칙
grep_first: |
  파일 전체 Read 전에 Grep으로 필요한 섹션 먼저 탐색.
  적용 대상: TODO.md, CHANGESET.md, PLAN_*.md, 대형 참조파일
  예외: 신규 파일 또는 전체 구조 파악이 필요한 경우

# 에이전트 사용 제한 (토큰/처리시간 보호)
agent_policy:
  Explore:          사용자 명시 요청 시만 — 자동 호출 절대 금지
  general-purpose:  사용자 명시 요청 시만 — 자동 호출 절대 금지
  허용 자동 호출:   planning-architect (PLAN 단계), senior-reviewer (SR 단계), learning-coach (LEARN 단계)
  기타 에이전트:    사용자 명시 요청 시만
```


## HEAVY_OP_POLICY
```yaml
대상:
  - Agent 호출 (종류 무관)
  - 파일 9개 이상 연속 Read
  - 디렉터리 전체 탐색 (Glob + 다수 Read 조합)

실행 전 필수 안내 형식:
  ⚠️ [무거운 작업 예고]
  작업: [무엇을 하려는지 한 줄]
  예상 비용: [에이전트 호출 / 파일 N개 읽기 등]
  A) 진행
  B) 방식 변경 (경량 대안 제시)
  C) 취소

예외 (안내 없이 진행 가능):
  - SESSION_START 필수 파일 (TODO.md, 최신 PLAN 1개)
  - 사용자가 직접 명령한 에이전트 호출
```

## REFERENCES
> 조회용 인덱스 — 자동 로드 금지. 해당 SKILL의 ON_DEMAND_REFS 지시 시만 읽기.
```yaml
프로젝트 정보:  _Design/References/README.md
워크플로우:     .claude/references/protocols.md
커밋 정책:      .claude/references/commit-policy.md
상호작용 형식:  .claude/references/ask-user-format.md
완료 상태:      .claude/references/completion-status.md
```
