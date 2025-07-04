# Pull Request Template

## Change Summary
<!-- Provide a brief description of the changes -->

## Type of Change
- [ ] 🐛 Bug fix (non-breaking change which fixes an issue)
- [ ] ✨ New feature (non-breaking change which adds functionality)
- [ ] 💥 Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] 🔧 Refactoring (no functional changes, no api changes)
- [ ] 📝 Documentation update
- [ ] 🎨 Code style/formatting changes
- [ ] ⚡ Performance improvement
- [ ] 🧪 Test changes

## AI Involvement
<!-- Please check all that apply -->
- [ ] This change was made entirely by humans
- [ ] This change was AI-assisted (human-guided with AI help)
- [ ] This change was primarily AI-generated (with human review)

### If AI-assisted, please provide details:
- **AI tool used**: <!-- e.g., Claude Code, ChatGPT, GitHub Copilot -->
- **AI contribution level**: <!-- e.g., code generation, debugging, documentation, testing -->
- **Human review level**: <!-- e.g., thorough review, spot-check, functional testing -->
- **Areas requiring manual adjustment**: <!-- List any parts that needed human modification -->

## Testing
- [ ] I have tested this change locally
- [ ] I have added/updated tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes
- [ ] I have run the code style checker (`./style/check_gnu_style.sh check`)

## DPDK-Specific Testing (if applicable)
- [ ] Tested with hugepages configured
- [ ] Verified DPDK port initialization
- [ ] Tested packet forwarding functionality
- [ ] Verified lcore assignment and threading
- [ ] Tested with multiple NICs

## Documentation
- [ ] I have updated documentation (README, CLAUDE.md, code comments)
- [ ] I have added/updated configuration examples
- [ ] I have documented any breaking changes

## Security Considerations
- [ ] This change does not introduce security vulnerabilities
- [ ] I have reviewed the code for potential security issues
- [ ] No sensitive information (keys, passwords) is exposed

## Checklist
- [ ] My code follows the project's coding standards
- [ ] Self-review of the code completed
- [ ] Code builds without warnings
- [ ] All tests pass
- [ ] Branch is up to date with main

## Additional Context
<!-- Add any other context, screenshots, or links relevant to this PR -->

## Related Issues
<!-- Link to related issues using #issue_number -->
Closes #
Related to #

---
<!-- 
AI Attribution (if applicable):
🤖 Generated with [AI Tool Name](link)
Co-Authored-By: AI Assistant <ai@example.com>
-->