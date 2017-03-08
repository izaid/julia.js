var assert = require('assert');

var Julia = require('../build/Release/julia.node');

//var a = new Julia.Array({dims: [10], data: null});
//console.dir(a);

//var t = new Julia.Tuple({
//  elements: [0, 1, 2, 3, 4]
//});

describe("Convert", () => {
    it("Bool", () => {
        assert.strictEqual(true, Julia.eval("true"));
        assert.strictEqual(false, Julia.eval("false"));
    });

    it("Int32", () => {
        assert.strictEqual(0, Julia.eval("Int32(0)"));
        assert.strictEqual(1, Julia.eval("Int32(1)"));
    });

    it("Int64", () => {
        assert.strictEqual(0, Julia.eval("Int64(0)"));
        assert.strictEqual(1, Julia.eval("Int64(1)"));
    });

    it("Float32", () => {
        assert.strictEqual(0.0, Julia.eval("0.0f0"));
        assert.strictEqual(0.5, Julia.eval("0.5f0"));
    });

    it("Float64", () => {
        assert.strictEqual(0.0, Julia.eval("0.0"));
        assert.strictEqual(0.5, Julia.eval("0.5"));
    });

    it("String", () => {
        assert.strictEqual("Hello, world!", Julia.eval("\"Hello, world!\""));
        assert.notStrictEqual("Hello, world!", Julia.eval("\"Hello, wo\""));
    });

    it("Nothing", () => {
        assert.strictEqual(null, Julia.eval("nothing"));
    });

    it("Tuple", () => {
        assert.deepEqual(new Julia.Tuple([]), Julia.eval("()"));

        assert.deepEqual(new Julia.Tuple(["Hello, world!"]), Julia.eval("(\"Hello, world!\",)"));

        assert.deepEqual(new Julia.Tuple([0, 1, 2, 3, 4]), Julia.eval("(0, 1, 2, 3, 4)"));
    });

    it("Array", () => {
        assert.deepEqual(new Julia.Array({
            dims: [5],
            data: new Float32Array([0, 1, 2, 3, 4])
        }), Julia.eval("[0.0f0, 1.0f0, 2.0f0, 3.0f0, 4.0f0]"));
        assert.deepEqual(new Julia.Array({
            dims: [
                2, 2
            ],
            data: new Float32Array([0, 1, 2, 3])
        }), Julia.eval("[[0.0f0, 1.0f0] [2.0f0, 3.0f0]]"));

        //        for (var i = 0; i < 2500; ++i) {
        //          var a = julia.eval("zeros(Float32, 512 * 512)");
        //    }
    });

    it("Function", () => {
        let f = Julia.eval("() -> 0");
        assert(f instanceof Function);
        assert.strictEqual(0, f());

        f = Julia.eval("() -> \"Hello, world!\"");
        assert(f instanceof Function);
        assert.strictEqual("Hello, world!", f());

        f = Julia.eval("x -> x");
        assert(f instanceof Function);
        assert.strictEqual(10.0, f(10.0));

        f = Julia.eval("x -> x + 1");
        assert(f instanceof Function);
        assert.strictEqual(11.0, f(10.0));

        f = Julia.eval("(x, y) -> x + y");
        assert(f instanceof Function);
        assert.strictEqual(5, f(2, 3));
    });

    it("Type", () => {
        assert.strictEqual(Boolean, Julia.eval("Bool"));

        let Foo = Julia.eval(`type Foo
           qux::Bool
           count::Int32
       end; function (self::Foo)(x) 12 + x + self.count end; Foo`);
        console.log(Foo)

        var val = new Foo(true, 1);
        assert(val instanceof Foo);
        assert.deepEqual([
            "qux", "count"
        ], Object.keys(val));
        assert.strictEqual(true, val.qux);
        assert.strictEqual(1, val.count);

        assert.strictEqual(17, val(4));
    });

    it("Module", () => {
        let module = Julia.$.Test;
        console.dir(module);
    });
});
