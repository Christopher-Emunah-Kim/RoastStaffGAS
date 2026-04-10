#!/bin/bash
# .claude/hooks/plan-check.sh
# PreToolUse — Write|Edit|MultiEdit 시 Source/ 경로 수정 전 활성 계획서 존재 확인

INPUT=$(cat)
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // .tool_input.path // ""')

# Source/ 경로가 아니면 통과
if [[ "$FILE_PATH" != *"/Source/"* ]]; then
  exit 0
fi

# 활성 계획서 존재 확인
PLAN_DIR="$CLAUDE_PROJECT_DIR/_Design/Plans/active"
PLAN_COUNT=$(ls "$PLAN_DIR"/PLAN_*.md 2>/dev/null | wc -l)

if [ "$PLAN_COUNT" -eq 0 ]; then
  echo '{"decision": "block", "reason": "⛔ [PLAN_REQUIRED] Source/ 수정 전 활성 계획서 필요. _Design/Plans/active/에 PLAN_*.md 없음. /planning으로 계획서를 먼저 작성하세요."}'
  exit 0
fi

exit 0
