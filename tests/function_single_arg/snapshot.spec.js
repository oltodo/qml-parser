const fs = require("fs");
const { parse } = require("../../src");

it("renders correctly", () => {
  const ast = parse(fs.readFileSync(`${__dirname}/entry.qml`).toString());

  expect(ast).toMatchSnapshot();
});
