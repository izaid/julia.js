var assert = require('assert');

var julia = require('../build/Release/julia.node');

var d = julia.ArrayDescriptor;
var o = new julia.ArrayDescriptor();

describe("Convert", () => {
    it("Bool", () => {
        assert.strictEqual(true, julia.eval("true"));
        assert.strictEqual(false, julia.eval("false"));
    });

    it("Integer", () => {
        assert.strictEqual(0, julia.eval("0"));
    });

    it("Double", () => {
        assert.strictEqual(0.0, julia.eval("0.0"));
        assert.strictEqual(0.5, julia.eval("0.5"));
    });

    it("String", () => {
        assert.strictEqual("Hello, world!", julia.eval("\"Hello, world!\""));
        assert.notStrictEqual("Hello, world!", julia.eval("\"Hello, wo\""));
    });

    it("Nothing", () => {
        assert.strictEqual(null, julia.eval("nothing"));
    });

    it("Array", () => {
        assert.deepEqual(new julia.ArrayDescriptor([5], new Float32Array([0, 1, 2, 3, 4])), julia.eval("[0.0f0, 1.0f0, 2.0f0, 3.0f0, 4.0f0]"));
        assert.deepEqual(new julia.ArrayDescriptor([
            2, 2
        ], new Float32Array([0, 1, 2, 3])), julia.eval("[[0.0f0, 1.0f0] [2.0f0, 3.0f0]]"));
    });
});
