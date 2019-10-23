var execSync = require("child_process").execSync;

function escapeString(str) {
  return str.replace(/("|`)/g, "\\$1");
}

function parse(code) {
  var bin = __dirname + "/qml-parser.app/Contents/MacOS/qml-parser";

  var result = execSync(bin + ' "' + escapeString(code) + '"');
  var ast = JSON.parse(result);

  if (ast === null) {
    return {};
  }

  return ast;
}

module.exports = {
  parse: parse
};
