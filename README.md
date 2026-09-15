# MouseOptimizer - Advanced Mouse Response & Raw Acceleration

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows%2010%2F11-0078D4?logo=windows)](https://www.microsoft.com/windows)

A professional-grade Windows application with kernel-level driver that enhances mouse responsiveness and provides customizable raw acceleration (raw accel) for gaming and productivity.

## 🎯 Features

- **Kernel Filter Driver** - Direct hardware input processing
- **Raw Acceleration Algorithm** - Customizable curves (0.0-3.0x)
- **Polling Rate Optimization** - Up to 8000 Hz support
- **Multi-Profile Support** - Gaming, Productivity, Custom
- **Real-time Monitoring** - Live stats and diagnostics
- **Safe Design** - Memory protection, error handling, signed driver
- **Hotkey Support** - Quick profile switching

## 📋 System Requirements

- Windows 10 (Build 19041) or Windows 11
- .NET 6.0+
- Administrator privileges
- 15 MB free space

## 🚀 Quick Installation

```bash
# Clone repository
git clone https://github.com/testerdonutsmp-cmd/MouseOptimizer.git
cd MouseOptimizer

# Build
cd src/ui/WindowsApp
dotnet build -c Release

# Install driver (Admin)
.\scripts\install-driver.ps1

# Run application
.\bin\Release\MouseOptimizer.exe
```

## 📊 Raw Acceleration Formula

```
accelerated = base × (1 + factor × velocity^power)
```

**Presets:**
- Gaming: factor=0.6, power=2.0, threshold=50px
- Productivity: factor=0.2, power=1.5, threshold=100px

## 🏗️ Project Structure

```
src/
├── kernel/driver/      - C++ kernel filter driver
├── core/
│   ├── acceleration/   - Raw accel algorithms
│   └── profiles/       - Profile management
└── ui/WindowsApp/      - WPF C# application
docs/                   - Technical documentation
scripts/                - Installation scripts
```

## 📖 Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Driver Development](docs/DRIVER_DEV.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## ⚠️ Safety & Security

- Digitally signed kernel driver
- Memory protection & bounds checking
- Input validation
- Non-fatal error handling
- CPU impact: <2%, Memory: 15MB

## 📄 License

MIT License - See LICENSE file