# Contributing to CH32V003 Display Test

Thank you for your interest in contributing to this project! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Issue Reporting](#issue-reporting)

---

## Code of Conduct

### Our Pledge

We are committed to making participation in this project a harassment-free experience for everyone, regardless of level of experience, gender, gender identity, sexual orientation, disability, personal appearance, race, ethnicity, age, religion, or nationality.

### Our Standards

- Be respectful and inclusive
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Hardware**
   - CH32V003 development board or custom PCB
   - WCH-Link programmer
   - 7-segment display (common anode, 3-digit)

2. **Software**
   - [MounRiver Studio](http://www.mounriver.com/) (recommended IDE)
   - OR RISC-V GCC toolchain
   - Git for version control

3. **Knowledge**
   - Basic C programming
   - Embedded systems concepts
   - Understanding of GPIO and microcontrollers

---

## Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/Display-Test.git
cd Display-Test

# Add upstream remote
git remote add upstream https://github.com/Joyboy7385/Display-Test.git
```

### 2. Create a Branch

```bash
# Create a feature branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/bug-description
```

### 3. Set Up Development Environment

**Using MounRiver Studio:**
1. Open MounRiver Studio
2. File → Import → Existing Projects
3. Navigate to the cloned repository
4. Select and import the project

**Using Command Line:**
```bash
# Ensure RISC-V toolchain is in PATH
export PATH=$PATH:/path/to/riscv-none-embed/bin

# Build the project
make all
```

---

## How to Contribute

### Types of Contributions

1. **Bug Fixes**
   - Fix issues with display functionality
   - Resolve voltage stability problems
   - Correct timing issues

2. **New Features**
   - Additional display patterns
   - New demo modes
   - Support for different display types

3. **Documentation**
   - Improve existing documentation
   - Add examples and tutorials
   - Translate to other languages

4. **Testing**
   - Test on different hardware configurations
   - Report compatibility issues
   - Verify fixes and features

### Contribution Workflow

1. **Check Existing Issues**
   - Look for open issues you can help with
   - Avoid duplicate work by commenting on issues

2. **Discuss Major Changes**
   - Open an issue to discuss significant changes before implementing
   - Get feedback on your approach

3. **Make Changes**
   - Write clean, documented code
   - Follow coding standards
   - Test thoroughly

4. **Submit Pull Request**
   - Provide clear description
   - Reference related issues
   - Be responsive to feedback

---

## Coding Standards

### C Code Style

```c
// Function names: CamelCase with descriptive names
void GPIO_Init_Display(void)
{
    // Variable declarations at the beginning of blocks
    uint8_t i;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Comments for complex logic
    // Use 4 spaces for indentation (no tabs)
    for(i = 0; i < 10; i++) {
        // Braces on same line for control structures
        if(condition) {
            DoSomething();
        } else {
            DoSomethingElse();
        }
    }
}
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Functions | CamelCase | `Display_Refresh()` |
| Constants | UPPER_SNAKE_CASE | `SEG_A_PIN` |
| Variables | camelCase | `segPattern` |
| Macros | UPPER_SNAKE_CASE | `DIGIT_PAT` |

### Documentation Requirements

```c
/*=============================================================================
 * Function: FunctionName
 * Description: Brief description of what the function does
 * Parameters:
 *   param1 - Description of first parameter
 *   param2 - Description of second parameter
 * Returns: Description of return value (or "None" for void)
 * Notes: Any important notes about usage or limitations
 *=============================================================================*/
void FunctionName(uint8_t param1, uint16_t param2)
{
    // Implementation
}
```

### Code Quality Guidelines

1. **Keep functions small** - Single responsibility principle
2. **Avoid magic numbers** - Use named constants
3. **Handle edge cases** - Validate inputs where appropriate
4. **Comment why, not what** - Code should be self-documenting

---

## Testing Guidelines

### Hardware Testing

Before submitting changes:

1. **Test at multiple voltages**
   - 3.3V operation
   - 5.0V operation
   - Verify no issues at voltage boundaries

2. **Test all display modes**
   - Single digit display
   - Multi-digit display
   - All segments (888 test)

3. **Long-running tests**
   - Run for at least 1 hour
   - Check for flickering or instability

### Test Checklist

```markdown
- [ ] Compiles without warnings
- [ ] Works at 3.3V
- [ ] Works at 5.0V
- [ ] All digits display correctly
- [ ] No visible flickering
- [ ] No ghosting between digits
- [ ] Button inputs work (if applicable)
- [ ] Power consumption is reasonable
```

### Reporting Test Results

Include in your pull request:

```markdown
## Testing Performed

**Hardware Configuration:**
- MCU: CH32V003A4M6
- Display: [Model/Type]
- Voltage: [3.3V / 5V]
- Programmer: WCH-Link

**Test Results:**
- [x] Basic functionality
- [x] 5V stability
- [x] Long-running test (X hours)
- [ ] Edge case (describe any failures)

**Notes:**
Any additional observations...
```

---

## Pull Request Process

### Before Submitting

1. **Sync with upstream**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Run final checks**
   - Code compiles without warnings
   - All tests pass
   - Documentation is updated

3. **Clean commit history**
   ```bash
   # Squash commits if needed
   git rebase -i HEAD~N
   ```

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactoring

## Related Issues
Closes #XX

## Changes Made
- Change 1
- Change 2
- Change 3

## Testing
Describe testing performed

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings generated
```

### Review Process

1. **Automated checks** - Ensure build passes
2. **Code review** - Maintainer reviews changes
3. **Testing** - Changes tested on hardware
4. **Feedback** - Address any requested changes
5. **Merge** - Approved changes are merged

---

## Issue Reporting

### Bug Reports

Use this template for bug reports:

```markdown
## Bug Description
Clear description of the bug

## Steps to Reproduce
1. Step one
2. Step two
3. Step three

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- MCU: CH32V003A4M6
- Voltage: 5V
- Display Type: Common Anode 3-digit
- IDE: MounRiver Studio v1.X

## Additional Information
Screenshots, logic analyzer captures, etc.
```

### Feature Requests

Use this template for feature requests:

```markdown
## Feature Description
Clear description of the proposed feature

## Use Case
Why this feature would be useful

## Proposed Implementation
How you think it could be implemented (optional)

## Alternatives Considered
Other approaches you've thought about
```

---

## Recognition

Contributors will be recognized in:

- README.md contributors section
- Release notes for significant contributions
- GitHub contributors list

---

## Questions?

- Open an issue for questions about contributing
- Check existing issues and documentation first
- Be patient - maintainers are volunteers

---

Thank you for contributing to make this project better!
