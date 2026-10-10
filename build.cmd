@echo off
setlocal
if not defined LLVM_MINGW set "LLVM_MINGW=%LOCALAPPDATA%\Programs\llvm-mingw\llvm-mingw-20261006-msvcrt-x86_64"
if not exist "%LLVM_MINGW%\bin\clang.exe" (
    echo LLVM toolchain not found. Set LLVM_MINGW to its directory.
    exit /b 1
)
pushd "%~dp0"
set "keygen_sources=keygen.c pe\pe.c pe\scan.c pe\constants.c app\console.c app\key.c app\registration.c app\rng.c"
set "keygen_crypto=crypto\sha1.c crypto\crc32.c crypto\gf15.c crypto\field.c crypto\curve.c crypto\bigint.c crypto\chacha20.c crypto\derive.c crypto\signature.c"
"%LLVM_MINGW%\bin\clang.exe" --target=x86_64-w64-windows-gnu -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind -std=c99 -O2 -march=x86-64 -Wall -Wextra -Wpedantic -Werror -s -o keygen.exe %keygen_sources% %keygen_crypto% -lbcrypt
set "keygen_result=%errorlevel%"
popd
exit /b %keygen_result%
