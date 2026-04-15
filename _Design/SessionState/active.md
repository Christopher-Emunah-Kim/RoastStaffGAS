# Session State — Active
> PreCompact 훅이 자동 갱신. 압축 후 컨텍스트 복원에 사용.

## Current Task
(없음 — 세션 시작 시 업데이트)

## Progress
<!-- 완료된 단계 체크 -->
- [x] PLAN
- [ ] CODE
- [ ] TEST
- [ ] SR
- [ ] COMMIT

## Key Decisions
- PassiveSlotWidget: Hidden(not Collapsed), 8슬롯 프리할당, max4 채움
- 툴팁: 아이콘 위쪽 고정, DisplayName + Description
- hover: Btn_PassiveSlot OnHovered/OnUnhovered → Bdr_Tooltip 토글
- PC OnPassiveSlotChanged stub → 기존 RefreshSkillSlotUI 패턴 동일 구현

## Files In Progress
- Source/RoastStaffGAS/Public/UI/InGame/PassiveSlotWidget.h (신규)
- Source/RoastStaffGAS/Private/UI/InGame/PassiveSlotWidget.cpp (신규)
- Source/RoastStaffGAS/Public/UI/InGame/SlotContainerWidget.h (수정)
- Source/RoastStaffGAS/Private/UI/InGame/SlotContainerWidget.cpp (수정)
- Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp (수정)

## Open Questions
<!-- 미결 질문 / 다음 세션 이슈 -->
