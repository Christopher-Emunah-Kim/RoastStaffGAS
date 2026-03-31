# PROJECT — RoastStaffGAS
> 프로젝트 정보 허브. CLAUDE.md에서 참조.

## 프로젝트 개요
```yaml
name:  RoastStaffGAS
genre: 쿼터뷰 디펜스 RPG 로그라이크
tech:  UE5 C++, GAS
mode:  싱글플레이어 전용 (Replication 비활성)
arch:  CSV → DataTable → Subsystem → GA
```

## 핵심 서브시스템
```yaml
GDS: UGameDataSubsystem    # 정적 DataTable 로드/캐싱/조회 허브
SGS: USaveGameSubsystem    # 영구 저장 데이터 관리
RDS: URuntimeDataSubsystem # 세션 내 동적 상태 (해금, 슬롯, 선택 캐릭터)
UMS: UIManagerSubsystem    # 위젯 생명주기, 레이어, 팝업 스택
```

## 데이터 흐름
```
CSV (ExternalSource/) → DataTable (콘텐츠 브라우저)
  → GDS.GetXxx() → Subsystem → GA 적용
```

## 폴더 구조
```
_Design/
  References/
    README.md          ← 이 파일
    Systems/           ← 기획서 (시스템별 상세 규칙)
  Plans/
    active/            ← 진행 중 계획서 (PLAN_*.md)
    completed/         ← 완료된 계획서
  Changesets/
    CHANGESET.md       ← 에이전트용 코드 변화 추적
  Handoff/
    HANDOFF_LATEST.md  ← 세션 간 컨텍스트 이어받기
  Learning/
    LEARNING_LOG.md    ← 학습 이력
    reports/           ← 세션별 학습 리포트
  Reviews/             ← 시니어 리뷰 결과
  TODO.md              ← 살아있는 작업 이력 (최우선 읽기)

.claude/
  references/          ← 시스템 운영 규칙 (protocols, commit 등)
  skills/              ← Skill 파일들
  agents/              ← Agent 파일들
  skills/coding/references/  ← 코딩 컨벤션, OOP 원칙
```

## 기획서 목록 (_Design/References/Systems/)
> planning-architect가 기획서 탐색 시 이 목록을 우선 참조

| 파일명 | 내용 |
|--------|------|
| 게임 플로우 아키텍처 기획 v1.0 | 전체 서브시스템 토폴로지, 게임플로우 |
| 스킬 시스템 기획 v1.4.md | GA, GE, DataTable 스킬 구조 |
| 투사체 시스템 기획 v1.3.md | 투사체 이동/충돌/히트 타입 |
| 무기 시스템 기획 v1.2.md | 무기 장착/스왑/데이터 |
| 캐릭터 시스템 기획 v1.2.md | 캐릭터 스탯/능력치/성장 |
| 플레이어 조작 시스템 기획 v1.2.md | 입력/이동/카메라 |
| 레벨업 시스템 기획 v1.2.md | 경험치/레벨/보상 |
| 게임 데이터 시스템 기획 v1.1.md | GDS 구조/캐싱 |
| 런타임 데이터 시스템 기획 v1.0.md | RDS 구조 |
| 세이브 데이터 시스템 기획 v1.1.md | SGS 구조 |
| UI관리 시스템 기획 v1.0.md | UMS 구조 |
| AI_에너미 시스템 기획 v1.1.md | AI 행동/스폰/공격 |
