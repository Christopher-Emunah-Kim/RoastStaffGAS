---
name: harness
version: 1.0.0
depends-on: []
suggests-next: ["@senior-reviewer", "COMMIT"]
allowed-tools: Read, Write, Edit, Bash, Grep, Glob
---
# /harness RUNBOOK
> 역할: 워크플로 실패 패턴 감지 → 구조적 차단 장치 설계 + 적용
> 원칙: 프롬프트 수정 금지. 시스템 레벨 차단 필수.

## PHILOSOPHY (핵심만)
```
하네스 엔지니어링 = AI 실수를 '구조적으로 불가능'하게 만드는 시스템 강화

4가지 기둥:
A. 컨텍스트 (CLAUDE.md, SKILL.md 규칙 강화)
B. 게이트 (Git hooks, 린터, 테스트 자동 차단)
C. 도구 경계 (allowed-tools, 파일 접근 제한)
D. 피드백 루프 (자동 검증, 안티패턴 감지)
```

## STATE_MACHINE
```
INIT ──→ [A] 증상 분류 (매트릭스 조회)
          └─ [B] 진단 체크리스트 실행
                └─ [C] 액션 카탈로그에서 해결책 선택
                      └─ [D] 변경 계획 제시 + 승인
                            └─ A) → [E] 파일 수정 → [F] 검증 → DONE
```

## EXEC

### [A] 증상 분류 (결정 트리)
```
사용자 발언 → 증상 매핑:

| 증상 패턴 | 분류 코드 | 대상 기둥 |
|-----------|-----------|-----------|
| "플랜 없이 코딩했어" "TODO 안 읽어" | SKIP_PREREQ | A+B |
| "TODO 갱신 안 해" "CHANGESET 빠뜨려" | SKIP_POST | A+B |
| "특정 폴더 수정하면 안 돼" | FORBIDDEN_PATH | C |
| "커밋 전 검증 필수로 해줘" | GATE_MISSING | B |
| "셀프리뷰 건너뛰어" | SKIP_VALIDATION | A+D |
| "하드코딩 자꾸 해" | ANTI_PATTERN | D |
| "위험한 명령 실행했어" | DANGEROUS_CMD | C |

분류 실패 시:
  → "구체적으로 어떤 실수를 막고 싶으신가요?"
  → 예시 제공
```

### [B] 진단 체크리스트
```
분류 코드별 진단:

[SKIP_PREREQ] 사전 조건 누락:
  □ CLAUDE.md ABSOLUTE_RULES에 규칙 있는가?
  □ 해당 SKILL.md RULES에 차단 조건 있는가?
  □ 조건문이 명확한가? (모호한 표현 없음)
  □ Git hooks에 검증 로직 있는가?

  진단 결과:
    4개 모두 ✓  → 규칙이 있는데 무시됨 → 표현 강화 필요
    1개 이상 ✗ → 규칙 자체 없음 → 신규 추가 필요

[SKIP_POST] 사후 조건 누락:
  □ SKILL.md에 "갱신" 단계 명시돼 있는가?
  □ 갱신 검증 체크리스트 있는가?
  □ Git hooks에 파일 변경 체크 있는가?

  진단 결과:
    ✓✓✓ → 표현 강화
    ✗ 있음 → 규칙 추가

[FORBIDDEN_PATH] 금지 경로:
  □ allowed-tools에 Write/Edit 제한 있는가?
  □ SKILL.md에 접근 금지 경로 명시돼 있는가?
  □ Git hooks에 경로 검증 있는가?

[GATE_MISSING] 게이트 부재:
  □ .git/hooks/ 디렉터리 존재하는가?
  □ 해당 훅 파일 (pre-commit 등) 존재하는가?
  □ 훅이 실행 권한 있는가? (chmod +x)

[SKIP_VALIDATION] 검증 건너뜀:
  □ SKILL.md에 셀프리뷰 체크리스트 있는가?
  □ 체크리스트 항목이 구체적인가?
  □ 검증 실패 시 중단 조건 명시돼 있는가?

[ANTI_PATTERN] 안티패턴:
  □ 감지 가능한 패턴인가? (정규식/AST)
  □ SKILL.md에 금지 패턴 명시돼 있는가?
  □ 린터/테스트로 자동 감지 가능한가?

[DANGEROUS_CMD] 위험 명령:
  □ allowed-tools에 Bash 있는가?
  □ Bash 사용 시 제약 조건 있는가?
  □ 특정 명령어 블랙리스트 있는가?
```

### [C] 액션 카탈로그 (기둥별)
```
진단 결과 → 액션 선택:

[기둥 A: 컨텍스트 파일 강화]
A1. CLAUDE.md ABSOLUTE_RULES 추가/강화
    대상: CLAUDE.md
    형식: "N. [조건] 시 [액션] 금지/필수"
    예: "3. 기획서 또는 계획서 없이 코드 작성 금지"

A2. SKILL.md RULES 섹션 추가/강화
    대상: .claude/skills/[스킬명]/SKILL.md
    위치: ## RULES 섹션
    형식: "- [조건] 시 [액션] 금지/필수"

A3. SKILL.md 실행 단계에 조건 체크 추가
    대상: .claude/skills/[스킬명]/SKILL.md
    위치: ## EXEC → ### [단계명]
    형식: "조건 확인 → 미충족 시 BLOCKED"

A4. allowed-tools 제한
    대상: .claude/skills/[스킬명]/SKILL.md
    위치: YAML frontmatter
    예: allowed-tools: [Read, Grep] (Write, Edit 제거)

[기둥 B: Git Hooks 게이트]
B1. pre-commit 훅 추가
    파일: .git/hooks/pre-commit
    검증: CHANGESET.md, TODO.md, PLAN 파일 존재
    실패 시: 커밋 차단 + 안내 메시지

B2. commit-msg 훅 추가
    파일: .git/hooks/commit-msg
    검증: 커밋 메시지 형식
    실패 시: 커밋 차단

[기둥 C: 도구 경계]
C1. 파일 경로 화이트리스트
    대상: SKILL.md
    위치: ## RULES
    형식: "- 접근 허용: [경로 패턴]"
    예: "- 접근 허용: Source/*, _Design/* 만"

C2. 파일 경로 블랙리스트
    대상: SKILL.md
    위치: ## RULES
    형식: "- 접근 금지: [경로 패턴]"
    예: "- 접근 금지: .git/*, Binaries/*"

C3. Bash 명령 블랙리스트
    대상: SKILL.md
    위치: ## RULES
    형식: "- Bash 금지 명령: [패턴]"
    예: "- Bash 금지 명령: rm -rf, DROP, DELETE FROM"

[기둥 D: 피드백 루프]
D1. 셀프리뷰 체크리스트 추가
    대상: SKILL.md
    위치: ## EXEC → 검증 단계
    형식: □ [검증 항목] (체크박스)

D2. suggests-next에 검증 단계 추가
    대상: SKILL.md
    위치: YAML frontmatter
    예: suggests-next: ["TEST(필수)", "SR(선택)"]
```

### [D] 변경 계획 제시 (템플릿)
```
📌 [HARNESS] | [분류코드]: [증상 한 줄]

진단:
  분류: [SKIP_PREREQ / SKIP_POST / ...]
  체크리스트: [N/M 통과]
  부족 요소: [구체적으로]

선택된 액션:
  1️⃣ [A1] CLAUDE.md ABSOLUTE_RULES 강화
     파일: CLAUDE.md
     위치: ## ABSOLUTE_RULES
     추가 내용:
       ```
       N. [구체적 규칙]
       ```

  2️⃣ [B1] pre-commit 훅 추가
     파일: .git/hooks/pre-commit
     검증 로직:
       ```bash
       if [ ! -f _Design/Plans/active/PLAN_*.md ]; then
         echo "❌ 활성 플랜 없음. /planning 먼저 실행."
         exit 1
       fi
       ```

차단 효과:
  즉시 차단: [B1 Git hooks]
  다음 세션부터: [A1 CLAUDE.md]

A) 적용
B) 수정
```

### [E] 파일 수정
```
수정 순서:
1. CLAUDE.md (있으면)
2. .claude/skills/[대상]/SKILL.md (있으면)
3. Git hooks (있으면)
4. 기타 참조 파일

각 파일마다:
  1. Read (현재 내용 확인)
  2. Edit (정확한 위치만 수정)
  3. version 필드 +0.1
  4. 주석 추가: # [HARNESS] YYYY-MM-DD: [증상 코드]
```

### [F] 검증 체크리스트
```
□ 규칙 표현이 명령형인가? (금지/필수, ~하지 마/해야 함)
□ 조건이 명확한가? (모호한 단어 없음)
□ 차단 메커니즘이 있는가? (BLOCKED, exit 1 등)
□ 다른 SKILL과 충돌 없는가?
□ Git hooks 실행 권한 설정했는가? (chmod +x)
```

## ON_DEMAND_REFS
```yaml
protocols:     .claude/references/protocols.md
commit_policy: .claude/references/commit-policy.md
conventions:   .claude/skills/coding/references/conventions.md
```

## COMPLETION
```
DONE:           하네스 강화 완료
DONE_WITH_HOOK: 완료 + Git hooks 추가 (테스트 권장)
BLOCKED:        현재 구조로 차단 불가 (수동 검증 필요)
```

## ABSOLUTE_RULES
```
1. 프롬프트 문구 추가(~하세요, 주의, 권장) 절대 금지
2. 승인 없이 파일 수정 금지
3. 계획 제시 시 반드시 액션 코드(A1, B2 등) 명시
4. Git hooks 추가 시 chmod +x 필수
5. 전체 파일 재작성 금지 (Edit만)
```
