#!/bin/bash
# MouseOptimizer Build Script (Linux/Mac - for reference)
# Note: Actual application requires Windows

set -e

echo "========================================"
echo "MouseOptimizer Build Script"
echo "========================================"
echo ""

# Check if dotnet is installed
if ! command -v dotnet &> /dev/null; then
    echo "ERROR: .NET SDK not found"
    echo "Download from: https://dotnet.microsoft.com/download"
    exit 1
fi

echo "✓ .NET SDK found: $(dotnet --version)"
echo ""

# Create output directory
mkdir -p build-output

# Build steps
echo "[1] Restoring NuGet packages..."
cd src/ui
dotnet restore

echo "[2] Building project (Release)..."
dotnet build -c Release --no-restore

echo "[3] Publishing executable..."
dotnet publish -c Release --no-build -o ../../build-output/publish

cd ../../

echo "[4] Copying files..."
cp src/ui/bin/Release/net6.0-windows/MouseOptimizer.exe build-output/ 2>/dev/null || true
cp README.md build-output/
cp LICENSE build-output/

echo ""
echo "========================================"
echo "✓ Build completed successfully!"
echo "========================================"
echo ""
echo "Output: build-output/MouseOptimizer.exe"
echo ""
echo "Note: This application requires Windows to run."
echo "Run on Windows machine: .\build-output\MouseOptimizer.exe"
