# PROJECT VANGUARD — CLAUDE.md

## 프로젝트
- RoastStaffGAS: 쿼터뷰 디펜스 RPG 로그라이크 (UE5 C++, GAS, 싱글플레이어)
- 아키텍처: 데이터 드리븐 — CSV → DataTable → Subsystem → GA
- 네트워킹: 싱글플레이어 전용 (Replication 코드 비활성)

## 핵심 서브시스템
- GDS(UGameDataSubsystem): 정적 DataTable 로드/캐싱/조회 허브
- SGS(USaveGameSubsystem): 영구 저장 데이터 관리
- RDS(URuntimeDataSubsystem): 세션 내 동적 상태 (해금, 슬롯, 선택 캐릭터)
- UMS(UIManagerSubsystem): 위젯 생명주기, 레이어, 팝업 스택 관리

## 절대 규칙
1. 모든 구현 작업은 파이프라인을 따른다: PLAN → CROSS-REVIEW → 승인 → CODE(+셀프리뷰) → 승인 → (선택)TEST → (선택)SENIOR-REVIEW → (선택)LEARN → 커밋 요청 → 승인
2. 기획서(`_Design/Systems/`)를 읽지 않고 코드를 작성하지 않는다.
3. 기획서와 충돌 시 "기획서 ○○의 ○○ 규칙과 충돌합니다" 명시 후 선택지 제시.
4. 하드코딩 금지. 수치/규칙은 DataTable 또는 외부 데이터에서 참조.
5. 모든 if문은 중괄호 필수. 인라인 return 금지.
6. 새 개념 등장 시 코드 전 Q&A로 개념 먼저 설명 (학습 모드).
7. CODE 단계 승인 후, TEST나 SENIOR-REVIEW, LEARN 단계는 실행여부를 반드시 사용자에게 승인을 요청한다. 호출 전에 `/compact`를 실행하여 컨텍스트를 정리한다.
8. 새 세션 시작 시 `_Design/Handoff/HANDOFF_LATEST.md`를 확인하여 이전 작업 컨텍스트를 이어받는다.

## 코딩 컨벤션 참조
- 상세 컨벤션: `.claude/skills/coding/references/conventions.md`
- 예외처리 계층/GAS 패턴/GC 참조 규칙 모두 해당 파일 참조.

## 기획서 참조
- 기획서 위치: `_Design/Systems/`, `_Design/_Overview/`
- 기획서 목록은 스킬이 자동으로 탐색. CLAUDE.md에 목록 유지하지 않음.

## 에이전트/스킬 구조
- 스킬: `/planning`, `/coding`, `/test`, `/sync-doc`, `/update-design`
- 에이전트: `@cross-reviewer`, `@senior-reviewer`, `@senior-reviewer-full`, `@learning-coach`

## 세션 종료 프로토콜                                                          
 사용자가 "세션 종료", "종료할게", "그만할게", "세션 종료하고", "핸드오프 작성하자" 등을 말하면:                           
 1. `touch "$CLAUDE_PROJECT_DIR/.claude/.session_end_flag"` 실행
 2. 핸드오프 문서를 직접 작성하지 않는다. Stop Hook이 템플릿을 생성하고 채우기를 요청한다.

## 새 세션 시작 시 (Worktree 병합 후)
1. `_Design/Handoff/HANDOFF_LATEST.md` 확인
2. 명령: "Main handoff 업데이트: Worktree별 완료 내용을 통합해줘"
3. Claude가 자동으로 다음 포맷으로 통합:

### Main Handoff 통합 포맷
```
# Main Handoff — [Timestamp]

## 최근 Worktree 작업 통합
### [Worktree 1 작업 이름]
- 완료: (핵심 사항)
- 변경: (수정 파일)

### [Worktree 2 작업 이름]
- 완료: (핵심 사항)
- 변경: (수정 파일)

## 다음 작업 계획
(Planning에서 결정할 사항)
```

4. 통합 완료 후 Planning 시작