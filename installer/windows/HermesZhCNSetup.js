const fs = require("fs");
const os = require("os");
const path = require("path");
const https = require("https");
const http = require("http");
const readline = require("readline");
const { spawnSync } = require("child_process");

const DEFAULT_BASE_URL = "https://useai.live/hermes";
const DEFAULT_FALLBACK_BASE_URL = "https://cdn.jsdelivr.net/gh/fresh-claw/hermes-cn@main";

function envOrDefault(name, fallback) {
  const value = process.env[name];
  return value && value.trim() ? value.trim() : fallback;
}

function pause() {
  return new Promise((resolve) => {
    const rl = readline.createInterface({ input: process.stdin, output: process.stdout });
    rl.question("按回车关闭窗口", () => {
      rl.close();
      resolve();
    });
  });
}

function downloadText(url, redirects = 0) {
  return new Promise((resolve, reject) => {
    const client = url.startsWith("https:") ? https : http;
    const request = client.get(
      url,
      {
        headers: {
          "User-Agent": "HermesZhCNSetup/2026.06.06",
        },
        timeout: 30000,
      },
      (response) => {
        const status = response.statusCode || 0;
        if (status >= 300 && status < 400 && response.headers.location && redirects < 5) {
          response.resume();
          const nextUrl = new URL(response.headers.location, url).toString();
          downloadText(nextUrl, redirects + 1).then(resolve, reject);
          return;
        }
        if (status < 200 || status >= 300) {
          response.resume();
          reject(new Error(`下载失败：${status} ${url}`));
          return;
        }
        response.setEncoding("utf8");
        let body = "";
        response.on("data", (chunk) => {
          body += chunk;
        });
        response.on("end", () => resolve(body));
      },
    );
    request.on("timeout", () => {
      request.destroy(new Error(`下载超时：${url}`));
    });
    request.on("error", reject);
  });
}

function looksLikeInstaller(script) {
  return script.includes("param(")
    && script.includes("Hermes 中文增强")
    && script.includes("Find-HermesCommand")
    && script.includes("Ensure-WindowsGitBash")
    && script.includes("node-v22.22.3-win");
}

async function resolveInstallerScript(baseUrl, fallbackBaseUrl) {
  const exeDir = path.dirname(process.execPath);
  const localScript = path.join(exeDir, "install.ps1");
  if (fs.existsSync(localScript)) {
    const localBody = fs.readFileSync(localScript, "utf8");
    if (looksLikeInstaller(localBody)) {
      return { scriptPath: localScript, activeBaseUrl: baseUrl };
    }
  }

  const tempScript = path.join(os.tmpdir(), "xiaoma-hermes-install.ps1");
  const sources = [baseUrl, fallbackBaseUrl, "https://fastly.jsdelivr.net/gh/fresh-claw/hermes-cn@main"];
  let lastError = null;
  for (const source of sources) {
    try {
      const script = await downloadText(`${source}/install.ps1`);
      if (!looksLikeInstaller(script)) {
        throw new Error("当前入口返回的内容不是安装器脚本。");
      }
      fs.writeFileSync(tempScript, `\ufeff${script}`, "utf8");
      return { scriptPath: tempScript, activeBaseUrl: source };
    } catch (error) {
      lastError = error;
      console.log("当前入口不可用或过慢，正在切换下一个入口。");
      console.log(error.message);
    }
  }
  if (lastError) {
    throw lastError;
  }
  throw new Error("中文增强安装器下载失败。");
}

function runPowerShellInstaller(scriptPath, baseUrl, fallbackBaseUrl) {
  const result = spawnSync(
    "powershell.exe",
    [
      "-NoProfile",
      "-ExecutionPolicy",
      "Bypass",
      "-File",
      scriptPath,
      "-BaseUrl",
      baseUrl,
      "-FallbackBaseUrl",
      fallbackBaseUrl,
    ],
    { stdio: "inherit" },
  );
  if (result.error) {
    throw result.error;
  }
  if (result.status !== 0) {
    throw new Error(`安装器退出码：${result.status}`);
  }
}

async function main() {
  process.title = "Hermes 中文增强安装器";
  const baseUrl = envOrDefault("XIAOMA_HERMES_BASE_URL", DEFAULT_BASE_URL).replace(/\/+$/, "");
  const fallbackBaseUrl = envOrDefault("XIAOMA_HERMES_FALLBACK_BASE_URL", DEFAULT_FALLBACK_BASE_URL).replace(/\/+$/, "");

  console.log("Hermes 中文增强安装器");
  console.log("");
  console.log("将安装官方 Hermes 桌面端，并应用中文增强。");
  console.log("");

  try {
    const resolved = await resolveInstallerScript(baseUrl, fallbackBaseUrl);
    runPowerShellInstaller(resolved.scriptPath, resolved.activeBaseUrl, fallbackBaseUrl);
    console.log("");
    console.log("完成。请重新打开 Hermes。");
  } catch (error) {
    console.log("");
    console.log("安装失败。请把这个窗口截图发给小马。");
    console.log(error.message || error);
    process.exitCode = 1;
  }
  console.log("");
  await pause();
}

main();
