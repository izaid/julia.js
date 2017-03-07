var assert = require('assert');

var julia = require('../build/Release/julia.node');

describe("Convert", () => {
    it("Bool", () => {
        assert.strictEqual(true, julia.eval("true"));
        assert.strictEqual(false, julia.eval("false"));
    });

    it("Int32", () => {
        assert.strictEqual(0, julia.eval("Int32(0)"));
        assert.strictEqual(1, julia.eval("Int32(1)"));
    });

    it("Int64", () => {
        assert.strictEqual(0, julia.eval("Int64(0)"));
        assert.strictEqual(1, julia.eval("Int64(1)"));
    });

    it("Float32", () => {
        assert.strictEqual(0.0, julia.eval("0.0f0"));
        assert.strictEqual(0.5, julia.eval("0.5f0"));
    });

    it("Float64", () => {
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

    it("Tuple", () => {
        // ...
    });

    it("Array", () => {
        assert.deepEqual(new julia.ArrayDescriptor([5], new Float32Array([0, 1, 2, 3, 4])), julia.eval("[0.0f0, 1.0f0, 2.0f0, 3.0f0, 4.0f0]"));
        assert.deepEqual(new julia.ArrayDescriptor([
            2, 2
        ], new Float32Array([0, 1, 2, 3])), julia.eval("[[0.0f0, 1.0f0] [2.0f0, 3.0f0]]"));

        //        for (var i = 0; i < 2500; ++i) {
        //          var a = julia.eval("zeros(Float32, 512 * 512)");
        //    }
    });

    it("Function", () => {
        let f = julia.eval("() -> 0");
        assert(f instanceof Function);
        assert.strictEqual(0, f());

        f = julia.eval("() -> \"Hello, world!\"");
        assert(f instanceof Function);
        assert.strictEqual("Hello, world!", f());

        f = julia.eval("x -> x");
        assert(f instanceof Function);
        assert.strictEqual(10.0, f(10.0));

        f = julia.eval("x -> x + 1");
        assert(f instanceof Function);
        assert.strictEqual(11.0, f(10.0));

        f = julia.eval("(x, y) -> x + y");
        assert(f instanceof Function);
        assert.strictEqual(5, f(2, 3));
    });

    it("Type", () => {
        assert.strictEqual(Boolean, julia.eval("Bool"));

        let Foo = julia.eval(`type Foo
           qux::Bool
           count::Int32
       end; function (self::Foo)(x) 12 + x + self.count end; Foo`);
        console.log(Foo)

        var val = new Foo(true, 1);
        assert(val instanceof Foo);
        console.dir(val);

        var res = val(4);
        console.log(res);
    });
});
