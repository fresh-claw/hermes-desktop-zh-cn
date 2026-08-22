#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "embedded_install_ps1.h"

static const wchar_t kLauncherScript[] =
    L"$ErrorActionPreference = 'Stop'\r\n"
    L"try { [Console]::OutputEncoding = [Text.Encoding]::Default } catch { }\r\n"
    L"$Host.UI.RawUI.WindowTitle = 'Hermes 中文增强安装器'\r\n"
    L"$base = $env:XIAOMA_HERMES_BASE_URL\r\n"
    L"if ([string]::IsNullOrWhiteSpace($base)) { $base = 'https://useai.live/hermes' }\r\n"
    L"$base = $base.TrimEnd('/')\r\n"
    L"$fallback = $env:XIAOMA_HERMES_FALLBACK_BASE_URL\r\n"
    L"if ([string]::IsNullOrWhiteSpace($fallback)) { $fallback = 'https://cdn.jsdelivr.net/gh/fresh-claw/hermes-cn@main' }\r\n"
    L"$fallback = $fallback.TrimEnd('/')\r\n"
    L"$exeDir = $env:XIAOMA_HERMES_EXE_DIR\r\n"
    L"$localPs1 = if ([string]::IsNullOrWhiteSpace($exeDir)) { '' } else { Join-Path $exeDir 'install.ps1' }\r\n"
    L"$embeddedPs1 = $env:XIAOMA_HERMES_EMBEDDED_INSTALL_PS1\r\n"
    L"function Pause-Hermes { Write-Host ''; Read-Host '按回车关闭窗口' | Out-Null }\r\n"
    L"function Test-InstallerPs1([string]$path) {\r\n"
    L"  if ([string]::IsNullOrWhiteSpace($path)) { return $false }\r\n"
    L"  if (-not (Test-Path $path)) { return $false }\r\n"
    L"  $text = Get-Content -Raw -Path $path -ErrorAction SilentlyContinue\r\n"
    L"  return ($text -and $text.Contains('Hermes 中文增强') -and $text.Contains('Find-HermesCommand') -and $text.Contains('Ensure-WindowsGitBash') -and $text.Contains('node-v22.22.3-win') -and $text.Contains('Get-LocalInstallerAssetPaths'))\r\n"
    L"}\r\n"
    L"Write-Host 'Hermes 中文增强安装器'\r\n"
    L"Write-Host ''\r\n"
    L"Write-Host '将安装官方 Hermes 桌面端，并应用中文增强。'\r\n"
    L"Write-Host ''\r\n"
    L"try {\r\n"
    L"  $installerPs1 = $null\r\n"
    L"  if (Test-InstallerPs1 $embeddedPs1) { $installerPs1 = $embeddedPs1 }\r\n"
    L"  elseif (Test-InstallerPs1 $localPs1) { $installerPs1 = $localPs1 }\r\n"
    L"  if ($installerPs1) {\r\n"
    L"    & powershell -NoProfile -ExecutionPolicy Bypass -File $installerPs1 -BaseUrl $base -FallbackBaseUrl $fallback\r\n"
    L"  } else {\r\n"
    L"    $tmp = Join-Path $env:TEMP 'xiaoma-hermes-install.ps1'\r\n"
    L"    $sources = @()\r\n"
    L"    if (-not [string]::IsNullOrWhiteSpace($env:XIAOMA_HERMES_BASE_URLS)) {\r\n"
    L"      $sources = $env:XIAOMA_HERMES_BASE_URLS -split ','\r\n"
    L"    } else {\r\n"
    L"      $sources = @($base, $fallback, 'https://fastly.jsdelivr.net/gh/fresh-claw/hermes-cn@main', 'https://gcore.jsdelivr.net/gh/fresh-claw/hermes-cn@main', 'https://raw.githubusercontent.com/fresh-claw/hermes-cn/main')\r\n"
    L"    }\r\n"
    L"    $active = $null\r\n"
    L"    foreach ($source in $sources) {\r\n"
    L"      if ([string]::IsNullOrWhiteSpace($source)) { continue }\r\n"
    L"      $clean = $source.Trim().TrimEnd('/')\r\n"
    L"      $url = if ($clean.EndsWith('/install.ps1')) { $clean } else { $clean + '/install.ps1' }\r\n"
    L"      try {\r\n"
    L"        Write-Host ('正在下载中文增强安装器：' + $url)\r\n"
    L"        Invoke-WebRequest -UseBasicParsing $url -OutFile $tmp -TimeoutSec 60\r\n"
    L"        if (-not (Test-InstallerPs1 $tmp)) { throw '当前入口返回的不是安装器脚本。' }\r\n"
    L"        $active = if ($clean.EndsWith('/install.ps1')) { $clean.Substring(0, $clean.Length - '/install.ps1'.Length).TrimEnd('/') } else { $clean }\r\n"
    L"        break\r\n"
    L"      } catch {\r\n"
    L"        Write-Host '当前入口不可用或过慢，正在切换下一个入口。'\r\n"
    L"      }\r\n"
    L"    }\r\n"
    L"    if (-not $active) { throw '中文增强安装器下载失败，请稍后重试。' }\r\n"
    L"    & powershell -NoProfile -ExecutionPolicy Bypass -File $tmp -BaseUrl $active -FallbackBaseUrl $fallback\r\n"
    L"  }\r\n"
    L"  if ($LASTEXITCODE -and $LASTEXITCODE -ne 0) { throw \"安装器退出码: $LASTEXITCODE\" }\r\n"
    L"  Write-Host ''\r\n"
    L"  Write-Host '完成。请重新打开 Hermes。'\r\n"
    L"  Pause-Hermes\r\n"
    L"} catch {\r\n"
    L"  Write-Host ''\r\n"
    L"  Write-Host '安装失败。请把这个窗口截图发给小马。' -ForegroundColor Yellow\r\n"
    L"  Write-Host $_\r\n"
    L"  Pause-Hermes\r\n"
    L"  exit 1\r\n"
    L"}\r\n";

static void pause_for_user(void) {
  fwprintf(stderr, L"\n按回车关闭窗口");
  (void)_getwch();
}

static void print_error(const wchar_t *message) {
  fwprintf(stderr, L"%ls\n", message);
}

static int write_utf8_script(const wchar_t *path) {
  int byte_count = WideCharToMultiByte(CP_UTF8, 0, kLauncherScript, -1, NULL, 0, NULL, NULL);
  if (byte_count <= 1) {
    return 0;
  }

  char *buffer = (char *)malloc((size_t)byte_count);
  if (!buffer) {
    return 0;
  }

  int converted = WideCharToMultiByte(CP_UTF8, 0, kLauncherScript, -1, buffer, byte_count, NULL, NULL);
  if (converted <= 1) {
    free(buffer);
    return 0;
  }

  HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    free(buffer);
    return 0;
  }

  const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  DWORD written = 0;
  BOOL ok = WriteFile(file, bom, sizeof(bom), &written, NULL);
  if (ok) {
    ok = WriteFile(file, buffer, (DWORD)(converted - 1), &written, NULL);
  }

  CloseHandle(file);
  free(buffer);
  return ok ? 1 : 0;
}

static int write_embedded_install_script(const wchar_t *path) {
  HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    return 0;
  }

  const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  DWORD written = 0;
  BOOL ok = WriteFile(file, bom, sizeof(bom), &written, NULL);
  if (ok) {
    ok = WriteFile(file, kEmbeddedInstallPs1Utf8, (DWORD)kEmbeddedInstallPs1Utf8_len, &written, NULL);
  }

  CloseHandle(file);
  return ok ? 1 : 0;
}

static void set_exe_dir_env(void) {
  wchar_t module_path[MAX_PATH];
  DWORD length = GetModuleFileNameW(NULL, module_path, MAX_PATH);
  if (length == 0 || length >= MAX_PATH) {
    return;
  }

  wchar_t *slash = wcsrchr(module_path, L'\\');
  if (slash) {
    *slash = L'\0';
    SetEnvironmentVariableW(L"XIAOMA_HERMES_EXE_DIR", module_path);
  }
}

int wmain(void) {
  SetConsoleOutputCP(GetACP());
  SetConsoleCP(GetACP());
  SetConsoleTitleW(L"Hermes 中文增强安装器");
  set_exe_dir_env();

  wchar_t temp_dir[MAX_PATH];
  DWORD temp_len = GetTempPathW(MAX_PATH, temp_dir);
  if (temp_len == 0 || temp_len >= MAX_PATH) {
    print_error(L"无法读取临时目录。");
    pause_for_user();
    return 1;
  }

  wchar_t script_path[MAX_PATH];
  int path_len = swprintf(script_path, MAX_PATH, L"%lsxiaoma-hermes-windows-launcher.ps1", temp_dir);
  if (path_len <= 0 || path_len >= MAX_PATH) {
    print_error(L"临时脚本路径过长。");
    pause_for_user();
    return 1;
  }

  if (!write_utf8_script(script_path)) {
    print_error(L"无法创建临时启动脚本。");
    pause_for_user();
    return 1;
  }

  wchar_t embedded_script_path[MAX_PATH];
  int embedded_path_len =
      swprintf(embedded_script_path, MAX_PATH, L"%lsxiaoma-hermes-embedded-install.ps1", temp_dir);
  if (embedded_path_len <= 0 || embedded_path_len >= MAX_PATH) {
    print_error(L"内置安装脚本路径过长。");
    pause_for_user();
    return 1;
  }

  if (!write_embedded_install_script(embedded_script_path)) {
    print_error(L"无法创建内置安装脚本。");
    pause_for_user();
    return 1;
  }
  SetEnvironmentVariableW(L"XIAOMA_HERMES_EMBEDDED_INSTALL_PS1", embedded_script_path);

  wchar_t command_line[MAX_PATH * 2 + 128];
  int command_len = swprintf(
      command_line,
      sizeof(command_line) / sizeof(command_line[0]),
      L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%ls\"",
      script_path);
  if (command_len <= 0 || command_len >= (int)(sizeof(command_line) / sizeof(command_line[0]))) {
    print_error(L"启动命令过长。");
    pause_for_user();
    return 1;
  }

  STARTUPINFOW startup;
  PROCESS_INFORMATION process;
  ZeroMemory(&startup, sizeof(startup));
  ZeroMemory(&process, sizeof(process));
  startup.cb = sizeof(startup);

  if (!CreateProcessW(NULL, command_line, NULL, NULL, TRUE, 0, NULL, NULL, &startup, &process)) {
    print_error(L"无法启动 PowerShell。");
    pause_for_user();
    return 1;
  }

  WaitForSingleObject(process.hProcess, INFINITE);
  DWORD exit_code = 1;
  GetExitCodeProcess(process.hProcess, &exit_code);
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
  return (int)exit_code;
}
