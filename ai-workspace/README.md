# AI Workspace

This directory is dedicated to AI-assisted development and experimentation for the sdplane-dev project.

## Purpose

- **Experimentation**: Test AI-generated code before integration
- **Isolation**: Keep AI experiments separate from main development
- **Documentation**: Track AI contributions and learnings
- **Staging**: Prepare AI-assisted changes for review and integration

## Directory Structure

```
ai-workspace/
├── README.md                 # This file
├── experiments/              # AI code experiments
├── integration/              # Staging area for integration
├── templates/                # Code templates and examples
├── logs/                     # AI session logs and notes
└── docs/                     # AI-generated documentation drafts
```

## Usage Guidelines

### Starting an AI Session

1. **Create a branch**: Use AI naming convention (`claude/`, `ai/`, `ai-assist/`)
   ```bash
   git checkout -b claude/experiment-name
   ```

2. **Set AI git identity** (optional):
   ```bash
   git config user.name "Your Name + Claude"
   git config user.email "yourname+claude@example.com"
   ```

3. **Work in this directory**: Use subdirectories for organization

### During Development

- **Document decisions**: Keep notes in `logs/` about AI suggestions and human choices
- **Test thoroughly**: AI-generated code requires extra verification
- **Review carefully**: Don't blindly accept AI suggestions

### Integration Process

1. **Move to integration/**: Copy working code to staging area
2. **Human review**: Thorough code review by human developer
3. **Testing**: Run full test suite and manual testing
4. **Documentation**: Update relevant docs and comments
5. **Merge**: Create PR using the AI-aware template

## Best Practices

### Code Quality
- Always run style checker: `./style/check_gnu_style.sh check`
- Verify DPDK-specific functionality
- Test with actual hardware when possible
- Check for memory leaks and performance impact

### Documentation
- Mark AI-generated functions with appropriate comments
- Document the AI's reasoning for complex decisions
- Include human verification notes

### Testing
- AI code often needs additional edge case testing
- Verify integration with existing DPDK components
- Test with different network configurations

## Templates and Examples

See `templates/` directory for:
- AI-generated function comment templates
- Common DPDK patterns
- Error handling examples
- Testing patterns

## Integration Checklist

Before moving AI code to main development:

- [ ] Code follows project style guidelines
- [ ] All tests pass
- [ ] Human code review completed  
- [ ] Documentation updated
- [ ] AI attribution added to commits
- [ ] Performance impact assessed
- [ ] Security review (if applicable)

## Notes

- This workspace is excluded from main builds
- Experimental code here may be incomplete or broken
- Always backup working code before experimenting
- Consider performance implications of AI suggestions