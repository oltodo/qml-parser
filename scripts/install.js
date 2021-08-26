const fs = require("fs");
const fetch = require("node-fetch");
const semver = require("semver");
const unzipper = require("unzipper");
const { chmodSync } = require("fs-chmod");
const pkg = require("../package.json");

require("dotenv").config();

const supportedPlatforms = ["darwin", "linux"];

function getBinaryUrl() {
  const endpoint = "https://github.com/oltodo/qml-parser/releases/download";
  const version = semver.coerce(pkg.version);
  const { platform } = process;

  if (!supportedPlatforms.includes(platform)) {
    throw new Error(`Unsupported ${platform} platform.`);
  }

  // https://github.com/oltodo/qml-parser/releases/download/v0.6.0/macos.zip
  return `${endpoint}/v${version}/${platform}.zip`;
}

(async function main() {
  if (process.env.QML_PARSER_DISABLE_DOWNLOAD) {
    console.log(
      '**INFO** Skipping binary download. "QML_PARSER_DISABLE_DOWNLOAD" environment variable was found.'
    );
    return;
  }

  const binaryUrl = getBinaryUrl();
  const installPath = `${__dirname}/../vendor`;

  console.log("**INFO** Downloading binary from", binaryUrl);

  fs.rmdirSync(installPath, { recursive: true });
  fs.mkdirSync(installPath);

  const res = await fetch(binaryUrl);

  if (!res.ok) {
    throw new Error("Unable to fetch binaries");
  }

  res.body.pipe(unzipper.Extract({ path: installPath }));

  res.body.on("end", () => {
    if (process.platform === "darwin") {
      fs.symlinkSync(
        `qml-parser.app/Contents/MacOS/qml-parser`,
        `${installPath}/qml-parser`
      );
    }

    chmodSync(`${installPath}/qml-parser`, "+x");
  });
})();
