var assert = require('assert');

var Julia = require('../julia');

describe("Julia", () => {
    it("Exception", () => {
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

    it("Bool", () => {
        assert.strictEqual(true, Julia.eval("true"));
        assert.strictEqual(false, Julia.eval("false"));
    });

    it("Int32", () => {
        assert.strictEqual(0, Julia.eval("Int32(0)").valueOf());
        assert.strictEqual(1, Julia.eval("Int32(1)").valueOf());
    });

    it("Int64", () => {
        assert.strictEqual(0, Julia.eval("Int64(0)"));
        assert.strictEqual(1, Julia.eval("Int64(1)"));
    });

/*
    it("Float32", () => {
        assert.strictEqual(0.0, Julia.eval("0.0f0").valueOf());
        assert.strictEqual(0.5, Julia.eval("0.5f0").valueOf());
    });
*/

    it("Float64", () => {
        assert.strictEqual(0.0, Julia.eval("0.0"));
        assert.strictEqual(0.5, Julia.eval("0.5"));
    });

/*
    it("Complex64", () => {
        assert.deepStrictEqual({
            re: 0.0,
            im: 1.0
        }, Julia.eval("1.0f0im").valueOf());
    });

    it("Complex128", () => {
        assert.deepStrictEqual({
            re: 0.0,
            im: 1.0
        }, Julia.eval("1.0im").valueOf());
        assert.deepStrictEqual({
            re: 1.0,
            im: 1.0
        }, Julia.eval("1.0 + 1.0im").valueOf());
    });
*/

    it("GarbageCollection", () => {
        for (let i = 0; i < 250; ++i) {
            Julia.eval("rand(500, 500)");
        }
    }).timeout(10000);

    /*
    it("String", () => {
        assert.strictEqual("Hello, world!", Julia.eval("\"Hello, world!\""));
        assert.notStrictEqual("Hello, world!", Julia.eval("\"Hello, wo\""));
    });

    it("Nothing", () => {
        assert.strictEqual(null, Julia.eval("nothing"));
    });

    it("Tuple", () => {
        assert.deepStrictEqual([], Julia.eval("()").valueOf());

        assert.deepStrictEqual(["Hello, world!"], Julia.eval("(\"Hello, world!\",)").valueOf());

        assert.deepStrictEqual([
            0, 1, 2, 3, 4
        ], Julia.eval("(0, 1, 2, 3, 4)").valueOf());
    });
*/

    /*
    it("Array", () => {
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
    }).timeout(10000);
*/

    /*
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
        assert.strictEqual(Number, Julia.eval("Float64"));

        let Foo = Julia.eval(`type Foo
                                  qux::Bool
                                  count::Int64
                              end;
                              function (self::Foo)(x) 12 + x + self.count end;
                              Foo`);
        console.log(Foo)

        var val = new Foo(true, 1);
        console.log(val.valueOf());
        assert(val instanceof Foo);
        assert.deepStrictEqual({
            qux: true,
            count: 1
        }, val.valueOf());
        assert.deepEqual([
            "qux", "count"
        ], Object.keys(val));
        assert.strictEqual(true, val.qux);
        assert.strictEqual(1, val.count);

        assert.strictEqual(17, val(4));
    });

    it("Module", () => {
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
*/

    /*
  it("JavaScriptNull",
     () => { assert.strictEqual(null, Julia.eval("js\"null\"")); });

  it("JavaScriptNumber",
     () => { assert.strictEqual(2, Julia.eval("js\"2\"")); });

  it("JavaScriptArray", () => {
    var convertArray = Julia.eval("(value) -> convert(Array, value)");
    var size = Julia.eval("size");

    assert.deepEqual({dims : [ 5 ], data : new Float32Array([ 0, 1, 2, 3, 4 ])},
                     convertArray({
                       dims : [ 5 ],
                       data : new Float32Array([ 0, 1, 2, 3, 4 ])
                     }).valueOf());

    assert.deepEqual({dims : [ 2, 2 ], data : new Uint8Array([ 0, 1, 2, 3 ])},
                     convertArray({
                       dims : [ 2, 2 ],
                       data : new Uint8Array([ 0, 1, 2, 3 ])
                     }).valueOf());
  });
*/

    //    it("JavaScriptString", () => {
    //      assert.strictEqual("Hello, world!", Julia.eval("js\"\"Hello,
    //      world!\"\""));
    //  });

    //    it("JavaScriptValue", () => {
    //      assert.deepEqual({
    //        x: 1,
    //      y: 2
    //}, Julia.eval("js\"var z = {x: 1, y: 2}; z\""));

    //        var res = Julia.eval("convert(Any, js\"var z = {x: 1, y: 2}; z\")");
    //      console.log(res);
    //  });
});
