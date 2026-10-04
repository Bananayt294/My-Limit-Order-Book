$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build-tests"

cmake -S $ProjectRoot -B $BuildDir -G "MinGW Makefiles"
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

cmake --build $BuildDir --parallel
if ($LASTEXITCODE -ne 0) {
    throw "Test build failed."
}

ctest --test-dir $BuildDir --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "One or more tests failed."
}
