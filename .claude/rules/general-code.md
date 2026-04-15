# 일반 코딩 컨벤션
> 적용 경로: Source/**
> 모든 C++ 파일 작업 시 기본 적용

## 금지 패턴

```cpp
// ❌ GET_GI_SUBSYSTEM 뒤 세미콜론
GET_GI_SUBSYSTEM(URSGameDataSubsystem, GDS);  // 컴파일 에러

// ✅ 세미콜론 없음
GET_GI_SUBSYSTEM(URSGameDataSubsystem, GDS)

// ❌ 중괄호 없는 인라인 if
if (!SkillData) return;

// ✅ 항상 중괄호
if (!SkillData)
{
    return;
}
```

## 로깅

```cpp
// ❌ 일반 UE_LOG 사용
UE_LOG(LogTemp, Warning, TEXT("..."));

// ✅ 프로젝트 매크로 사용 (LoggingSystem.h 참조)
KHS_WARN(TEXT("SkillData is null"));
```

## NULL 체크 계층

| 레벨 | 사용처 | 코드 |
|------|--------|------|
| L1 — 일반 유효성 | 런타임 null 가능 | `if (!ptr) { KHS_WARN(...); return; }` |
| L2 — 필수 존재 | 반드시 있어야 함 | `check(ptr)` 또는 `ensureMsgf(ptr, ...)` |

## 설계 원칙

- **SOLID 원칙** 준수 — 단일 책임, 개방/폐쇄, 리스코프, 인터페이스 분리, 의존성 역전
- **비즈니스 로직 / 표현-연출 로직** 필수 분리
- **조합 메서드 패턴** — 함수는 한 가지 일만
- **하드코딩 금지** — 수치와 규칙은 항상 DataTable 또는 EditDefaultsOnly

## GC 참조 규칙

- UObject* / Actor* 멤버 변수 → `UPROPERTY()` 필수
- `FGameplayAbilitySpec.SourceObject` → 약참조(Weak) — 런타임 데이터 보관 불가
- 런타임 데이터 오브젝트 → `UPROPERTY()` 강참조

## Private 헬퍼 배치 원칙

```
"이 함수가 이 클래스의 동작을 설명하는가?" → YES → 헤더 private 선언
"단순 데이터 변환/조회인가?"              → 조건부 → anonymous namespace 가능

anonymous namespace 허용 조건:
  - 해당 클래스와 무관한 순수 유틸리티
  - 클래스 멤버에 접근하지 않음
  - 2개+ 클래스 공유 가능성 → 별도 헤더로 분리
```

## 네트워크 코드

- 싱글플레이어 프로젝트 — 네트워크 코드는 주석 처리 유지
- 네트워킹 관련 RPC 함수 추가 금지

## CSV / DataTable

- CSV 인코딩: **UTF-8-BOM** 필수

## 새 요소 추가 체크리스트

코드 작성 전 반드시 확인:
- [ ] CSV/DataTable 수정 필요 여부
- [ ] 새 C++ 클래스 생성 필요 여부
- [ ] 기존 클래스 수정 파일 목록
- [ ] Gameplay Tag 추가 필요 여부
- [ ] 에디터 설정 (BP/에셋) 필요 여부
