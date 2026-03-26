#!/bin/bash

##############################################################################
# Git Worktree + Claude Code Automation Scripts
# RoastStaffGAS 프로젝트 전용
##############################################################################

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

##############################################################################
# 1. worktree-create.sh - 새 worktree 생성 & Claude Code 초기화
##############################################################################

create_worktree() {
    local FEATURE_NAME=$1
    local BASE_BRANCH=${2:-"main"}
    
    if [ -z "$FEATURE_NAME" ]; then
        echo -e "${RED}Error: Feature name required${NC}"
        echo "Usage: ./worktree-create.sh <feature-name> [base-branch]"
        echo "Example: ./worktree-create.sh enemy-healthbar"
        return 1
    fi
    
    # 변수 설정
    REPO_ROOT=$(git rev-parse --show-toplevel)
    WORKTREE_NAME="WT-${FEATURE_NAME}"
    WORKTREE_PATH="$C:\Users\KGA\Projects\RoastStaffGAS-${FEATURE_NAME}"
    BRANCH_NAME="feature/${FEATURE_NAME}"
    
    echo -e "${BLUE}=== Creating Worktree ===${NC}"
    echo "Repository: $REPO_ROOT"
    echo "Worktree path: $WORKTREE_PATH"
    echo "Branch: $BRANCH_NAME"
    echo "Base: $BASE_BRANCH"
    
    # 1. git pull로 최신 유지
    echo -e "\n${YELLOW}[1/5] Updating main branch...${NC}"
    git checkout "$BASE_BRANCH" || { echo -e "${RED}Failed to checkout $BASE_BRANCH${NC}"; return 1; }
    git pull origin "$BASE_BRANCH" || { echo -e "${RED}Failed to pull${NC}"; return 1; }
    
    # 2. worktree 생성
    echo -e "\n${YELLOW}[2/5] Creating worktree...${NC}"
    if git worktree add --track -b "$BRANCH_NAME" "origin/$BASE_BRANCH" "$WORKTREE_PATH"; then
        echo -e "${GREEN}Worktree created successfully${NC}"
    else
        echo -e "${RED}Failed to create worktree${NC}"
        return 1
    fi
    
    # 3. 디렉토리 이동
    echo -e "\n${YELLOW}[3/5] Navigating to worktree...${NC}"
    cd "$WORKTREE_PATH" || { echo -e "${RED}Failed to enter worktree${NC}"; return 1; }
    echo -e "${GREEN}Current directory: $(pwd)${NC}"
    
    # 4. git 상태 확인
    echo -e "\n${YELLOW}[4/5] Verifying git status...${NC}"
    echo "Branch: $(git branch --show-current)"
    echo "Worktree list:"
    git worktree list
    
    # 5. Claude Code 초기화 (선택사항)
    echo -e "\n${YELLOW}[5/5] Claude Code readiness check...${NC}"
    if [ -f ".claude/CLAUDE.md" ]; then
        echo -e "${GREEN}.claude/CLAUDE.md found${NC}"
        echo "Ready to run: claude-code --plan"
    else
        echo -e "${YELLOW}Warning: .claude/CLAUDE.md not found${NC}"
        echo "Copy from main: cp ../RoastStaffGAS/.claude/CLAUDE.md .claude/"
    fi
    
    echo -e "\n${GREEN}=== Worktree creation complete ===${NC}"
    echo -e "Next step: ${BLUE}cd $WORKTREE_PATH && claude-code --plan${NC}"
}

##############################################################################
# 2. worktree-list.sh - 모든 worktree 상태 표시
##############################################################################

list_worktrees() {
    echo -e "${BLUE}=== Git Worktrees ===${NC}"
    
    REPO_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
    if [ -z "$REPO_ROOT" ]; then
        echo -e "${RED}Not in a git repository${NC}"
        return 1
    fi
    
    echo -e "Repository: ${GREEN}$REPO_ROOT${NC}\n"
    
    # worktree 목록
    git worktree list --porcelain | while IFS= read -r line; do
        if [[ $line == worktree* ]]; then
            path="${line#worktree }"
            branch_line=$(git -C "$path" symbolic-ref --short HEAD 2>/dev/null || echo "(detached)")
            
            # 변경사항 확인
            changes=$(cd "$path" && git status --porcelain | wc -l)
            
            # 색상 지정
            if [[ $path == "$REPO_ROOT" ]]; then
                status_color="${GREEN}"
            else
                status_color="${BLUE}"
            fi
            
            echo -e "${status_color}$(basename $path)${NC}"
            echo "  Path: $path"
            echo "  Branch: $branch_line"
            echo "  Changes: $changes files"
            echo ""
        fi
    done
}

##############################################################################
# 3. worktree-remove.sh - worktree 안전 삭제
##############################################################################

remove_worktree() {
    local FEATURE_NAME=$1
    
    if [ -z "$FEATURE_NAME" ]; then
        echo -e "${RED}Error: Feature name required${NC}"
        echo "Usage: ./worktree-remove.sh <feature-name>"
        echo "Example: ./worktree-remove.sh enemy-healthbar"
        list_worktrees
        return 1
    fi
    
    REPO_ROOT=$(git rev-parse --show-toplevel)
    WORKTREE_PATH="${REPO_ROOT}/../RoastStaffGAS-${FEATURE_NAME}"
    BRANCH_NAME="feature/${FEATURE_NAME}"
    
    if [ ! -d "$WORKTREE_PATH" ]; then
        echo -e "${RED}Worktree not found: $WORKTREE_PATH${NC}"
        return 1
    fi
    
    echo -e "${YELLOW}=== Removing Worktree ===${NC}"
    echo "Worktree: $WORKTREE_PATH"
    echo "Branch: $BRANCH_NAME"
    
    # 변경사항 확인
    echo -e "\n${YELLOW}Checking for uncommitted changes...${NC}"
    cd "$WORKTREE_PATH" || return 1
    
    if [ ! -z "$(git status --porcelain)" ]; then
        echo -e "${RED}Uncommitted changes detected:${NC}"
        git status --short
        echo -e "\n${YELLOW}Options:${NC}"
        echo "1. Commit changes: git commit -am 'message'"
        echo "2. Stash changes: git stash"
        echo "3. Force remove (discard changes): ./worktree-remove.sh $FEATURE_NAME --force"
        return 1
    fi
    
    # worktree 삭제
    echo -e "\n${YELLOW}Removing worktree...${NC}"
    cd "$REPO_ROOT"
    
    if git worktree remove "$WORKTREE_PATH"; then
        echo -e "${GREEN}Worktree removed${NC}"
    else
        echo -e "${RED}Failed to remove worktree${NC}"
        return 1
    fi
    
    # branch 삭제 (선택사항)
    echo -e "\n${YELLOW}Delete branch '$BRANCH_NAME'? (y/n)${NC}"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        git branch -d "$BRANCH_NAME" 2>/dev/null && echo -e "${GREEN}Branch deleted${NC}"
    fi
    
    # 정리
    echo -e "\n${YELLOW}Pruning orphaned worktrees...${NC}"
    git worktree prune
    
    echo -e "\n${GREEN}=== Cleanup complete ===${NC}"
}

##############################################################################
# 4. worktree-sync.sh - main의 최신 변경을 worktree에 반영
##############################################################################

sync_worktree() {
    local FEATURE_NAME=$1
    local STRATEGY=${2:-"rebase"}  # rebase or merge
    
    if [ -z "$FEATURE_NAME" ]; then
        echo -e "${RED}Error: Feature name required${NC}"
        return 1
    fi
    
    REPO_ROOT=$(git rev-parse --show-toplevel)
    WORKTREE_PATH="${REPO_ROOT}/../RoastStaffGAS-${FEATURE_NAME}"
    
    if [ ! -d "$WORKTREE_PATH" ]; then
        echo -e "${RED}Worktree not found${NC}"
        return 1
    fi
    
    echo -e "${BLUE}=== Syncing Worktree ===${NC}"
    echo "Worktree: $FEATURE_NAME"
    echo "Strategy: $STRATEGY"
    
    cd "$WORKTREE_PATH" || return 1
    
    # 1. 현재 상태 확인
    echo -e "\n${YELLOW}[1/3] Current status:${NC}"
    echo "Branch: $(git branch --show-current)"
    echo "Changes: $(git status --porcelain | wc -l) files"
    
    # 2. main에서 최신 가져오기
    echo -e "\n${YELLOW}[2/3] Fetching latest from origin/main...${NC}"
    git fetch origin main
    
    # 3. Rebase or Merge
    echo -e "\n${YELLOW}[3/3] Performing $STRATEGY...${NC}"
    
    if [ "$STRATEGY" = "rebase" ]; then
        git rebase origin/main && echo -e "${GREEN}Rebase successful${NC}" || {
            echo -e "${RED}Rebase conflict detected${NC}"
            echo "Resolve conflicts and run: git rebase --continue"
            return 1
        }
    else
        git merge origin/main && echo -e "${GREEN}Merge successful${NC}" || {
            echo -e "${RED}Merge conflict detected${NC}"
            echo "Resolve conflicts and run: git add . && git commit"
            return 1
        }
    fi
    
    echo -e "\n${GREEN}=== Sync complete ===${NC}"
}

##############################################################################
# 5. claude-code-parallel.sh - 여러 worktree에서 동시 Claude Code 실행
##############################################################################

claude_code_parallel() {
    local STAGE=$1  # plan, code, test, etc.
    
    if [ -z "$STAGE" ]; then
        echo -e "${RED}Error: Stage required${NC}"
        echo "Usage: ./claude-code-parallel.sh <stage>"
        echo "Stages: plan, code, test, review, handoff"
        return 1
    fi
    
    echo -e "${BLUE}=== Running Claude Code Parallel ===${NC}"
    echo "Stage: $STAGE"
    
    REPO_ROOT=$(git rev-parse --show-toplevel)
    
    # 모든 worktree 찾기
    worktrees=()
    git worktree list --porcelain | while IFS= read -r line; do
        if [[ $line == worktree* ]]; then
            path="${line#worktree }"
            if [[ "$path" != "$REPO_ROOT" ]]; then
                worktrees+=("$path")
            fi
        fi
    done
    
    if [ ${#worktrees[@]} -eq 0 ]; then
        echo -e "${YELLOW}No worktrees found${NC}"
        return 1
    fi
    
    echo -e "\n${YELLOW}Found ${#worktrees[@]} worktree(s)${NC}"
    
    # 각 worktree에서 Claude Code 실행 (순차)
    # (병렬로 하면 token limit 초과 가능)
    for worktree in "${worktrees[@]}"; do
        if [ -d "$worktree" ]; then
            echo -e "\n${BLUE}Processing: $(basename $worktree)${NC}"
            cd "$worktree" || continue
            
            # Claude Code 실행
            case "$STAGE" in
                plan)
                    echo "Running: claude-code --plan"
                    claude-code --plan || echo -e "${RED}Failed${NC}"
                    ;;
                code)
                    echo "Running: claude-code --code"
                    claude-code --code || echo -e "${RED}Failed${NC}"
                    ;;
                test)
                    echo "Running: claude-code --test"
                    claude-code --test || echo -e "${RED}Failed${NC}"
                    ;;
                *)
                    echo -e "${YELLOW}Unknown stage: $STAGE${NC}"
                    ;;
            esac
        fi
    done
    
    echo -e "\n${GREEN}=== Parallel execution complete ===${NC}"
}

##############################################################################
# 6. worktree-merge-safe.sh - 안전한 merge with conflict detection
##############################################################################

merge_safe() {
    local FEATURE_NAME=$1
    
    if [ -z "$FEATURE_NAME" ]; then
        echo -e "${RED}Error: Feature name required${NC}"
        return 1
    fi
    
    REPO_ROOT=$(git rev-parse --show-toplevel)
    BRANCH_NAME="feature/${FEATURE_NAME}"
    
    echo -e "${BLUE}=== Safe Merge ===${NC}"
    echo "Branch: $BRANCH_NAME"
    
    cd "$REPO_ROOT" || return 1
    
    # 1. main 최신화
    echo -e "\n${YELLOW}[1/5] Updating main...${NC}"
    git checkout main
    git pull origin main || { echo -e "${RED}Pull failed${NC}"; return 1; }
    
    # 2. conflict 사전 확인 (dry-run)
    echo -e "\n${YELLOW}[2/5] Checking for conflicts (dry-run)...${NC}"
    git merge --no-commit --no-ff "$BRANCH_NAME" 2>&1 | tee /tmp/merge-check.log
    
    if grep -q "CONFLICT" /tmp/merge-check.log; then
        echo -e "${RED}Merge conflict detected!${NC}"
        git merge --abort
        return 1
    fi
    
    echo -e "${GREEN}No conflicts detected${NC}"
    git merge --abort
    
    # 3. 변경사항 리뷰
    echo -e "\n${YELLOW}[3/5] Changes to be merged:${NC}"
    git log --oneline main.."$BRANCH_NAME"
    
    echo -e "\n${YELLOW}Proceed with merge? (y/n)${NC}"
    read -r response
    if [[ ! "$response" =~ ^[Yy]$ ]]; then
        echo "Merge cancelled"
        return 1
    fi
    
    # 4. 실제 merge
    echo -e "\n${YELLOW}[4/5] Merging...${NC}"
    git merge --no-ff "$BRANCH_NAME" -m "Merge $BRANCH_NAME into main"
    
    # 5. push
    echo -e "\n${YELLOW}[5/5] Pushing to origin...${NC}"
    git push origin main || {
        echo -e "${RED}Push failed - rolling back${NC}"
        git reset --hard HEAD~1
        return 1
    }
    
    echo -e "\n${GREEN}=== Merge successful ===${NC}"
}

##############################################################################
# 메인 실행 (스크립트로 호출)
##############################################################################

# 스크립트 사용 예시
if [ "$#" -gt 0 ]; then
    COMMAND=$1
    shift  # Remove first argument
    
    case "$COMMAND" in
        create)
            create_worktree "$@"
            ;;
        list)
            list_worktrees "$@"
            ;;
        remove)
            remove_worktree "$@"
            ;;
        sync)
            sync_worktree "$@"
            ;;
        claude)
            claude_code_parallel "$@"
            ;;
        merge)
            merge_safe "$@"
            ;;
        *)
            echo -e "${RED}Unknown command: $COMMAND${NC}"
            echo ""
            echo "Available commands:"
            echo "  create <feature-name>    - Create new worktree"
            echo "  list                     - List all worktrees"
            echo "  remove <feature-name>    - Remove worktree"
            echo "  sync <feature-name>      - Sync with origin/main"
            echo "  claude <stage>           - Run Claude Code in all worktrees"
            echo "  merge <feature-name>     - Safe merge to main"
            exit 1
            ;;
    esac
else
    echo -e "${BLUE}Git Worktree + Claude Code Helper${NC}"
    echo ""
    echo "Usage: source this script or run individual scripts"
    echo ""
    echo "Functions available:"
    echo "  create_worktree <feature-name>"
    echo "  list_worktrees"
    echo "  remove_worktree <feature-name>"
    echo "  sync_worktree <feature-name> [strategy]"
    echo "  claude_code_parallel <stage>"
    echo "  merge_safe <feature-name>"
fi
