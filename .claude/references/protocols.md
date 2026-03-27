# PROTOCOLS
> 워크플로우 상세 규칙. CLAUDE.md에서 참조.
> Skills/Agents는 이 파일의 해당 섹션을 on_demand로 읽는다.

## PIPELINE_FLOW
```
[PLAN]
  │  ├─ @planning-architect 호출 (기획서 분석 + 모듈 분해)
  │  └─ _Design/TODO.md 갱신 + _Design/Plans/active/PLAN_*.md 저장
  ▼
[CODE] — MODULE 단위 반복
  │  ├─ _Design/TODO.md 체크박스 갱신
  │  └─ _Design/Changesets/CHANGESET.md 갱신
  ▼
[TEST]*
  │  └─ _Design/TODO.md TEST_OK 표시
  ▼
[SR]*
  │  ├─ _Design/Reviews/SR_*.md 저장
  │  └─ REVIEW_STATUS 갱신
  ▼
[LEARN]*
  │  ├─ _Design/Learning/reports/LEARN_*.md 저장
  │  └─ _Design/Learning/LEARNING_LOG.md 갱신
  ▼
COMMIT
  ├─ _Design/Changesets/CHANGESET.md commit 필드 기입
  ├─ _Design/Plans/active/ → completed/ 이동
  └─ _Design/TODO.md FEATURE 완료 시 COMPLETED_LOG로 이동

* = 사용자 승인 필요
```

## TODO_WORKFLOW
```yaml
계획:   PLAN → _Design/TODO.md ACTIVE_WORK에 FEATURE+MODULE+태스크 추가
코드:   CODE → MODULE 시작 시 [>] ACTIVE, 태스크 완료 시 [x] DONE
나중에: [~] DEFERRED로 이동 + 이유 기록
종료:   완료 MODULE → COMPLETED_LOG로 요약 이동
다음:   _Design/TODO.md 읽으면 현황 즉시 파악
```

## TODO_COMPACT
```yaml
트리거:
  - COMPLETED_LOG 10개 초과
  - 세션 종료 시 사용자 요청
  - "TODO 정리해줘" 발언

방법:
  완료FEATURE → "[x] 기능명 | 커밋 | 날짜 | Plans/completed/파일명" 한 줄
  DEFERRED 중 P2 오래된 것 → 별도 확인 후 제거 또는 유지

결과:
  ACTIVE_WORK: 진행 중인 것만
  DEFERRED:    아직 유효한 것만
  COMPLETED_LOG: 한 줄 요약만
```

## SESSION_END
```
트리거: "세션 종료" / "핸드오프" / "종료할게" 언급 시

절차:
1. _Design/TODO.md 정리 (완료 항목 COMPLETED_LOG 이동)
2. touch "$CLAUDE_PROJECT_DIR/.claude/.session_end_flag"
3. Stop Hook이 Handoff 생성 (Claude 직접 작성 금지)
```

## GEMINI_POLICY
```yaml
PLAN단계:  사용자 선택 시만 (@cross-reviewer) — 설계 외부 검증 가치 있음
SR단계:    기본 OFF. 사용자 "Gemini도 봐줘" 명시 시만
자동호출:  어느 단계에서도 금지
이유:      SR은 Claude가 이미 깊이 분석 후라 중복 가능성 높음
```
