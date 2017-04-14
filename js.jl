macro NODE_FILE()
    path = joinpath(dirname(@__FILE__), "julia.node")
    return :($path)
end

type JavaScriptValue
    val::Ptr{Void}
end

module JavaScript
    const SHARED = Set()

    function catch_message(e)
        sprint(showerror, e, catch_backtrace())
    end
end

function js(src)
    ccall((:JSEval, @NODE_FILE), Any, (Cstring,), src)
end

macro js_str(src)
    js(src)
end

function convert(::Type{Array}, x::JavaScriptValue)
    ccall((:ToJuliaArray, @NODE_FILE), Any, (Any,), x)
end
