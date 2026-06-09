#!/bin/bash
#
# AI Statistics Script for sdplane-dev
# Analyzes git history to show AI contribution statistics
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

# Configuration
AI_PATTERNS="Claude|AI|🤖|ai-assist|claude/"
DATE_RANGE=${1:-"--since='1 month ago'"}

echo -e "${BLUE}=== AI Contribution Statistics ===${NC}"
echo -e "Date range: ${DATE_RANGE}"
echo ""

# Total commits
TOTAL_COMMITS=$(git log --oneline $DATE_RANGE | wc -l)
echo -e "${CYAN}Total commits in range: ${TOTAL_COMMITS}${NC}"

# AI-assisted commits
AI_COMMITS=$(git log --grep="$AI_PATTERNS" --oneline $DATE_RANGE | wc -l)
echo -e "${GREEN}AI-assisted commits: ${AI_COMMITS}${NC}"

# Calculate percentage
if [ $TOTAL_COMMITS -gt 0 ]; then
    AI_PERCENTAGE=$(echo "scale=1; $AI_COMMITS * 100 / $TOTAL_COMMITS" | bc)
    echo -e "${YELLOW}AI assistance percentage: ${AI_PERCENTAGE}%${NC}"
else
    echo -e "${YELLOW}AI assistance percentage: 0%${NC}"
fi

echo ""

# AI commits by author
echo -e "${PURPLE}=== AI Commits by Author ===${NC}"
git log --grep="$AI_PATTERNS" --pretty=format:"%an" $DATE_RANGE | sort | uniq -c | sort -nr

echo ""
echo ""

# AI commits by branch pattern
echo -e "${PURPLE}=== AI Commits by Branch Pattern ===${NC}"
git log --grep="$AI_PATTERNS" --pretty=format:"%D" $DATE_RANGE | \
    grep -E "(claude|ai|ai-assist)/" | \
    sed 's/.*\(claude\|ai\|ai-assist\)\/[^,]*/\1/g' | \
    sort | uniq -c | sort -nr

echo ""

# Recent AI commits
echo -e "${PURPLE}=== Recent AI-Assisted Commits ===${NC}"
git log --grep="$AI_PATTERNS" --oneline --decorate -10 $DATE_RANGE

echo ""

# AI contributions by file type
echo -e "${PURPLE}=== AI Contributions by File Type ===${NC}"
git log --grep="$AI_PATTERNS" --name-only --pretty=format: $DATE_RANGE | \
    grep -E '\.[a-zA-Z]+$' | \
    sed 's/.*\./\./' | \
    sort | uniq -c | sort -nr | head -10

echo ""

# Monthly AI contribution trend (last 6 months)
echo -e "${PURPLE}=== Monthly AI Contribution Trend ===${NC}"
for i in {5..0}; do
    MONTH_START=$(date -d "$i months ago" +%Y-%m-01)
    MONTH_END=$(date -d "$((i-1)) months ago" +%Y-%m-01)
    MONTH_NAME=$(date -d "$i months ago" +%B\ %Y)
    
    if [ $i -eq 0 ]; then
        MONTH_COMMITS=$(git log --grep="$AI_PATTERNS" --oneline --since="$MONTH_START" | wc -l)
    else
        MONTH_COMMITS=$(git log --grep="$AI_PATTERNS" --oneline --since="$MONTH_START" --until="$MONTH_END" | wc -l)
    fi
    
    printf "%-15s: %s\n" "$MONTH_NAME" "$MONTH_COMMITS"
done

echo ""

# Check for AI log file
AI_LOG_FILE=".git/ai-contributions.log"
if [ -f "$AI_LOG_FILE" ]; then
    echo -e "${PURPLE}=== AI Contributions Log Summary ===${NC}"
    echo -e "Log entries: $(grep -c '^[0-9]' "$AI_LOG_FILE" 2>/dev/null || echo 0)"
    echo -e "Most recent AI contribution:"
    tail -1 "$AI_LOG_FILE" 2>/dev/null | cut -d'|' -f1,3,5 | tr '|' ' - ' || echo "No entries"
    echo ""
fi

# Warnings and recommendations
echo -e "${RED}=== Recommendations ===${NC}"
if [ $AI_COMMITS -gt 0 ]; then
    echo "✅ AI contributions are being tracked"
    if [ $AI_PERCENTAGE -gt 50 ]; then
        echo "⚠️  High AI contribution percentage - ensure adequate human review"
    fi
else
    echo "ℹ️  No AI contributions detected in the specified time range"
fi

# Check for missing AI attribution
POTENTIAL_AI_BRANCHES=$(git branch -a | grep -E "(claude|ai|ai-assist)/" | wc -l)
if [ $POTENTIAL_AI_BRANCHES -gt 0 ]; then
    echo "ℹ️  Found $POTENTIAL_AI_BRANCHES AI-named branches - verify they have proper attribution"
fi

echo ""
echo -e "${BLUE}Use './tools/check-ai-branch.sh' to check current branch AI status${NC}"
echo -e "${BLUE}Use 'git log --grep=\"$AI_PATTERNS\" --oneline' for detailed AI commit list${NC}"