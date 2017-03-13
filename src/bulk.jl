R"(
function say_hello()
    println("Hello, world!")
end

type JavaScriptValue
    val::Ptr{Void}
end

function say_hello_callback()
    ccall((:callback, "/Users/irwin/git/julia.js/build/Release/julia.node"), Void, ())
end

function convert(::Type{Any}, x::JavaScriptValue)
    return 12
end

function js(src)
    ccall((:JSEval, "/Users/irwin/git/julia.js/build/Release/julia.node"), Any, (Cstring,), src)
end

macro js_str(src)
    js(src)
end
)"
