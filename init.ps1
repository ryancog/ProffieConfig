# ProffieConfig, All-In-One Proffieboard Management Utility
# Copyright (C) 2025-2026 Ryan Ogurek
#
# This is a pseudo-equivalent to init.sh specifically for Windows with MSVC.
# It's a lot more "thrown-together" (not that the init.sh is pretty) and I
# know even less about PowerShell than I do about bash, so I'm sure there's
# better/more PowerShell-y ways to do a lot of this.

$ErrorActionPreference = "Stop"
$ROOT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path

Push-Location $ROOT_DIR

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
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
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

cd ..\.. # 3rdparty\tomcrypt

Write-Output "Preparing wxWidgets..."
cd 3rdparty\wxWidgets

Write-Output "    Initializing repository..."
git submodule update --init | out-null

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
    "-DwxUSE_REGEX=OFF"
    "-DwxUSE_LIBJPEG=OFF"
    "-DwxUSE_LIBTIFF=OFF"
    "-DwxUSE_LIBWEBP=OFF"
    "-DwxUSE_EXPAT=OFF"
    "-DwxUSE_WINSOCK2=ON"
)

if ((Test-Path -Path "install-win32-static-dbg") -and (Test-Path -Path "install-win32-static-rel")) {
    Write-Output "    wxWidgets static already built, skipping! (Remove install-win32-static to reset)"
} else {
    if (Test-Path "build-win32-static-rel") {
        Remove-Item -Path "build-win32-static-rel" -Recurse -Force | out-null
    }
    if (Test-Path "build-win32-static-dbg") {
        Remove-Item -Path "build-win32-static-dbg" -Recurse -Force | out-null
    }

    Write-Output "    Configuring static..."
    cmake -S . -B build-win32-static-rel `
        -DCMAKE_INSTALL_PREFIX=install-win32-static-rel `
        -DwxBUILD_SHARED=OFF `
        -DCMAKE_BUILD_TYPE=Release $wxFlags

    cmake -S . -B build-win32-static-dbg `
        -DCMAKE_INSTALL_PREFIX=install-win32-static-dbg `
        -DwxBUILD_SHARED=OFF `
        -DCMAKE_BUILD_TYPE=Debug $wxFlags

    Write-Output "    Building static..."

    cd build-win32-static-rel
    cmake --build . --config Release --target install | out-null
    cd .. # build-win32-static-rel

    cd build-win32-static-dbg
    cmake --build . --config Debug --target install | out-null
    cd .. # build-win32-static-dbg
}

if ((Test-Path -Path "install-win32-shared-dbg") -and (Test-Path -Path "install-win32-shared-rel")) {
    Write-Output "    wxWidgets shared already built, skipping! (Remove install-win32-shared to reset)"
} else {
    if (Test-Path "build-win32-shared-rel") {
        Remove-Item -Path "build-win32-shared-rel" -Recurse -Force | out-null
    }
    if (Test-Path "build-win32-shared-dbg") {
        Remove-Item -Path "build-win32-shared-dbg" -Recurse -Force | out-null
    }

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
    cd .. # build-win32-shared-rel

    cd build-win32-shared-dbg
    cmake --build . --config Debug --target install
    cd .. # build-win32-shared-dbg
}

# DLL Patching

# This (Read/WriteAllBytes) I guess needs a full path (FullName)...
function ReplaceIn {
    param(
        [string]$file,
        [string]$oldString,
        [string]$newString
    )

    $bytes = [IO.File]::ReadAllBytes($file)
    $old = [Text.Encoding]::ASCII.GetBytes($oldString)
    $new = [Text.Encoding]::ASCII.GetBytes($newString)

    # In order to find index of a whole string, have to convert the bytes to
    # a string...
    $binText = [Text.Encoding]::ASCII.GetString($bytes)

    $pos = $binText.IndexOf($oldString)
    while ($pos -ge 0) {
        [Array]::Clear($bytes, $pos, $old.Length)
        [Array]::Copy($new, 0, $bytes, $pos, $new.Length)

        $pos = $binText.IndexOf($oldString, $pos + 1)
    }

    [IO.File]::WriteAllBytes($file, $bytes)
}

function PatchIn {
    param(
        [string]$path
    )
    Push-Location $path

    # Just clear out all the libs, they're regenerated below
    Get-ChildItem *.lib | ForEach-Object {
        Remove-Item $_
    }

    Get-ChildItem *.dll | ForEach-Object {
        $oldName = $_.Name
        $newName = $oldName `
            -replace '^(wx.*)[0-9]{3}u(.*)_vc_x64_custom\.dll$', '$1u$2.dll'

        Rename-Item $_ $newName
        $newPath = Join-Path $_.DirectoryName $newName

        ReplaceIn $newPath $oldName $newName

        Get-ChildItem *.dll | ForEach-Object {
            ReplaceIn $_.FullName $oldName $newName
        }
    }

    # Regenerate libs after all dll patching is done
    Get-ChildItem *.dll | ForEach-Object {
        # Nothing quite like parsing text as structured data
        $exports = dumpbin /exports $_.FullName |
            ForEach-Object {
                if ($_ -match "^\s+\d+\s+\w+\s+\w+\s+(\S+).*$") {
                    $matches[1]
                }
            }

        $name = $_.BaseName
        $defName = $name + ".def"
        $expName = $name + ".exp"
        $libName = $name + ".lib"

        "LIBRARY $($_.Name)`nEXPORTS`n$($exports -join "`n")" |
            Set-Content $defName

        lib /def:$defName /machine:x64 /out:$libName | out-null

        # lib creates unneeded .exp artifacts
        Remove-Item $expName
        Remove-Item $defName
    }

    # In addition to patching the `.lib`s, also need to patch the `setup.h`
    # which uses MSVC `#pragma commant(lib)` to pull in lib files to use the
    # new names.
    #
    # This is specifically shooting for `#define wxWX_LIB_NAME`
    # The only difference the `.lib` rename here should be is the lack of
    # version numbers.
    #
    # I think this means I could either completely remove the pragmas for wx
    # libs in here and rely on CMake or remove the CMake definitions and rely
    # on the pragmas, but this works and I don't feel like messing with it more
    # right now.

    $SetupFile = "../../include/msvc/wx/setup.h"
    $SetupContent = Get-Content $SetupFile
    $SetupContent.Replace(
        '"wx" name wxSHORT_VERSION_STRING',
        '"wx" name'
    ) | Set-Content $SetupFile

    Pop-Location
}

PatchIn "install-win32-shared-rel\lib\vc_x64_dll"
PatchIn "install-win32-shared-dbg\lib\vc_x64_dll"

cd ..\.. # 3rdparty\wxWidgets

Write-Output "Done."

Pop-Location

