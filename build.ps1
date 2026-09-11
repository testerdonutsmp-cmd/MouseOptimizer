# Build script untuk MouseOptimizer
# Membuat .exe langsung jadi

[CmdletBinding()]
param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Standalone,
    [switch]$CleanFirst
)

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "MouseOptimizer Build Script" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan

$projectPath = "src/ui"
$outputDir = ".\build-output"

# Check if dotnet is installed
try {
    $version = dotnet --version
    Write-Host "✓ .NET SDK found: $version" -ForegroundColor Green
} catch {
    Write-Error ".NET SDK not found. Please install .NET 6.0 or later"
    exit 1
}

# Clean if requested
if ($CleanFirst) {
    Write-Host "`nCleaning previous builds..." -ForegroundColor Yellow
    Remove-Item "$projectPath/bin" -Recurse -ErrorAction SilentlyContinue
    Remove-Item "$projectPath/obj" -Recurse -ErrorAction SilentlyContinue
    Remove-Item $outputDir -Recurse -ErrorAction SilentlyContinue
}

# Create output directory
New-Item -ItemType Directory -Path $outputDir -Force | Out-Null

Write-Host "`nStep 1: Restore NuGet packages..." -ForegroundColor Yellow
cd $projectPath
dotnet restore
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "`nStep 2: Build project ($Configuration)..." -ForegroundColor Yellow
dotnet build -c $Configuration --no-restore
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "`nStep 3: Create standalone executable..." -ForegroundColor Yellow
if ($Standalone) {
    dotnet publish -c $Configuration --no-build --self-contained -r win-x64 -o "../../$outputDir/standalone"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Standalone executable created" -ForegroundColor Green
        Write-Host "  Location: $outputDir/standalone/MouseOptimizer.exe" -ForegroundColor Cyan
    }
}

Write-Host "`nStep 4: Copy executable to output..." -ForegroundColor Yellow
$exePath = "bin/$Configuration/net6.0-windows/MouseOptimizer.exe"
if (Test-Path $exePath) {
    Copy-Item $exePath "../../$outputDir/MouseOptimizer.exe" -Force
    Write-Host "✓ Executable copied" -ForegroundColor Green
} else {
    Write-Error "Build failed: MouseOptimizer.exe not found"
    exit 1
}

cd ../..

Write-Host "`nStep 5: Copy supporting files..." -ForegroundColor Yellow
Copy-Item "scripts/install-driver.ps1" "$outputDir/" -Force
Copy-Item "README.md" "$outputDir/" -Force
Copy-Item "LICENSE" "$outputDir/" -Force
Copy-Item "CONTRIBUTING.md" "$outputDir/" -Force

Write-Host "`n======================================" -ForegroundColor Green
Write-Host "✓ Build completed successfully!" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Green

Write-Host "`nOutput location:" -ForegroundColor Cyan
Write-Host "  📁 $((Get-Item $outputDir).FullName)" -ForegroundColor Cyan

Write-Host "`nExecutables:" -ForegroundColor Cyan
Get-ChildItem "$outputDir/*.exe" -Recurse | ForEach-Object {
    $size = [math]::Round($_.Length / 1MB, 2)
    Write-Host "  ✓ $($_.Name) ($size MB)" -ForegroundColor Green
}

Write-Host "`nNext steps:" -ForegroundColor Yellow
Write-Host "  1. Run: .\build-output\MouseOptimizer.exe" -ForegroundColor White
Write-Host "  2. Install driver: .\build-output\install-driver.ps1 (as Admin)" -ForegroundColor White
Write-Host "  3. Restart computer" -ForegroundColor White

Write-Host "`nUsage:" -ForegroundColor Cyan
Write-Host "  .\build.ps1                    - Standard build" -ForegroundColor White
Write-Host "  .\build.ps1 -Standalone        - Include standalone .exe" -ForegroundColor White
Write-Host "  .\build.ps1 -CleanFirst        - Clean before build" -ForegroundColor White
