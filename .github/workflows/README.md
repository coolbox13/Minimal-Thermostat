# CI/CD Pipeline Documentation

This directory contains GitHub Actions workflows for automated building, testing, and deployment.

## Workflows

### ci.yml - Continuous Integration/Deployment

Main CI/CD pipeline that runs on every push and pull request.

#### Jobs

**1. Build (`build`)**
- Builds ESP32 firmware using PlatformIO
- Caches PlatformIO dependencies for faster builds
- Uploads firmware binary as artifact
- Runs on: pushes to main, develop, claude/* branches

**2. Test (`test`)**
- Runs unit tests using native environment
- Executes all tests in test/ directory
- Uploads test results
- Independent from build job (runs in parallel)

**3. Lint (`lint`)**
- Runs cppcheck for static code analysis
- Checks for warnings, performance issues, portability
- Runs in parallel with build and test
- Non-blocking (warnings don't fail the build)

**4. Documentation (`documentation`)**
- Generates Doxygen documentation
- Only runs on main branch
- Uploads HTML documentation as artifact
- Requires Doxyfile in repository root

**5. Release (`release`)**
- Triggers only on version tags (v*.*.*)
- Downloads firmware artifact from build job
- Creates GitHub release with firmware binary
- Includes installation instructions

## Triggering Workflows

### Automatic Triggers

- **Push to main**: Full pipeline + documentation
- **Push to develop**: Full pipeline
- **Push to claude/***: Full pipeline (for Claude AI branches)
- **Pull request to main/develop**: Build + test + lint
- **Tag push (v*)**: Full pipeline + release creation

### Manual Trigger

Workflows can be triggered manually from the GitHub Actions tab.

## Status Badges

Add these to README.md to show build status:

```markdown
![Build Status](https://github.com/YOUR_USERNAME/Minimal-Thermostat/workflows/CI%2FCD%20Pipeline/badge.svg)
```

## Artifacts

### Build Artifacts
- **firmware.bin**: Compiled ESP32 firmware
- **Retention**: 30 days
- **Location**: Actions run → Artifacts section

### Test Artifacts
- **test-results**: Unit test output logs
- **Retention**: 30 days
- **Available even if tests fail**

### Documentation Artifacts
- **documentation**: Doxygen HTML output
- **Retention**: 90 days
- **Only on main branch**

## Caching

The pipeline uses GitHub Actions cache to speed up builds:

- **PlatformIO packages**: ~/.platformio
- **Build artifacts**: .pio/
- **Cache key**: Based on platformio.ini hash
- **Fallback**: Uses latest cache if exact match not found

This reduces typical build time from 5-8 minutes to 1-2 minutes.

## Environment Variables

No secrets required for basic operation. For advanced features:

### Optional Secrets

- `PLATFORMIO_AUTH_TOKEN`: For private library access
- `SLACK_WEBHOOK`: For build notifications
- `CODECOV_TOKEN`: For code coverage reports

Add these in: Repository Settings → Secrets → Actions

## Customization

### Changing Build Targets

Edit `platformio.ini` and update workflow:

```yaml
- name: Build firmware for ESP32
  run: pio run --environment YOUR_ENVIRONMENT
```

### Adding More Tests

1. Add test file to `test/` directory
2. Tests are automatically discovered
3. No workflow changes needed

### Enabling Release Automation

1. Create version tag: `git tag v1.0.0`
2. Push tag: `git push origin v1.0.0`
3. Workflow creates release automatically

## Troubleshooting

### Build Fails

1. Check platformio.ini syntax
2. Verify all dependencies are listed
3. Check build logs for specific errors

### Tests Fail

1. Review test output in artifacts
2. Run tests locally: `pio test --environment native`
3. Check for platform-specific issues

### Cache Issues

1. Clear cache from Actions tab
2. Wait for new cache to rebuild
3. Cache is automatically updated on successful runs

## Performance Metrics

Typical workflow times (with cache):

- Build: 1-2 minutes
- Test: 30-60 seconds
- Lint: 30 seconds
- Documentation: 1 minute
- **Total**: ~3-4 minutes

Without cache: 8-10 minutes total

## Future Enhancements

Possible additions to the pipeline:

1. **Code Coverage**: Add coverage reporting with Codecov
2. **Security Scanning**: Add dependency vulnerability checks
3. **Firmware Size Check**: Warn if firmware exceeds limits
4. **OTA Update Publishing**: Automatically publish OTA updates
5. **Notification System**: Slack/Discord build notifications
6. **Performance Testing**: Benchmark PID controller performance

## CI/CD Best Practices

✅ **DO:**
- Keep builds fast with caching
- Run tests in parallel with builds
- Use artifacts for debugging
- Tag releases with semantic versioning

❌ **DON'T:**
- Commit large binary files
- Hard-code secrets in workflow
- Skip tests to "save time"
- Ignore linter warnings

## Support

For CI/CD issues:
1. Check GitHub Actions documentation
2. Review workflow logs
3. Test locally with PlatformIO
4. Create issue with logs attached
