#!/bin/bash
#
# Add AI Comments Script
# Helps add proper AI attribution comments to source files
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
CURRENT_DATE=$(date +%Y-%m-%d)
REVIEWER="pending"

usage() {
    echo "Usage: $0 [OPTIONS] <file> <function_name>"
    echo ""
    echo "Adds AI attribution comment above a function in a source file"
    echo ""
    echo "Options:"
    echo "  -t, --tool TOOL       AI tool name (default: Claude Code)"
    echo "  -r, --reviewer NAME   Reviewer name (default: pending)"
    echo "  -p, --purpose TEXT    Function purpose description"
    echo "  -c, --complex         Use complex template with detailed fields"
    echo "  -d, --dpdk           Use DPDK-specific template"
    echo "  -P, --packet         Use packet processing template"
    echo "  -n, --dry-run        Show what would be added without modifying file"
    echo "  -l, --list           List available templates"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 src/packet.c process_packet_burst"
    echo "  $0 -p 'Validates packet headers' src/validate.c validate_headers"
    echo "  $0 -d -r 'john.doe' src/dpdk.c init_dpdk_ports"
    echo "  $0 -c src/complex.c complex_algorithm"
    exit 1
}

list_templates() {
    echo -e "${BLUE}Available AI Comment Templates:${NC}"
    echo ""
    echo -e "${GREEN}1. Basic (-t basic)${NC}         - Simple AI function"
    echo -e "${GREEN}2. Complex (-c)${NC}             - Complex function with detailed docs"
    echo -e "${GREEN}3. DPDK (-d)${NC}                - DPDK-specific function"
    echo -e "${GREEN}4. Packet (-P)${NC}              - Packet processing function"
    echo -e "${GREEN}5. Algorithm (-a)${NC}           - Algorithm implementation"
    echo -e "${GREEN}6. Data Structure (-s)${NC}      - Data structure definition"
    echo -e "${GREEN}7. Configuration (-C)${NC}       - Configuration/setup function"
    echo -e "${GREEN}8. Debug (-D)${NC}               - Debug/logging function"
    echo -e "${GREEN}9. Error (-e)${NC}               - Error handling code"
    echo -e "${GREEN}10. Integration (-i)${NC}        - Integration/glue code"
    echo ""
    echo "Use the corresponding flag to select a template type."
    exit 0
}

# Parse command line arguments
FILE=""
FUNCTION=""
PURPOSE=""
TEMPLATE_TYPE="basic"
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--tool)
            AI_TOOL="$2"
            shift 2
            ;;
        -r|--reviewer)
            REVIEWER="$2"
            shift 2
            ;;
        -p|--purpose)
            PURPOSE="$2"
            shift 2
            ;;
        -c|--complex)
            TEMPLATE_TYPE="complex"
            shift
            ;;
        -d|--dpdk)
            TEMPLATE_TYPE="dpdk"
            shift
            ;;
        -P|--packet)
            TEMPLATE_TYPE="packet"
            shift
            ;;
        -a|--algorithm)
            TEMPLATE_TYPE="algorithm"
            shift
            ;;
        -s|--structure)
            TEMPLATE_TYPE="structure"
            shift
            ;;
        -C|--config)
            TEMPLATE_TYPE="config"
            shift
            ;;
        -D|--debug)
            TEMPLATE_TYPE="debug"
            shift
            ;;
        -e|--error)
            TEMPLATE_TYPE="error"
            shift
            ;;
        -i|--integration)
            TEMPLATE_TYPE="integration"
            shift
            ;;
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -l|--list)
            list_templates
            ;;
        -h|--help)
            usage
            ;;
        *)
            if [ -z "$FILE" ]; then
                FILE="$1"
            elif [ -z "$FUNCTION" ]; then
                FUNCTION="$1"
            else
                echo "Error: Too many arguments"
                usage
            fi
            shift
            ;;
    esac
done

# Validate arguments
if [ -z "$FILE" ] || [ -z "$FUNCTION" ]; then
    echo "Error: File and function name are required"
    usage
fi

if [ ! -f "$FILE" ]; then
    echo "Error: File '$FILE' does not exist"
    exit 1
fi

# Set default purpose based on function name if not provided
if [ -z "$PURPOSE" ]; then
    PURPOSE="[Brief description of $FUNCTION purpose]"
fi

# Generate comment based on template type
generate_comment() {
    case $TEMPLATE_TYPE in
        basic)
            cat << EOF
/* 
 * AI-generated function - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * Human review: $REVIEWER
 */
EOF
            ;;
        complex)
            cat << EOF
/* 
 * AI-generated function - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * AI reasoning: [Why the AI chose this approach]
 * Human modifications: [What human changed from AI suggestion]
 * Performance notes: [Any performance considerations]
 * Security review: [Security implications, if any]
 * 
 * Human review: $REVIEWER
 * Testing status: [unit tested/integration tested/manual tested]
 */
EOF
            ;;
        dpdk)
            cat << EOF
/* 
 * AI-generated DPDK function - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * DPDK version: [Compatible DPDK version]
 * Thread safety: [thread-safe/not thread-safe/lcore-specific]
 * Performance impact: [high/medium/low/negligible]
 * 
 * Human review: $REVIEWER
 * DPDK testing: [tested on hardware/tested in VM/simulation only]
 */
EOF
            ;;
        packet)
            cat << EOF
/* 
 * AI-generated packet processor - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * Packet types: [which packet types are handled]
 * Burst size: [recommended burst size for optimal performance]
 * Memory requirements: [memory pools, hugepages, etc.]
 * Error handling: [how errors are handled and reported]
 * 
 * Human review: $REVIEWER
 * Traffic testing: [tested with real traffic/synthetic traffic/untested]
 */
EOF
            ;;
        algorithm)
            cat << EOF
/* 
 * AI-generated algorithm - $AI_TOOL $CURRENT_DATE
 * Algorithm: [name/type of algorithm]
 * Purpose: $PURPOSE
 * 
 * Complexity: [time/space complexity]
 * Scalability: [how it scales with input size]
 * Edge cases: [known edge cases and handling]
 * Alternative approaches: [other approaches considered by AI]
 * 
 * Human review: $REVIEWER
 * Validation: [algorithm correctness verified/pending verification]
 */
EOF
            ;;
        structure)
            cat << EOF
/* 
 * AI-generated data structure - $AI_TOOL $CURRENT_DATE
 * Structure: $PURPOSE
 * 
 * Memory layout: [memory organization and alignment]
 * Thread safety: [concurrent access safety]
 * Performance characteristics: [access patterns and performance]
 * Initialization: [how to properly initialize]
 * Cleanup: [how to properly cleanup/free]
 * 
 * Human review: $REVIEWER
 * Memory testing: [tested for leaks/corruption/pending]
 */
EOF
            ;;
        config)
            cat << EOF
/* 
 * AI-generated configuration function - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * Configuration parameters: [list of key parameters]
 * Default values: [default behavior]
 * Validation: [parameter validation performed]
 * Error recovery: [how configuration errors are handled]
 * Dependencies: [what must be configured first]
 * 
 * Human review: $REVIEWER
 * Integration testing: [tested with full system/isolated testing]
 */
EOF
            ;;
        debug)
            cat << EOF
/* 
 * AI-generated debug function - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * Log level: [debug/info/warning/error]
 * Performance impact: [impact on performance when enabled]
 * Information captured: [what information is logged]
 * Format: [log format and structure]
 * 
 * Human review: $REVIEWER
 * Log testing: [verified log output/pending verification]
 */
EOF
            ;;
        error)
            cat << EOF
/* 
 * AI-generated error handling - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * Error types: [list of error conditions handled]
 * Recovery actions: [what the code does to recover]
 * Logging: [how errors are logged]
 * Return codes: [error codes returned]
 * Resource cleanup: [how resources are cleaned up on error]
 * 
 * Human review: $REVIEWER
 * Error testing: [error conditions tested/pending testing]
 */
EOF
            ;;
        integration)
            cat << EOF
/* 
 * AI-generated integration code - $AI_TOOL $CURRENT_DATE
 * Purpose: $PURPOSE
 * 
 * Interfaces: [APIs and interfaces used]
 * Data flow: [how data flows between components]
 * Synchronization: [synchronization mechanisms used]
 * Error propagation: [how errors are propagated]
 * 
 * Human review: $REVIEWER
 * Integration testing: [tested end-to-end/pending testing]
 */
EOF
            ;;
        *)
            echo "Error: Unknown template type '$TEMPLATE_TYPE'"
            exit 1
            ;;
    esac
}

# Find the function in the file
FUNCTION_LINE=$(grep -n "^[a-zA-Z_][a-zA-Z0-9_]*.*$FUNCTION.*(" "$FILE" | head -1 | cut -d: -f1)

if [ -z "$FUNCTION_LINE" ]; then
    echo "Error: Function '$FUNCTION' not found in '$FILE'"
    echo "Make sure the function definition starts at the beginning of a line"
    exit 1
fi

echo -e "${BLUE}=== Adding AI Comment ===${NC}"
echo -e "File: ${CYAN}$FILE${NC}"
echo -e "Function: ${GREEN}$FUNCTION${NC} (line $FUNCTION_LINE)"
echo -e "Template: ${YELLOW}$TEMPLATE_TYPE${NC}"
echo -e "AI Tool: ${GREEN}$AI_TOOL${NC}"
echo -e "Reviewer: ${GREEN}$REVIEWER${NC}"
echo ""

# Generate the comment
COMMENT=$(generate_comment)

echo -e "${YELLOW}Comment to be added:${NC}"
echo "----------------------------------------"
echo "$COMMENT"
echo "----------------------------------------"
echo ""

if [ "$DRY_RUN" = true ]; then
    echo -e "${BLUE}DRY RUN: Would add comment above line $FUNCTION_LINE${NC}"
else
    # Create backup
    cp "$FILE" "$FILE.bak"
    
    # Insert comment above function
    {
        head -n $((FUNCTION_LINE - 1)) "$FILE"
        echo "$COMMENT"
        tail -n +$FUNCTION_LINE "$FILE"
    } > "$FILE.tmp"
    
    mv "$FILE.tmp" "$FILE"
    
    echo -e "${GREEN}✅ AI comment added to $FILE${NC}"
    echo -e "${BLUE}Backup saved as: $FILE.bak${NC}"
    
    # Suggest next steps
    echo ""
    echo -e "${PURPLE}=== Next Steps ===${NC}"
    echo -e "${YELLOW}1.${NC} Review and customize the comment fields"
    echo -e "${YELLOW}2.${NC} Update the purpose and technical details"
    echo -e "${YELLOW}3.${NC} Have code reviewed and update reviewer field"
    echo -e "${YELLOW}4.${NC} Run style checker: ./style/check_gnu_style.sh check $FILE"
fi