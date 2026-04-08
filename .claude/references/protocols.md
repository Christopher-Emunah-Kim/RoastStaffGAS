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
[EXPLAIN_IMPL]* — CODE 완료 후 구현 결정 설명 + 지식 체크
  │  ├─ 핵심 결정 3~5개 설명 (함수 흐름 / 데이터 타입 / UE 패턴 선택 이유)
  │  ├─ 사용자 응답: 알아 / 몰라 / 애매해
  │  └─ _Design/Learning/KnowledgeCheck_YYYY-MM-DD_MODULE.md 저장
  │     + _Design/Learning/KnowledgeGaps.md 갱신
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
  - ACTIVE_WORK 내 커밋완료(✓ COMMITTED) FEATURE 4개 이상 누적
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

## PLAN_ARCHIVE
```yaml
트리거:
  - Plans/active/ 내 커밋완료 플랜 5개 이상 누적
  - /commit 스킬 [F] 갱신 단계에서 함께 처리 권장
  - "플랜 정리해줘" 발언

방법:
  대상: Plans/active/ 내 COMPLETED_LOG에 기록된 FEATURE와 일치하는 PLAN_*.md
  이동: Plans/active/PLAN_*.md → Plans/completed/PLAN_*.md
  순서: TODO_COMPACT와 동시 수행 권장

결과:
  Plans/active/: 진행 중(미커밋) 플랜만
  Plans/completed/: 커밋 완료 플랜 전체
```

## SESSION_END
```
트리거: "세션 종료" / "핸드오프" / "종료할게" / "세션 마칠" / "끝낼게" / "마무리할게" 언급 시

절차:
1. _Design/TODO.md 정리
   - commit [F]에서 처리된 FEATURE(COMPLETED_LOG 이동 완료)는 스킵
   - 미처리 완료 FEATURE만 COMPLETED_LOG 이동 + Plan active→completed 이동
2. [PR] 파이프라인 자가 진단 (아래 참조)
3. touch "$CLAUDE_PROJECT_DIR/.claude/.session_end_flag"
4. Stop Hook이 Handoff 생성 (Claude 직접 작성 금지)
```

## PIPELINE_REVIEW
```
[PR] 파이프라인 자가 진단 — SESSION_END 절차 2번에서 실행

목적: 이번 세션에서 낭비된 토큰·병목·규칙 불명확성을 진단하고,
      사용자 승인 하에 파이프라인 파일을 직접 개선한다.

─────────────────────────────────────────
[PR-1] 이번 세션 관찰 항목
─────────────────────────────────────────
아래 신호를 대화 흐름에서 자가 진단:

  토큰 낭비 신호:
    · 같은 파일을 2회 이상 Read한 경우
    · 불필요한 에이전트 호출 (결과가 단순 파일 읽기였던 경우)
    · 설명이 과도하게 길었던 응답 (사용자가 짧게 답한 경우)
    · Grep/Glob으로 충분한데 Read를 먼저 쓴 경우

  병목 신호:
    · 같은 주제로 사용자와 3회 이상 왕복한 경우
    · BLOCKED 발생 및 원인
    · 규칙 해석이 애매해서 판단을 오래 한 경우
    · 사용자가 내 결과물을 2회 이상 수정 요청한 경우

  규칙 불명확 신호:
    · 규칙 간 충돌로 어느 쪽을 따를지 판단이 필요했던 경우
    · 파이프라인 순서를 우회하거나 예외 처리한 경우
    · HEAVY_OP_POLICY 기준이 애매해서 판단이 느렸던 경우

─────────────────────────────────────────
[PR-2] 진단 보고 형식
─────────────────────────────────────────
  🔧 [PIPELINE_REVIEW] 세션 자가 진단

  ## 관찰 패턴
  | 항목 | 발생 내용 | 심각도 |
  |------|----------|--------|
  | 파일 재읽기 | [파일명] N회 | 낮음/중간/높음 |
  | 병목 Q&A   | [주제] N회 왕복 | ... |
  | 규칙 불명확 | [어떤 규칙] | ... |

  ## 병목 원인 진단
  [가장 낭비된 지점 + 구조적 원인]

  ## 개선 제안 (1~3개)
  A) [대상 파일] — [어떤 규칙/절차를] [어떻게 바꿀지] / 기대효과: [N회 왕복 → 1회]
  B) ...

  → 승인하시면 해당 파일 직접 수정합니다.
    A 승인 / B 승인 / 전체 승인 / 건너뛰기

─────────────────────────────────────────
[PR-3] 승인 후 처리
─────────────────────────────────────────
  → 승인된 항목: 해당 .claude/ 파일 직접 Edit
  → 건너뛰기: 관찰 내용만 _Design/Reviews/PIPELINE_LOG.md 에 기록
  → 모든 항목 "없음": "이번 세션 특이사항 없음" 한 줄 기록 후 종료

  저장 위치: _Design/Reviews/PIPELINE_LOG.md
  형식:
    ## YYYY-MM-DD
    관찰: [요약]
    개선: [적용된 항목] or 없음
```

## GEMINI_POLICY
```yaml
PLAN단계:  사용자 선택 시만 (@cross-reviewer) — 설계 외부 검증 가치 있음
SR단계:    기본 OFF. 사용자 "Gemini도 봐줘" 명시 시만
자동호출:  어느 단계에서도 금지
이유:      SR은 Claude가 이미 깊이 분석 후라 중복 가능성 높음
```
