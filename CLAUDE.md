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
```

## SESSION_START
> 상세 프로토콜: .claude/references/protocols.md
```
1. _Design/TODO.md 읽기                      ← 최우선
2. _Design/Plans/active/ 확인                ← 진행 중 플랜
3. _Design/Changesets/CHANGESET.md 확인     ← PENDING_COMMIT만
4. _Design/Handoff/HANDOFF_LATEST.md        ← 추가 컨텍스트 필요 시만
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

CROSS:  trigger: ["Gemini리뷰","@cross-reviewer"]
        load:    [.claude/agents/cross-reviewer.md]

SYNC:   trigger: ["동기화","sync","정합성","기획서랑 코드 맞아"]
        load:    [.claude/skills/sync-doc/SKILL.md]

UPDOC:  trigger: ["기획서 수정","기획서 업데이트","update-design"]
        load:    [.claude/skills/update-design/SKILL.md]

COMMIT: trigger: ["커밋해줘","커밋하자","commit"]
        load:    [.claude/skills/commit/SKILL.md]
        pre:     _Design/Changesets/CHANGESET.md + _Design/TODO.md 읽기

UPDATE: trigger: ["플로우 개선","스킬 수정","규칙 바꿔","agent-update","시스템 업데이트"]
        load:    [.claude/skills/agent-update/SKILL.md]
```

## PIPELINE_FLOW
> 상세: .claude/references/protocols.md#pipeline
```
[PLAN] → [CODE] → [TEST]* → [SR]* → [LEARN]* → COMMIT
  * = 사용자 승인 필요
```

## LOAD_STRATEGY
```yaml
always:     [CLAUDE.md]
session:    [_Design/TODO.md]
on_route:   해당 SKILL.md 또는 agent.md 만
on_demand:  각 SKILL의 ON_DEMAND_REFS 명시 시만
never_auto: _Design/References/Systems/ 전체 순회 금지
```

## REFERENCES
```yaml
프로젝트 정보:  _Design/References/README.md
워크플로우:     .claude/references/protocols.md
커밋 정책:      .claude/references/commit-policy.md
상호작용 형식:  .claude/references/ask-user-format.md
완료 상태:      .claude/references/completion-status.md
```
