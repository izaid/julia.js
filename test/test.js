var assert = require('assert');

var julia = require('../build/Release/julia.node');

describe("Convert", () => {
    it("Bool", () => {
        assert.strictEqual(true, julia.eval("true"));
        assert.strictEqual(false, julia.eval("false"));
    });

    it("Double", () => {
        assert.strictEqual(0.5, julia.eval("0.5"));
    });

    it("String", () => {
        assert.strictEqual("Hello, world!", julia.eval("\"Hello, world!\""));
        assert.notStrictEqual("Hello, world!", julia.eval("\"Hello, wo\""));
    });

    it("Nothing", () => {
        assert.strictEqual(null, julia.eval("nothing"));
    });
});
