#!/bin/bash
# .claude/hooks/post-edit-report.sh
# PostToolUse — Write|Edit|MultiEdit 매칭 시 실행
# 매 파일 수정마다 변경 내역을 로그 파일에 기록한다.

INPUT=$(cat)
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // .tool_input.path // "unknown"')
TOOL_NAME=$(echo "$INPUT" | jq -r '.tool_name')
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

LOG_FILE="$CLAUDE_PROJECT_DIR/_Design/Reviews/CHANGE_LOG.md"

# Reviews 디렉터리가 없으면 생성
mkdir -p "$(dirname "$LOG_FILE")"

# 로그 파일이 없으면 헤더 생성
if [ ! -f "$LOG_FILE" ]; then
  cat > "$LOG_FILE" << 'EOF'
# 코드 변경 로그

| 시간 | 도구 | 파일 |
|------|------|------|
EOF
fi

echo "| $TIMESTAMP | $TOOL_NAME | \`$FILE_PATH\` |" >> "$LOG_FILE"

exit 0
