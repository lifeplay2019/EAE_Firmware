param(
    [double]$TargetTemp = 65,
    [double]$InitialTemp = 42,
    [int]$Steps = 90,
    [double]$Dt = 1.0
)

$exeCandidates = @(
    "$PSScriptRoot\..\build\Release\eae_firmware.exe",
    "$PSScriptRoot\..\build\Debug\eae_firmware.exe",
    "$PSScriptRoot\..\build\eae_firmware.exe"
)

$exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $exe) {
    Write-Error "Executable not found. Build first with: cmake -S . -B build; cmake --build build --config Release"
    exit 1
}

& $exe --target-temp $TargetTemp --initial-temp $InitialTemp --steps $Steps --dt $Dt
