const { parseFile } = require("../../bin");

it("renders correctly", () => {
  const ast = parseFile(`${__dirname}/entry.qml`);

  expect(ast).toMatchSnapshot();
});
