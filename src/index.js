var execSync = require("child_process").execSync;

require("dotenv").config();

const binPath = process.env.QML_PARSER_BIN_PATH || `${__dirname}/../vendor`;

function execute(arg) {
  var bin = `${binPath}/qml-parser`;
  var result = execSync(bin + " " + arg, { maxBuffer: Infinity });
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
  parseFile: parseFile,
};
