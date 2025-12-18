# Contributing to CH32V003 Voltage Stabilizer

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
   - Relay modules for testing (5x)
   - Voltage sensing circuit
   - Multi-tap transformer (for full system test)

2. **Software**
   - [MounRiver Studio](http://www.mounriver.com/) (recommended IDE)
   - OR RISC-V GCC toolchain
   - Git for version control

3. **Knowledge**
   - Basic C programming
   - Embedded systems concepts
   - Understanding of GPIO, ADC, and timers
   - Familiarity with voltage regulation concepts

---

## Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/Stablizer.git
cd Stablizer

# Add upstream remote
git remote add upstream https://github.com/Joyboy7385/Stablizer.git
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
2. File -> Import -> Existing Projects
3. Navigate to the cloned repository
4. Select and import the project

**Using Command Line:**
```bash
# Ensure RISC-V toolchain is in PATH
export PATH=$PATH:/path/to/riscv-none-embed/bin

# Build the project
riscv-none-embed-gcc -march=rv32ec -mabi=ilp32e -O2 \
  -o stabilizer.elf main.c system_ch32v00x.c \
  ch32v00x_gpio.c ch32v00x_rcc.c ch32v00x_adc.c \
  ch32v00x_tim.c ch32v00x_flash.c ch32v00x_misc.c
```

---

## How to Contribute

### Types of Contributions

1. **Bug Fixes**
   - Fix issues with voltage regulation logic
   - Resolve ADC reading problems
   - Correct relay control timing
   - Fix 5V stability issues

2. **New Features**
   - Additional protection features
   - Serial communication for monitoring
   - Display support for voltage readout
   - Configurable threshold parameters

3. **Documentation**
   - Improve existing documentation
   - Add examples and tutorials
   - Create wiring diagrams
   - Translate to other languages

4. **Testing**
   - Test on different hardware configurations
   - Report compatibility issues
   - Verify fixes and features
   - Long-term reliability testing

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
void StateMachine1_Calculate_Voltages(void)
{
    // Variable declarations at the beginning of blocks
    uint16_t adc;
    float opv;

    // Comments for complex logic
    // Use 4 spaces for indentation (no tabs)
    adc = ADC_ReadCount_Filtered();
    opv = Calculate_OPV(adc);

    // Update global state
    currentOPV = opv;
    currentIPV = opv * relaySteps[currentStep].tap_ratio;
}
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Functions | CamelCase | `StateMachine2_Control_R5()` |
| Constants | UPPER_SNAKE_CASE | `HICUT_THRESHOLD` |
| Variables | camelCase | `currentStep` |
| Macros | UPPER_SNAKE_CASE | `PIN_R1` |
| Types | CamelCase_t | `RelayStep_t` |

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
5. **Use volatile** - For variables shared with ISRs

---

## Testing Guidelines

### Hardware Testing

Before submitting changes:

1. **Test at multiple voltages**
   - 3.3V MCU operation
   - 5.0V MCU operation (critical!)
   - Verify no issues at voltage boundaries

2. **Test all relay combinations**
   - Each step (0-7) functions correctly
   - Step transitions are smooth
   - No relay chatter

3. **Test protection features**
   - High-cut triggers at correct threshold
   - Low-cut triggers when enabled
   - Resume functions correctly

4. **Long-running tests**
   - Run for at least 1 hour
   - Check for drift or instability
   - Monitor temperature of components

### Test Checklist

```markdown
- [ ] Compiles without warnings
- [ ] Works at 3.3V
- [ ] Works at 5.0V (with Flash latency fix)
- [ ] All relay steps function correctly
- [ ] ADC readings are stable
- [ ] High-cut protection works
- [ ] Low-cut protection works (when enabled)
- [ ] Settings save and load correctly
- [ ] Calibration mode functions
- [ ] LED indicators show correct states
```

### Reporting Test Results

Include in your pull request:

```markdown
## Testing Performed

**Hardware Configuration:**
- MCU: CH32V003A4M6
- Operating Voltage: 5V
- Relay Type: SRD-05VDC-SL-C
- Transformer: [Description]
- Programmer: WCH-Link

**Test Results:**
- [x] Basic functionality
- [x] 5V stability
- [x] All 8 relay steps
- [x] Protection thresholds
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
- [ ] Performance improvement

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
- [ ] Tested at 5V operation
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
- Operating Voltage: 5V
- Firmware Version: [commit hash]
- IDE: MounRiver Studio v1.X

## Additional Information
Screenshots, oscilloscope captures, etc.
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

## Safety Considerations

When contributing to this project, keep in mind:

1. **Mains Voltage**: This project controls mains-voltage equipment
2. **Test Safely**: Always use isolated test setups
3. **Document Hazards**: Clearly document any safety-critical code
4. **Protection Logic**: Be extra careful with protection-related code
5. **Review Carefully**: All protection-related changes require thorough review

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
