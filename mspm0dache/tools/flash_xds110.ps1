$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$dslite = "C:\ti\ccs\ccs_base\DebugServer\bin\DSLite.exe"
$config = Join-Path $projectRoot "targetConfigs\mspm0g3507_xds110.ccxml"
$elf = Join-Path $projectRoot "ticlang\mspm0dache.out"

foreach ($path in @($dslite, $config, $elf)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required file not found: $path"
    }
}

$env:TI_APPDATA_DIR = Join-Path $projectRoot ".ti-appdata"

Write-Host "Loading TEST_MODE firmware through the onboard XDS110..."
& $dslite load "--config=$config" $elf
if ($LASTEXITCODE -ne 0) {
    throw "DSLite failed with exit code $LASTEXITCODE"
}

Write-Host "PASS: mspm0dache.out was loaded and the target was started." `
    -ForegroundColor Green
