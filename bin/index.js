var execSync = require("child_process").execSync;
var os = require('os');

function getBinPath() {
  switch(os.platform()) {
    case "darwin":
      return __dirname + "/packages/macos/Contents/MacOS/qml-parser";
    case "linux":
      return __dirname + "/packages/linux/qml-parser";
    default:
      throw new Error(`Unsupported ${os.platform()} platform.`)
  }
}

function execute(arg) {
  var bin = getBinPath();
  var result = execSync(bin + " " + arg);
  var ast = JSON.parse(result);

  if (ast === null) {
    return {};
  }

  return ast;
}

function parse(code) {
  return execute(Buffer.from(code).toString("base64"));
}

function parseFile(filepath) {
  return execute(filepath);
}

module.exports = {
  parse: parse,
  parseFile: parseFile
};
