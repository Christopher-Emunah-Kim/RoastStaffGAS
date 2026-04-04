---
name: garbage-collection
version: 1.0.0
depends-on: []
suggests-next: ["COMMIT", "harness"]
allowed-tools: Read, Grep, Glob, Edit, Bash
---
# /gc RUNBOOK
> 역할: 프로젝트 전체 안티패턴 감지 → 자동 청소 → 보고
> 원칙: 코드 품질이 점진적으로 저하되지 않도록 주기적으로 점검 + 자동 제거

## STATE_MACHINE
```
INIT ──→ [A] 스캔 범위 확정
          └─ [B] 안티패턴 전체 스캔 (Grep 기반)
                └─ [C] 스캔 결과 분류 + 우선순위
                      └─ [D] 사용자에게 보고 + 승인
                            ├─ A) 전체 정리 → [E] 자동 청소 → [F] 결과 보고
                            ├─ B) 선택 정리 → [E] 선택 항목만 → [F]
                            └─ C) 취소 → DONE
```

## EXEC

### [A] 스캔 범위 확정
```
기본 스캔 경로: Source/RoastStaffGAS/
제외 경로:
  - Binaries/, Intermediate/, Saved/
  - ThirdParty/
  - .git/

스캔 대상 확장자: .cpp, .h
```

### [B] 안티패턴 전체 스캔
```
우선순위 순으로 순차 실행:

[SCAN-P0] UE_LOG 잔존 검사
  명령: grep -rn 'UE_LOG(' Source/ --include="*.cpp" --include="*.h"
  기대: 0건 (하네스 강화 이후 잔존 여부)

[SCAN-P0] 데드코드 패턴 검사
  대상:
    - // TODO: 주석 (방치된 할일)
    - /* ... */ 블록 주석 코드
    - PRAGMA_DISABLE_OPTIMIZATION (임시 비활성화 방치)
    - #if 0 블록
  명령 예:
    grep -rn '#if 0' Source/ --include="*.cpp" --include="*.h"
    grep -rn 'TODO:' Source/ --include="*.cpp" --include="*.h"

[SCAN-P1] 하드코딩 검사
  대상: 매직 넘버 (int/float 리터럴 직접 사용)
  예외 허용: 0, 1, -1, 100.f (일반적 상수)
  명령: grep -rn '[^A-Za-z_][2-9][0-9]\{1,\}[^0-9]' Source/ --include="*.cpp"

[SCAN-P1] include 과잉 검사
  대상: .h 파일에서 #include "..." 가 15개 이상인 파일
  → 전방선언으로 대체 가능 여부 검토 필요

[SCAN-P2] 미사용 UPROPERTY 검사
  대상: UPROPERTY 붙어있는데 Blueprint에서도, C++에서도 참조 없는 멤버
  (정적 분석 한계 - 의심 항목만 보고)

[SCAN-P2] 긴 함수 목록
  대상: 100줄 이상 함수
  명령: awk '/^[A-Za-z].*\(/{fn=$0; count=0} {count++} count>100{print fn, NR}' Source/**/*.cpp
```

### [C] 스캔 결과 분류
```
결과 테이블:
| 분류 | 패턴 | 파일 수 | 건수 | 자동 수정 가능 |
|------|------|---------|------|----------------|
| [P0] | UE_LOG 잔존 | N | N건 | ✅ |
| [P0] | 데드코드(#if 0) | N | N건 | ✅ |
| [P0] | TODO 방치 | N | N건 | ❌ (수동) |
| [P1] | 하드코딩 | N | N건 | ❌ (수동) |
| [P1] | include 과잉 | N | N건 | ⚠️ (검토 필요) |
| [P2] | 긴 함수 | N | N건 | ⚠️ (검토 필요) |

자동 수정 가능: ✅ → [E]에서 즉시 처리
수동 필요: ❌ → 위치 목록 제공 후 사용자 처리
검토 필요: ⚠️ → 목록 제공 + 사용자 판단
```

### [D] 사용자 보고 + 승인 (ASK_USER_FORMAT)
```
📌 [GC] | 프로젝트 전체 안티패턴 스캔 결과

스캔 범위: Source/RoastStaffGAS/ ([파일 수]개 파일, [줄 수]줄)

🔍 스캔 결과:
  [자동 수정 가능]
  | 패턴 | 건수 | 대표 위치 |
  | UE_LOG 잔존 | N건 | File.cpp:45 외 N곳 |
  | #if 0 블록 | N건 | File.h:120 외 N곳 |

  [수동 처리 필요]
  | 패턴 | 건수 | 파일 목록 |
  | TODO 방치 | N건 | [파일 목록] |
  | 하드코딩 | N건 | [파일 목록] |

  [검토 필요]
  | 패턴 | 건수 | 파일 목록 |
  | include 과잉 | N건 | [파일 목록] |
  | 100줄+ 함수 | N건 | [파일 목록] |

결정: 어떻게 처리할까요?
A) 자동 수정 가능 항목 전체 정리 (N건)
B) 항목별 선택 정리
C) 보고만 (정리 안 함)
```

### [E] 자동 청소
```
처리 순서:
1. [P0] UE_LOG → KHS_* 자동 변환 (coding [C2]와 동일 로직)
2. [P0] #if 0 블록 삭제
3. [P0] // 주석 처리된 코드 블록 삭제 (3줄 이상)

각 파일마다:
  1. Grep으로 대상 확인
  2. Edit으로 수정
  3. 수정 로그 기록

완료 후 → [F] 결과 보고
```

### [F] 결과 보고
```
✅ [GC] DONE

정리 결과:
| 패턴 | 처리 건수 | 영향 파일 |
| UE_LOG → KHS_* | N건 | N개 파일 |
| 데드코드 삭제 | N줄 | N개 파일 |

수동 처리 필요 항목:
| 패턴 | 건수 | 위치 |
(목록 제공)

다음 권장:
  - 커밋: 정리된 파일 커밋
  - /harness: 재발 방지 규칙 강화
```

## ON_DEMAND_REFS
```yaml
conventions: .claude/skills/coding/references/conventions.md
harness:     .claude/skills/harness/SKILL.md
```

## COMPLETION
```
DONE:        스캔 완료, 자동 청소 완료
DONE_REPORT: 스캔 완료, 보고만 (수동 처리 항목 있음)
CLEAN:       발견된 안티패턴 없음 (프로젝트 깨끗함)
BLOCKED:     스캔 실패 (경로 오류 등)
```

## ABSOLUTE_RULES
```
1. 승인 없이 파일 수정 금지
2. Source/ 외 경로 수정 금지 (기획서, 스킬 파일 등)
3. TODO 주석은 자동 삭제 금지 (위치만 보고)
4. 하드코딩은 자동 수정 금지 (의미를 알 수 없음, 위치만 보고)
5. 전체 파일 재작성 금지 (Edit만)
```
