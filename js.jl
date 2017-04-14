macro NODE_FILE()
    path = joinpath(dirname(@__FILE__), "julia.node")
    return :($path)
end

function safe_print(s)
    ccall(:jl_,Void,(Any,), s)
end

type JavaScriptValue
    val::Ptr{Void}
end

store = ObjectIdDict()

function mypush(val)
    setindex!(store, val, val)
end

function mypop(val)
    pop!(store, val)
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

function catch_message(e)
    sprint(showerror, e, catch_backtrace())
end
