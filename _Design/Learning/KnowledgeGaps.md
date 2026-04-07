# KnowledgeGaps — 누적 공백 인덱스
> EXPLAIN_IMPL 단계에서 "몰라 / 애매해" 응답 누적.
> @learning-coach가 이 파일을 읽어 학습 순서 제안에 활용.

## 상태 키
```
🔴 미숙   — 2회 이상 몰라/애매해
🟡 학습중 — 1회 몰라/애매해
✅ 확인됨 — 이후 세션에서 "알아"로 응답
```

---

## 공백 인덱스

| 키워드 | 분류 | 횟수 | 최근 날짜 | 상태 | 출처 모듈 |
|--------|------|------|----------|------|----------|
| BT 4종 노드 구조 (Composite/Task/Decorator/Service) | UE AI 아키텍처 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7 |
| BT Decorator를 Composite 노드에 붙이는 방법 (우클릭 → Add Decorator) | UE 에디터 조작 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7 |
| BTTaskNode 생명주기 (InProgress/TickTask/FinishLatentTask) | UE AI 패턴 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7 |
| BTDecorator (CalculateRawConditionValue / Observer Abort) | UE AI 패턴 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7 |
| BTService — Blackboard 주기적 갱신 역할 | UE AI 패턴 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7 |
| NodeMemory — uint8* 버퍼 / GetInstanceMemorySize 패턴 | UE AI 패턴 | 1 | 2026-04-07 | ✅ 확인됨 | MODULE-7 |
| AutoPossessAI — 4가지 값의 의미 (Disabled/PlacedInWorld/Spawned/PlacedInWorldOrSpawned) | UE AI 아키텍처 | 2 | 2026-04-07 | 🔴 미숙 | MODULE-7 / MODULE-7-debug |
| KHS_* 로그 레벨 매핑 — KHS_DEBUG=Verbose 에디터 필터링됨, 진단 시 KHS_INFO 사용 | UE 로깅 시스템 | 2 | 2026-04-07 | 🔴 미숙 | MODULE-7 / MODULE-7-debug |
| Instigator pattern — SetInstigator(Owner) + IgnoreActorWhenMoving으로 자기 충돌 방지 | UE 투사체 패턴 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7-debug |
| FTransform constructor Scale trap — FTransform(Rot, Loc) 기본 Scale=(1,1,1) BP Scale 덮어씀 | UE5 스폰 패턴 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7-debug |
| TWeakObjectPtr lambda capture — 레벨 전환 시 dangling this 방지 | C++ 메모리 안전 | 1 | 2026-04-07 | 🟡 학습중 | MODULE-7-debug |

---

## KnowledgeCheck 파일 목록
> 세션별 상세 응답은 아래 파일에 기록됨.

| 파일 | 날짜 | 모듈 |
|------|------|------|
| KnowledgeCheck_2026-04-07_MODULE-7.md | 2026-04-07 | MODULE-7 |
| KnowledgeCheck_2026-04-07_MODULE-7-debug.md | 2026-04-07 | MODULE-7 디버깅 |
