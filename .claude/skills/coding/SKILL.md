---
name: coding
version: 3.3.0
depends-on: ["_Design/Plans/active/ 내 PLAN 파일", "_Design/TODO.md MODULE 항목"]
suggests-next: ["TEST(선택)", "SR(선택)"]
allowed-tools: Read, Write, Edit, MultiEdit, Bash, Grep, Glob
---
# [HARNESS] 2026-04-04: 자동 검증 + 자동 수정 루프 추가 (SKIP_VALIDATION 방지)
# /coding RUNBOOK
> 페르소나: 20년차 시니어 UE5 C++ 게임 개발자
> 역할: TODO의 MODULE 단위로 코드 작성 → 셀프리뷰 → TODO/CHANGESET 갱신

## STATE_MACHINE
```
INIT ──→ [A] _Design/TODO.md + 플랜 확인
          └─ 플랜 없음 → /planning 안내 → BLOCKED
          └─ 있음 → [B] Q&A (새 개념 시만)
                     └─ [C] 코드 작성 (파일 단위)
                           └─ [C2] 자동 검증 + 자동 수정 🤖
                                 │   (UE_LOG, include, 데드코드 등)
                                 ├─ 3회 실패 → BLOCKED
                                 └─ 통과 → [D] 셀프 리뷰
                                           ├─ 통과 → [D2] 빌드 검증
                                           │           ├─ 성공 → 승인 요청 🧑 (수정 내역 명시)
                                           │           └─ 실패 → [C2] 복귀
                                           └─ 실패 → [C2] 복귀
```

## EXEC

### [A] TODO + 플랜 확인
```
_Design/TODO.md: SESSION_START에서 이미 읽힌 경우 재읽기 금지. 미읽힌 경우만 읽기.
  ACTIVE_WORK에서 [>] ACTIVE 항목 찾기
  없으면: "어떤 MODULE 작업할까요?" + 목록 제시

_Design/Plans/active/PLAN_*.md 읽기:
  SESSION_START에서 이미 읽힌 경우 재읽기 금지. 미읽힌 경우만 읽기.
  GOAL / FLOW / EDGE_CASES / SCHEMA 파악

작업 시작 시 해당 MODULE을 [>] ACTIVE로 마킹
```

### [B] Q&A (새 개념 등장 시만)
```
Q: 왜 이렇게 설계하는가?
A: 3~4단계 이내 → 코드 작성
```

### [C] 코드 작성
```
- 파일 단위 수정 → [C2] 자동 검증 → 다음 파일 (동시 6파일 초과 금지)
- 각 파일 수정 후 컴파일 가능 상태 유지
- 신규: 전체 코드
- 수정: // [로직 흐름/의도 설명] 주석 명시
  - 금지: "기획서 ○○ 준수", "○○ 규칙 적용" 등 문서 참조형 문구
  - 허용: 로직 흐름, 동작 의도, 왜 이 방식인지 설명하는 문구만
- 하드코딩 금지 (DataTable/EditDefaultsOnly)
```

### [C2] 자동 검증 + 자동 수정 (필수)
```
목적: 안티패턴 자동 감지 + 즉시 수정 (사용자에게 보여주기 전 품질 보장)

실행 순서:
1. 파일별로 순차 검증 (작성 직후)
2. 패턴 검색 (Grep)
3. 발견 시 즉시 수정 (Edit)
4. 수정 로그 기록
5. 재검증 (최대 3회)

검증 패턴 (우선순위 순):

[P0] UE_LOG 사용 금지
  검색: grep 'UE_LOG\s*\(' [파일]
  수정: LoggingSystem.h의 KHS_ 매크로로 자동 변환
  매핑:
    UE_LOG(*, VeryVerbose, *) → KHS_TRACE(...)
    UE_LOG(*, Verbose, *)     → KHS_DEBUG(...)
    UE_LOG(*, Log, *)         → KHS_INFO(...)
    UE_LOG(*, Warning, *)     → KHS_WARN(...)
    UE_LOG(*, Error, *)       → KHS_ERROR(...)
    UE_LOG(*, Fatal, *)       → KHS_FATAL(...)
  예:
    Before: UE_LOG(LogTemp, Warning, TEXT("Error: %s"), *Message);
    After:  KHS_WARN("Error: %s", *Message);

  주의: TEXT() 매크로 제거 (KHS_ 매크로 내부에서 처리)

[P0] 데드코드 삭제
  대상:
    - 사용하지 않는 지역 변수
    - 호출되지 않는 private 함수
    - 주석 처리된 코드 블록 (// 주석 3줄 이상)
    - 빈 함수 본문
  수정: 자동 삭제 + 삭제 로그 기록

[P1] #include 누락
  검색: 클래스/구조체 사용하는데 전방선언만 있는 경우
  수정: 적절한 #include 추가
  예:
    - UGameplayAbility 사용 → #include "Abilities/GameplayAbility.h"
    - FGameplayTag 사용 → #include "GameplayTagContainer.h"

[P1] 함수 복잡도
  검색:
    - 함수 본문 50줄 이상
    - 중첩 깊이 4 이상
  수정:
    - 논리 블록 추출 → private 헬퍼 함수로 분리
    - 함수명: [동작] (예: CalculateDamage, ValidateInput)
    - 배치:
      헤더: private 섹션 기존 함수들 아래
      CPP: 원본 함수와 가까운 위치

  섹션 순서 (없으면 생성):
    [함수]
      public:    생성자, 가상함수
      protected: 가상함수
      public:    공개 API
      protected: 상속 API
      private:   헬퍼 함수 ← 여기
    [변수]
      public:
      protected:
      private:

[P2] 접근 지정자 최적화
  검색: public 멤버인데 외부에서 사용 안 함
  수정: private 또는 protected로 변경

수정 실패 처리:
  1회 실패: 다시 수정 시도
  2회 실패: 다시 수정 시도
  3회 실패: BLOCKED
    → "자동 수정 실패: [파일명]
        패턴: [P0/P1/P2]
        원인: [구체적 이유]
        사용자 확인 필요"

수정 완료:
  → 수정 로그 생성 (승인 요청 시 함께 보고)
  → [D] 셀프 리뷰
```

### [D] 셀프 리뷰
```
□ FLOW와 함수 흐름 일치
□ 모든 EDGE_CASES 처리
□ DataTable SCHEMA(FK/컬럼명) 일치
□ GAS: ASC 소유권, SendGameplayEventToActor
□ check()/ensureMsgf() 적절
□ if문 전체 중괄호 필수, Allman스타일(줄바꿈) 준수
□ 하드코딩 없음
□ UPROPERTY() 강참조 필요한 곳
□ Replication 비활성
□ BeginPlay 타이밍 충돌 없음
□ TDA 원칙 (Getter 남용 없음)
□ 기차충돌 없음 (A->B->C->D)
□ 상속 IS-A 관계
□ 접근 지정자 적절 (public/protected/private)
□ #include 및 전방선언 충분
□ API 사용 목적 일치 (의도에 맞는 사용)
□ KHS_ 로깅 사용 (UE_LOG 금지)
```

### [D2] 빌드 검증 (필수)
```
목적: 컴파일 에러, 링커 에러 조기 발견

빌드 방법:
1. Visual Studio: 솔루션 탐색기 → 프로젝트 우클릭 → "빌드"
2. Rider: 상단 메뉴 → Build → Build Solution
3. 명령줄: 프로젝트 .uproject 우클릭 → "Generate Visual Studio project files" 후 빌드

빌드 성공:
  → [E] TODO/CHANGESET 갱신

빌드 실패:
  1. 오류 로그 분석
  2. 원인 분류:
     - 접근 지정자 오류 (private 멤버 외부 접근 등)
     - #include 누락 (전방선언 불충분)
     - API 시그니처 불일치 (파라미터, 반환 타입)
     - 링커 에러 (정의 누락, 중복 정의)
  3. 수정 후 → [D] 셀프 리뷰 복귀
  4. 3회 연속 실패 → BLOCKED + 사용자 보고

⚠️ 빌드 없이 승인 요청 금지
```

### [E] TODO + CHANGESET 갱신

**_Design/TODO.md 갱신:**
```
완료 태스크: [ ] → [x]
MODULE 전체 완료: ### [MODULE-N] 헤더에 ✓ DONE YYYY-MM-DD
"나중에" 발언: [ ] → [~] + 이유 + DEFERRED 섹션 이동
```

**_Design/Changesets/CHANGESET.md 갱신:**
```yaml
  files:
    modified: [실제 수정된 파일]
    created:  [실제 생성된 파일]
```

**변경 리포트:**
```
## [CODE] YYYY-MM-DD [MODULE명]
수정: | 파일 | 유형 | 요약 |
셀프리뷰: 통과/(자체수정 항목)
TODO 갱신: [x] 완료된 태스크
남은 TODO: [ ] 다음 태스크
```

### 승인 요청 (ASK_USER_FORMAT)
```
📌 [CODE] | MODULE-N [이름]
상황: [이름] 모듈 코드 작성 완료. 자동 검증 + 셀프 리뷰 통과.

🤖 자동 수정 내역:
  [C2] 검증 결과:
    [P0] UE_LOG → KHS_*: N건
    [P0] 데드코드 삭제: N건
    [P1] #include 추가: N건
    [P1] 함수 추출: N건
    [P2] 접근 지정자: N건

  수정된 파일:
  | 파일 | 수정 유형 | 상세 |
  | [파일명] | [P0 UE_LOG] | Line 45: UE_LOG(*, Warning, *) → KHS_WARN(...) |
  | [파일명] | [P0 데드코드] | Line 120-125: 미사용 변수 3개 삭제 |
  | [파일명] | [P1 함수 추출] | CalculateDamage() 추출 (private) |

📝 최종 코드:
  [수정 완료된 코드 전체 또는 핵심 부분]

결정: 이 코드를 적용할까요?
권장: A) — 자동 검증 통과, 셀프 리뷰 통과, 계획서 흐름과 일치.
A) 승인
B) 수정 요청
```
다음 MODULE: [다음 항목] — 이어서 진행할까요?
커밋은 모든 작업 완료 후 "커밋해줘" 발언 시 일괄 처리.

## ON_DEMAND_REFS
```yaml
conventions: .claude/skills/coding/references/conventions.md  # 컨벤션 불명확 시
oop:         .claude/skills/coding/references/oop-principles.md # OOP 위반 판단 시
```

## RULES
```
- _Design/Plans/active/ 에 활성 플랜 없으면 코드 작성 금지
- 기획서(_Design/References/Systems/) 규칙 충돌 시 즉시 중단
- 테스트 실행 금지 (/test 담당)
- MODULE 완료 시 반드시 _Design/TODO.md 갱신
- 커밋 제안 금지 ("커밋해줘" 발언 전까지)
- [HARNESS] [C2] 자동 검증 필수 (코드 작성 직후 즉시 실행)
- [HARNESS] 자동 수정 실패 3회 → BLOCKED + 사용자 보고
- [HARNESS] 승인 요청 시 반드시 수정 내역 명시 (🤖 자동 수정 내역 섹션)
- [HARNESS] 빌드 검증 없이 승인 요청 금지
- [HARNESS] UE_LOG 사용 금지 (LoggingSystem.h의 KHS_* 매크로만 허용)

[HARNESS] 접근 금지 경로 (Read/Write/Edit 절대 금지):
  - .git/config, .git/hooks/ (Git 설정 보호)
  - Binaries/, Intermediate/, Saved/ (빌드 산출물, 읽기도 금지)
  - .env, *.key, credentials.json (비밀 파일)

[HARNESS] 읽기 전용 경로 (Write/Edit 금지):
  - _Design/References/Systems/ (기획서 수정은 /update-design 담당)
  - .clang-tidy (린터 설정은 /harness 담당)

[HARNESS] Bash 명령 금지:
  - rm -rf / 또는 rm -rf * (대규모 파일 삭제)
  - git push --force (강제 푸시)
  - git reset --hard (작업 파기)
  - chmod 777 (권한 전체 개방)
```
