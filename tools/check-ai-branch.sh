#!/bin/bash
#
# Check AI Branch Status Script
# Analyzes current branch for AI involvement and provides recommendations
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Get current branch info
CURRENT_BRANCH=$(git symbolic-ref --short HEAD 2>/dev/null || echo "detached HEAD")
CURRENT_COMMIT=$(git rev-parse HEAD)
CURRENT_AUTHOR=$(git config user.name)
CURRENT_EMAIL=$(git config user.email)

echo -e "${BLUE}=== Current Branch AI Status ===${NC}"
echo -e "Branch: ${CYAN}${CURRENT_BRANCH}${NC}"
echo -e "Current commit: ${CURRENT_COMMIT:0:8}"
echo -e "Git author: ${CURRENT_AUTHOR} <${CURRENT_EMAIL}>"
echo ""

# Check if branch name suggests AI involvement
IS_AI_BRANCH=false
if echo "$CURRENT_BRANCH" | grep -qE "^(claude|ai|ai-assist)/"; then
    IS_AI_BRANCH=true
    echo -e "${GREEN}✅ AI branch detected${NC} (based on naming convention)"
else
    echo -e "${YELLOW}ℹ️  Regular branch${NC} (no AI naming pattern)"
fi

# Check git author for AI indicators
if echo "$CURRENT_AUTHOR" | grep -qE "(Claude|AI|Bot)"; then
    echo -e "${GREEN}✅ AI author detected${NC} in git config"
elif echo "$CURRENT_EMAIL" | grep -qE "(claude|ai|bot)"; then
    echo -e "${GREEN}✅ AI email detected${NC} in git config"
else
    echo -e "${YELLOW}ℹ️  Human author${NC} in git config"
fi

echo ""

# Analyze recent commits on this branch
echo -e "${PURPLE}=== Recent Commits Analysis ===${NC}"

# Get commits since branching from main
MERGE_BASE=$(git merge-base HEAD main 2>/dev/null || git merge-base HEAD master 2>/dev/null || echo "")

if [ -n "$MERGE_BASE" ]; then
    COMMITS_SINCE_BRANCH=$(git rev-list --count ${MERGE_BASE}..HEAD)
    AI_COMMITS_SINCE_BRANCH=$(git log --grep="Claude|AI|🤖" --oneline ${MERGE_BASE}..HEAD | wc -l)
    
    echo -e "Commits since branching: ${COMMITS_SINCE_BRANCH}"
    echo -e "AI-attributed commits: ${AI_COMMITS_SINCE_BRANCH}"
    
    if [ $COMMITS_SINCE_BRANCH -gt 0 ]; then
        AI_PERCENTAGE=$(echo "scale=1; $AI_COMMITS_SINCE_BRANCH * 100 / $COMMITS_SINCE_BRANCH" | bc)
        echo -e "AI percentage: ${AI_PERCENTAGE}%"
    fi
    
    echo ""
    
    # Show recent commits with AI indicators
    echo -e "${CYAN}Recent commits:${NC}"
    git log --oneline --decorate -5 ${MERGE_BASE}..HEAD | while read line; do
        if echo "$line" | grep -q "🤖\|Claude\|AI"; then
            echo -e "  ${GREEN}[AI]${NC} $line"
        else
            echo -e "  ${YELLOW}[HU]${NC} $line"
        fi
    done
else
    echo "⚠️  Cannot determine branch point - showing last 5 commits"
    git log --oneline --decorate -5 | while read line; do
        if echo "$line" | grep -q "🤖\|Claude\|AI"; then
            echo -e "  ${GREEN}[AI]${NC} $line"
        else
            echo -e "  ${YELLOW}[HU]${NC} $line"
        fi
    done
fi

echo ""

# Check for uncommitted changes
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo -e "${YELLOW}⚠️  Uncommitted changes detected${NC}"
    echo "   - Unstaged changes: $(git diff --name-only | wc -l) files"
    echo "   - Staged changes: $(git diff --cached --name-only | wc -l) files"
    echo ""
fi

# Provide recommendations
echo -e "${RED}=== Recommendations ===${NC}"

if [ "$IS_AI_BRANCH" = true ]; then
    echo -e "${GREEN}✅ Branch follows AI naming convention${NC}"
    if [ $AI_COMMITS_SINCE_BRANCH -eq 0 ]; then
        echo -e "${YELLOW}⚠️  No AI-attributed commits found on AI branch${NC}"
        echo "   Consider adding AI attribution to commits or renaming branch"
    fi
else
    if [ $AI_COMMITS_SINCE_BRANCH -gt 0 ]; then
        echo -e "${YELLOW}⚠️  AI commits found on non-AI branch${NC}"
        echo "   Consider renaming branch to: claude/$(echo $CURRENT_BRANCH | sed 's/^[a-z]*\///')"
    fi
fi

# Git configuration recommendations
if ! echo "$CURRENT_AUTHOR" | grep -q "Claude\|AI"; then
    if [ "$IS_AI_BRANCH" = true ] || [ $AI_COMMITS_SINCE_BRANCH -gt 0 ]; then
        echo -e "${YELLOW}💡 Consider setting AI git identity:${NC}"
        echo "   git config user.name \"$CURRENT_AUTHOR + Claude\""
        echo "   git config user.email \"$(echo $CURRENT_EMAIL | sed 's/@/+claude@/')\""
    fi
fi

# Pre-commit hook status
if [ -f ".git/hooks/prepare-commit-msg" ]; then
    echo -e "${GREEN}✅ AI commit hook is installed${NC}"
else
    echo -e "${RED}❌ AI commit hook not found${NC}"
    echo "   Install with: cp tools/prepare-commit-msg .git/hooks/"
fi

# Integration readiness
echo ""
echo -e "${PURPLE}=== Integration Readiness ===${NC}"

# Check if code style is compliant
if [ -f "style/check_gnu_style.sh" ]; then
    echo "Checking code style..."
    if ./style/check_gnu_style.sh check >/dev/null 2>&1; then
        echo -e "${GREEN}✅ Code style compliant${NC}"
    else
        echo -e "${RED}❌ Code style issues found${NC}"
        echo "   Run: ./style/check_gnu_style.sh update"
    fi
else
    echo -e "${YELLOW}⚠️  Code style checker not found${NC}"
fi

# Check for AI log entry
AI_LOG_FILE=".git/ai-contributions.log"
if [ -f "$AI_LOG_FILE" ] && [ $AI_COMMITS_SINCE_BRANCH -gt 0 ]; then
    RECENT_COMMITS=$(git log --grep="Claude|AI|🤖" --pretty=format:"%H" ${MERGE_BASE}..HEAD)
    LOGGED_COMMITS=$(cat "$AI_LOG_FILE" | cut -d'|' -f2)
    
    MISSING_LOGS=0
    for commit in $RECENT_COMMITS; do
        if ! echo "$LOGGED_COMMITS" | grep -q "$commit"; then
            MISSING_LOGS=$((MISSING_LOGS + 1))
        fi
    done
    
    if [ $MISSING_LOGS -eq 0 ]; then
        echo -e "${GREEN}✅ All AI commits logged${NC}"
    else
        echo -e "${YELLOW}⚠️  ${MISSING_LOGS} AI commits missing from log${NC}"
    fi
fi

echo ""
echo -e "${BLUE}Run './tools/ai-stats.sh' for detailed AI statistics${NC}"