# CONSTRAINTS
> 이 프로젝트의 불변 제약. INIT마다 로드.

```
1. 파이프라인 강제: PLAN → [GAME-DESIGN?] → CODE → TEST → SR → LEARN → COMMIT
2. 기획서(_Design/References/Systems/) 또는 계획서(_Design/Plans/active/) 없이 코드 작성 금지
3. 기획서 충돌 시: 중단 → "기획서 ○○의 ○○ 규칙과 충돌" → 선택지 제시
4. 코딩 규칙: .claude/skills/coding/refs/conventions.md 준수
5. TEST: 빌드 성공 후 자동 실행
   SR: TEST 완료 후 자동 실행 (규모 기준 — 상세: protocols.md#ORCHESTRATOR_FLOW)
       HIGH [CODE] → 자동 수정 후 재검증 루프 (최대 2회) / HIGH [ARCH] → 시니 게이트
   LEARN: SR 완료 후 자동 실행 (이슈 기준 — 상세: protocols.md#ORCHESTRATOR_FLOW)
6. 3회 실패 → BLOCKED 선언
7. 무거운 작업 시작 전 COST_POLICY 확인 (.claude/refs/protocols.md#COST_POLICY)
8. git commit: "커밋해줘" 없이 실행 금지. 커밋 계획 제안 → 시니 승인 → 커밋+푸시 자동 실행
9. 작업 완료 후 결정 근거 공개 (선택지 + 채택/기각 이유)
10. 세션 마무리 역질문 1개 — 시니의 사고를 자극하는 질문
```
