# MouseOptimizer Troubleshooting Guide

## Common Issues & Solutions

### Driver Installation Issues

#### Problem: "Access Denied" when running install script
**Solution:**
1. Right-click PowerShell → Run as Administrator
2. Run: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned`
3. Run install script again

#### Problem: Driver fails to load on startup
**Solution:**
1. Open Event Viewer
2. Navigate to Windows Logs → System
3. Look for errors from "MouseOptimizerDriver"
4. Check if Secure Boot is enabled:
   ```powershell
   Confirm-SecureBootUEFI
   ```
5. If needed, temporarily disable for test signing:
   ```powershell
   BCDEdit /set testsigning on
   ```

### Application Issues

#### Problem: Application crashes on startup
**Solution:**
1. Update .NET 6 runtime:
   ```powershell
   dotnet --version
   ```
2. Reinstall from: https://dotnet.microsoft.com/download
3. Delete config files:
   ```
   %APPDATA%\MouseOptimizer\
   ```
4. Restart application

#### Problem: Mouse acceleration not working
**Solution:**
1. Verify driver is installed:
   ```powershell
   Get-Service MouseOptimizerDriver
   ```
2. Check if running as Administrator
3. Verify polling rate is enabled (≥500 Hz)
4. Try default profile first

### Performance Issues

#### Problem: High CPU usage
**Solution:**
1. Check slider values - lower acceleration factor
2. Increase velocity threshold
3. Disable debug logging:
   - Settings → Logging → Disable Debug Logging

#### Problem: Mouse stuttering or lag
**Solution:**
1. Update mouse driver
2. Check for conflicting input devices
3. Disable raw input temporarily:
   - Settings → General → Uncheck "Enable Raw Input"

### Error Messages

#### "DRIVER_IRQL_NOT_LESS_OR_EQUAL"
- **Cause:** Kernel memory access violation
- **Fix:** Restart computer in Safe Mode and uninstall driver

#### "CODE_INTEGRITY_CHECK_FAILED"
- **Cause:** Driver signature invalid
- **Fix:** Enable test signing or use signed driver build

#### "INSUFFICIENT_RESOURCES"
- **Cause:** Out of kernel memory
- **Fix:** Restart Windows, close background applications

### Uninstallation

#### Complete Removal
```powershell
# Run as Administrator
.\scripts\uninstall-driver.ps1

# Remove application data
Remove-Item "$env:APPDATA\MouseOptimizer" -Recurse

# Reset test signing if enabled
BCDEdit /set testsigning off
```

## Advanced Debugging

### Enable Kernel Logging
1. Open Registry Editor
2. Navigate to: `HKLM\SYSTEM\CurrentControlSet\Services\MouseOptimizerDriver`
3. Set "Start" value to 2 (for verbose logging)

### Check Driver Status
```powershell
# List driver info
Get-Service MouseOptimizerDriver

# View driver logs
Get-EventLog -LogName System -Source MouseOptimizerDriver -Newest 10
```

### Memory Leak Detection
```bash
# Using PoolMon (from WDK)
poolmon.exe

# Look for 'mOpt' tag usage
```

## Support

If issue persists:
1. Check [GitHub Issues](https://github.com/testerdonutsmp-cmd/MouseOptimizer/issues)
2. Collect logs:
   - `%APPDATA%\MouseOptimizer\logs\`
   - Event Viewer system logs
3. Report with system specs:
   - Windows version
   - Mouse model
   - Other software that intercepts input
