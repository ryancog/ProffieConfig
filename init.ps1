# ProffieConfig, All-In-One Proffieboard Management Utility
# Copyright (C) 2025 Ryan Ogurek

$ErrorActionPreference = "Stop"
$ROOT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ROOT_DIR

function Check-Exec {
    param([string]$Cmd)

    if (-not (Get-Command $Cmd -ErrorAction SilentlyContinue)) {
        Write-Error "$Cmd is missing, please install it!"
    }
}

Check-Exec git

# https://github.com/majkinetor/posh/blob/master/MM_Admin/Invoke-Environment.ps1
function Invoke-Environment {
    param
    (
        # Any cmd shell command, normally a configuration batch file.
        [Parameter(Mandatory=$true)]
        [string] $Command
    )

    $Command = "`"" + $Command + "`""
    cmd /c "$Command > nul 2>&1 && set" | . { process {
        if ($_ -match '^([^=]+)=(.*)') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }}

}
Invoke-Environment "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" | out-null

Write-Output "Initializing 3rd party repositories..."
git submodule update --init

Write-Output  "Setting up libtomcrypt..."
cd 3rdparty\tomcrypt

if (Test-Path -Path "build-win32") {
    Write-Output "Tomcrypt already built, skipping! (Remove 'build' directory to reset)"
} else {
    nmake /NOLOGO /S -f makefile.msvc clean clean | out-null

    Write-Output "    Building..."
    nmake /NOLOGO /S -f makefile.msvc CFLAGS=/Ox EXTRALIBS= | out-null

    New-Item -ItemType Directory -Force -Path build-win32 | out-null
    move tomcrypt.lib build-win32/tomcrypt.lib
}

cd ..\..

Write-Output "Preparing wxWidgets..."
cd 3rdparty\wxWidgets

Write-Output "    Initializing repository..."
git submodule update --init | out-null

if (Test-Path -Path "install-win32-static") {
    Write-Output "    wxWidgets static already built, skipping! (Remove install-win32-static to reset)"
} else {
    # if (Test-Path "build-win32-static") {
    #     Remove-Item -Path "build-win32-static" -Recurse -Force | out-null
    # }

    Write-Output "    Configuring static..."
    cmake -S . -B build-win32-static `
        -DCMAKE_INSTALL_PREFIX=install-win32-static `
        -DwxBUILD_SHARED=OFF `
        -DCMAKE_BUILD_TYPE=Release `
        -DwxUSE_UNICODE_UTF8=ON -DwxUSE_UNSAFE_WXSTRING_CONV=OFF `
        -DwxUSE_OPENGL=OFF -DwxUSE_AUI=OFF -DwxUSE_HTML=OFF -DwxUSE_MEDIACTRL=OFF `
        -DwxUSE_PROPGRID=OFF -DwxUSE_RIBBON=OFF -DwxUSE_WEBVIEW=OFF `
        -DwxUSE_XRC=OFF -DwxUSE_WXHTML_HELP=OFF -DwxUSE_RICHTEXT=OFF -DwxUSE_STC=OFF `
        -DwxUSE_DEBUGREPORT=OFF -DwxUSE_XML=OFF

    cd build-win32-static
    Write-Output "    Building static..."
    cmake --build . --config Release --target install | out-null
    cd ..
}

if ((Test-Path -Path "install-win32-shared-dbg") -and (Test-Path -Path "install-win32-shared-rel")) {
    Write-Output "    wxWidgets shared already built, skipping! (Remove install-win32-shared to reset)"
} else {
    # if (Test-Path "build-win32-shared") {
    #     Remove-Item -Path "build-win32-shared" -Recurse -Force | out-null
    # }

    $wxFlags = @(
        "-DwxUSE_UNICODE_UTF8=ON"
        "-DwxUSE_UNSAFE_WXSTRING_CONV=OFF"
        "-DwxUSE_OPENGL=OFF"
        "-DwxUSE_AUI=OFF"
        "-DwxUSE_HTML=OFF"
        "-DwxUSE_MEDIACTRL=OFF"
        "-DwxUSE_PROPGRID=OFF"
        "-DwxUSE_RIBBON=OFF"
        "-DwxUSE_WEBVIEW=OFF"
        "-DwxUSE_XRC=OFF"
        "-DwxUSE_WXHTML_HELP=OFF"
        "-DwxUSE_RICHTEXT=OFF"
        "-DwxUSE_STC=OFF"
        "-DwxUSE_DEBUGREPORT=OFF"
        "-DwxUSE_XML=OFF"
        "-DwxUSE_EXCEPTIONS=OFF"
    )

    Write-Output "    Configuring shared..."
    cmake -S . -B build-win32-shared-rel `
        -DCMAKE_INSTALL_PREFIX=install-win32-shared-rel `
        -DCMAKE_BUILD_TYPE=Release $wxFlags

    cmake -S . -B build-win32-shared-dbg `
        -DCMAKE_INSTALL_PREFIX=install-win32-shared-dbg `
        -DCMAKE_BUILD_TYPE=Debug $wxFlags

    Write-Output "    Building shared..."
    cd build-win32-shared-rel
    cmake --build . --config Release --target install
    cd ..
    cd build-win32-shared-dbg
    cmake --build . --config Debug --target install
    cd ..

    # Hack to ignore _DEBUG for wxWidgets setup.h
    # $SetupFile = "install-win32-shared/include/msvc/wx/setup.h"
    # (Get-Content $SetupFile).Replace('#ifdef _DEBUG', '#if 0') | Set-Content $SetupFile
    # $SetupFile = "install-win32-static/include/msvc/wx/setup.h"
    # (Get-Content $SetupFile).Replace('#ifdef _DEBUG', '#if 0') | Set-Content $SetupFile
}

cd ..\..

Write-Output "Done."

