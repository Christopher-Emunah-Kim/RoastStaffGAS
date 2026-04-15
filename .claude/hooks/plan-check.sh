#!/bin/bash
# .claude/hooks/plan-check.sh
# PreToolUse — Write|Edit|MultiEdit 시 Source/ 경로 수정 전:
#   1) 활성 계획서 존재 확인
#   2) 편집 파일이 계획서 범위 안에 있는지 확인 (스코프 이탈 차단)

INPUT=$(cat)
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // .tool_input.path // ""' 2>/dev/null)

# jq 없으면 fallback
if [ -z "$FILE_PATH" ]; then
    FILE_PATH=$(echo "$INPUT" | grep -oE '"file_path"[[:space:]]*:[[:space:]]*"[^"]*"' \
        | sed 's/"file_path"[[:space:]]*:[[:space:]]*"//;s/"$//' | head -1)
fi

# Source/ 경로가 아니면 통과
if [[ "$FILE_PATH" != *"/Source/"* ]] && [[ "$FILE_PATH" != *"\\Source\\"* ]]; then
    exit 0
fi

PLAN_DIR="$CLAUDE_PROJECT_DIR/_Design/Plans/active"
LATEST_PLAN=$(ls -t "$PLAN_DIR"/PLAN_*.md 2>/dev/null | head -1)

# ① 계획서 없음 → 차단
if [ -z "$LATEST_PLAN" ]; then
    echo '{"decision": "block", "reason": "⛔ [PLAN_REQUIRED] Source/ 수정 전 활성 계획서 필요. /planning으로 계획서를 먼저 작성하세요."}'
    exit 0
fi

# ② 파일이 계획서 범위 안에 있는지 확인
# Source/ 이후 상대 경로 추출 (Windows \ 와 Unix / 모두 처리)
REL_PATH=$(echo "$FILE_PATH" | sed 's/\\/\//g' | grep -oE 'Source/.*$')
BASENAME=$(basename "$FILE_PATH")

# PLAN에서 파일 언급 여부 확인 (경로 or 파일명)
if [ -n "$REL_PATH" ]; then
    IN_PLAN=$(grep -F "$REL_PATH" "$LATEST_PLAN" 2>/dev/null)
    [ -z "$IN_PLAN" ] && IN_PLAN=$(grep -F "$BASENAME" "$LATEST_PLAN" 2>/dev/null)
else
    IN_PLAN=$(grep -F "$BASENAME" "$LATEST_PLAN" 2>/dev/null)
fi

if [ -z "$IN_PLAN" ]; then
    PLAN_NAME=$(basename "$LATEST_PLAN")
    echo "{\"decision\": \"block\", \"reason\": \"⛔ [SCOPE_VIOLATION] '$BASENAME' 은 활성 계획서 '$PLAN_NAME' 범위 밖입니다. PLAN의 신규/수정 파일 목록에 추가하거나, /planning으로 범위를 먼저 확정하세요.\"}"
    exit 0
fi

exit 0
