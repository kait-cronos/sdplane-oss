#!/bin/bash
#
# AI Commit Helper Script
# Assists with creating properly attributed AI commits
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

# Default values
AI_TOOL="Claude Code"
AI_EMAIL="noreply@anthropic.com"
CURRENT_DATE=$(date +%Y-%m-%d)

usage() {
    echo "Usage: $0 [OPTIONS] <commit-message>"
    echo ""
    echo "Creates an AI-attributed commit with proper formatting"
    echo ""
    echo "Options:"
    echo "  -t, --tool TOOL     AI tool name (default: Claude Code)"
    echo "  -e, --email EMAIL   AI email (default: noreply@anthropic.com)"
    echo "  -a, --add-all       Add all changes before committing"
    echo "  -n, --dry-run       Show what would be committed without doing it"
    echo "  -h, --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 'feat: add packet validation'"
    echo "  $0 -a 'fix: resolve memory leak in handler'"
    echo "  $0 -t 'ChatGPT' -e 'ai@openai.com' 'docs: update README'"
    echo "  $0 -n 'test: add unit tests for parser'"
    exit 1
}

# Parse command line arguments
COMMIT_MSG=""
ADD_ALL=false
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--tool)
            AI_TOOL="$2"
            shift 2
            ;;
        -e|--email)
            AI_EMAIL="$2"
            shift 2
            ;;
        -a|--add-all)
            ADD_ALL=true
            shift
            ;;
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        -*)
            echo "Unknown option $1"
            usage
            ;;
        *)
            if [ -z "$COMMIT_MSG" ]; then
                COMMIT_MSG="$1"
            else
                echo "Error: Multiple commit messages provided"
                usage
            fi
            shift
            ;;
    esac
done

# Check if commit message was provided
if [ -z "$COMMIT_MSG" ]; then
    echo "Error: Commit message is required"
    usage
fi

# Get current branch and check if it's an AI branch
CURRENT_BRANCH=$(git symbolic-ref --short HEAD 2>/dev/null || echo "detached HEAD")
IS_AI_BRANCH=false

if echo "$CURRENT_BRANCH" | grep -qE "^(claude|ai|ai-assist)/"; then
    IS_AI_BRANCH=true
fi

# Prepare the full commit message with AI attribution
FULL_COMMIT_MSG="$COMMIT_MSG

🤖 Generated with [$AI_TOOL](https://claude.ai/code)

Co-Authored-By: $(echo $AI_TOOL | sed 's/ /_/g') <$AI_EMAIL>"

echo -e "${BLUE}=== AI Commit Helper ===${NC}"
echo -e "Branch: ${CYAN}$CURRENT_BRANCH${NC}"
echo -e "AI Tool: ${GREEN}$AI_TOOL${NC}"
echo -e "AI Email: ${GREEN}$AI_EMAIL${NC}"

if [ "$IS_AI_BRANCH" = true ]; then
    echo -e "AI Branch: ${GREEN}✅ Detected${NC}"
else
    echo -e "AI Branch: ${YELLOW}⚠️  Not an AI branch${NC}"
    echo -e "Consider renaming to: ${CYAN}claude/$(echo $CURRENT_BRANCH | sed 's/^[a-z]*\///')${NC}"
fi

echo ""

# Check for changes to commit
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo -e "${CYAN}Changes to be committed:${NC}"
    
    if [ "$ADD_ALL" = true ]; then
        echo "Adding all changes..."
        if [ "$DRY_RUN" = false ]; then
            git add -A
        fi
    fi
    
    # Show what will be committed
    git diff --cached --name-status | while read status file; do
        case $status in
            A) echo -e "  ${GREEN}added:${NC}     $file" ;;
            M) echo -e "  ${YELLOW}modified:${NC}  $file" ;;
            D) echo -e "  ${RED}deleted:${NC}   $file" ;;
            R*) echo -e "  ${PURPLE}renamed:${NC}   $file" ;;
            C*) echo -e "  ${PURPLE}copied:${NC}    $file" ;;
            *) echo -e "  ${CYAN}$status:${NC}       $file" ;;
        esac
    done
    
    if ! git diff --cached --quiet; then
        echo ""
        echo -e "${YELLOW}Commit message preview:${NC}"
        echo "----------------------------------------"
        echo "$FULL_COMMIT_MSG"
        echo "----------------------------------------"
        echo ""
        
        if [ "$DRY_RUN" = true ]; then
            echo -e "${BLUE}DRY RUN: Would create commit with above message${NC}"
        else
            # Confirm before committing
            echo -e "${YELLOW}Proceed with AI-attributed commit? (y/N):${NC}"
            read -r CONFIRMATION
            
            case $CONFIRMATION in
                [Yy]|[Yy][Ee][Ss])
                    echo "Creating AI-attributed commit..."
                    git commit -m "$FULL_COMMIT_MSG"
                    
                    COMMIT_HASH=$(git rev-parse HEAD)
                    echo -e "${GREEN}✅ Commit created: ${COMMIT_HASH:0:8}${NC}"
                    
                    # Log to AI contributions file
                    AI_LOG_FILE=".git/ai-contributions.log"
                    if [ ! -f "$AI_LOG_FILE" ]; then
                        echo "# AI Contributions Log" > "$AI_LOG_FILE"
                        echo "# Format: TIMESTAMP|COMMIT_HASH|BRANCH|AUTHOR|COMMIT_SUBJECT" >> "$AI_LOG_FILE"
                        echo "" >> "$AI_LOG_FILE"
                    fi
                    
                    AUTHOR=$(git config user.name)
                    echo "$(date -u +%Y-%m-%dT%H:%M:%SZ)|$COMMIT_HASH|$CURRENT_BRANCH|$AUTHOR|$COMMIT_MSG" >> "$AI_LOG_FILE"
                    
                    echo -e "${BLUE}AI contribution logged to $AI_LOG_FILE${NC}"
                    ;;
                *)
                    echo "Commit cancelled"
                    exit 1
                    ;;
            esac
        fi
    else
        echo -e "${YELLOW}⚠️  No staged changes found${NC}"
        if [ "$ADD_ALL" = false ]; then
            echo "Use -a/--add-all to stage all changes, or manually stage changes first"
        fi
        exit 1
    fi
else
    echo -e "${YELLOW}⚠️  No changes to commit${NC}"
    exit 1
fi

# Post-commit recommendations
if [ "$DRY_RUN" = false ] && [ "$IS_AI_BRANCH" = false ]; then
    echo ""
    echo -e "${PURPLE}=== Recommendations ===${NC}"
    echo -e "${YELLOW}💡 Consider renaming branch for AI work:${NC}"
    echo "   git checkout -b claude/$(echo $CURRENT_BRANCH | sed 's/^[a-z]*\///')"
fi