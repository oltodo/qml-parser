var execSync = require("child_process").execSync;

function execute(arg) {
  var bin = __dirname + "/qml-parser.app/Contents/MacOS/qml-parser";

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
