var assert = require('assert');

var Julia = require('../julia');

after(function() {
    for (let i = 0; i < 1000; ++i) {
        Julia.eval("rand(500, 500)");
    }

    global.gc();
});

describe("Julia", function() {
    it("Exception", function() {
        assert.throws(() => {
            Julia.eval("assert(false)");
        }, /^Error: AssertionError/);

        assert.throws(() => {
            Julia.eval("[0, 1, 2, 3, 4][6]");
        }, /^Error: BoundsError/);

        assert.throws(() => {
            Julia.eval("sqrt(-1)");
        }, /^Error: DomainError/);

        assert.throws(() => {
            Julia.eval("convert(Int, 0.5)");
        }, /^Error: InexactError/);

        assert.throws(() => {
            Julia.eval("Dict(:x => 0, :y => 1)[:z]");
        }, /^Error: KeyError/);

        assert.throws(() => {
            Julia.eval("0 + \"Hello, world!\"");
        }, /^Error: MethodError/);

        assert.throws(() => {
            Julia.eval("x");
        }, /^Error: UndefVarError/);
    });

    it("Bool", function() {
        assert.strictEqual(true, Julia.eval("true"));
        assert.strictEqual(false, Julia.eval("false"));
    });

    it("Int32", function() {
        assert.strictEqual(0, Julia.eval("Int32(0)").valueOf());
        assert.strictEqual(1, Julia.eval("Int32(1)").valueOf());
        assert.strictEqual(1, Julia.eval("Int32(1)").valueOf());
    });

    it("Int64", function() {
        assert.strictEqual(0, Julia.eval("Int64(0)"));
        assert.strictEqual(1, Julia.eval("Int64(1)"));
    });

    it("Float32", function() {
        assert.strictEqual(0.0, Julia.eval("0.0f0").valueOf());
        assert.strictEqual(0.5, Julia.eval("0.5f0").valueOf());
        assert.strictEqual(0.5, Julia.eval("0.5f0").valueOf());
    });

    it("Float64", function() {
        assert.strictEqual(0.0, Julia.eval("0.0"));
        assert.strictEqual(0.5, Julia.eval("0.5"));
    });

    it("Complex64", function() {
        assert.deepStrictEqual({
            re: 0.0,
            im: 1.0
        }, Julia.eval("1.0f0im").valueOf());
    });

    it("Complex128", function() {
        let Complex128 = Julia.eval("Complex128");

        assert.deepStrictEqual({
            re: 0.0,
            im: 1.0
        }, Julia.eval("1.0im").valueOf());
        assert.deepStrictEqual({
            re: 0.0,
            im: 1.0
        }, (new Complex128(0.0, 1.0)).valueOf());

        assert.deepStrictEqual({
            re: 1.0,
            im: 1.0
        }, Julia.eval("1.0 + 1.0im").valueOf());
        assert.deepStrictEqual({
            re: 1.0,
            im: 1.0
        }, (new Complex128(1.0, 1.0)).valueOf());
    });

    it("String", function() {
        assert.strictEqual("Hello, world!", Julia.eval("\"Hello, world!\""));
        assert.notStrictEqual("Hello, world!", Julia.eval("\"Hello, wo\""));
    });

    it("Nothing", function() {
        assert.strictEqual(null, Julia.eval("nothing"));
    });

    it("Tuple", function() {
        assert.deepStrictEqual([], Julia.eval("()").valueOf());

        assert.deepStrictEqual(["Hello, world!"], Julia.eval("(\"Hello, world!\",)").valueOf());

        assert.deepStrictEqual([
            0, 1, 2, 3, 4
        ], Julia.eval("(0, 1, 2, 3, 4)").valueOf());
    });

    it("Array", function() {
        assert.deepEqual({
            dims: [5],
            data: new Float32Array([0, 1, 2, 3, 4])
        }, Julia.eval("[0.0f0, 1.0f0, 2.0f0, 3.0f0, 4.0f0]").valueOf());
        assert.deepEqual({
            dims: [
                2, 2
            ],
            data: new Float32Array([0, 1, 2, 3])
        }, Julia.eval("[[0.0f0, 1.0f0] [2.0f0, 3.0f0]]").valueOf());
    });

    it("Function", function() {
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

    it("Type", function() {
        let T = Julia.eval(`type T
                                bar
                                baz::Int
                                qux::Float64
                            end;
                            function (self::T)(x)
                                12 + x + self.baz
                            end;
                            T`);

        let obj = new T("Hello, world.", 23, 1.5);
        assert(obj instanceof T);
        assert.deepStrictEqual({
            bar: "Hello, world.",
            baz: 23,
            qux: 1.5
        }, obj.valueOf());
        assert.deepEqual([
            "bar", "baz", "qux"
        ], Object.keys(obj));
        assert.strictEqual("Hello, world.", obj.bar);
        assert.strictEqual(23, obj.baz);
        assert.strictEqual(1.5, obj.qux);
        assert.strictEqual(39, obj(4));

        let f = Julia.eval(`(self::T, x) -> self(x)`);
        assert.strictEqual(39, f(obj, 4));
    });

    it("Module", function() {
        let Test = Julia.eval("Test");
        assert.deepEqual([
            "@inferred",
            "@test",
            "@test_approx_eq",
            "@test_approx_eq_eps",
            "@test_broken",
            "@test_skip",
            "@test_throws",
            "@testset",
            "GenericString",
            "detect_ambiguities"
        ], Object.keys(Test).sort());
    });
});

describe("JavaScript", function() {
    it("Boolean", function() {
        assert.strictEqual(true, Julia.eval("js\"true\""));
        assert.strictEqual(false, Julia.eval("js\"false\""));
    });

    it("Number", function() {
        assert.strictEqual(2, Julia.eval("js\"2\""));
    });

    it("String", function() {
        assert.strictEqual("Hello, world!", Julia.eval("js\"'Hello, world!'\""));
    });

    it("Null", function() {
        assert.strictEqual(null, Julia.eval("js\"null\""));
    });

    it("Object", function() {
        assert.deepEqual({
            x: 1,
            y: 2
        }, Julia.eval("js\"var z = {x: 1, y: 2}; z\"").valueOf());
    });

    it("Array", function() {
        assert.deepEqual({
            dims: [5],
            data: new Float32Array([0, 1, 2, 3, 4])
        }, Julia.convert("Array", {
            dims: [5],
            data: new Float32Array([0, 1, 2, 3, 4])
        }).valueOf());

        assert.deepEqual({
            dims: [
                2, 2
            ],
            data: new Uint8Array([0, 1, 2, 3])
        }, Julia.convert("Array", {
            dims: [
                2, 2
            ],
            data: new Uint8Array([0, 1, 2, 3])
        }).valueOf());
    });
});
