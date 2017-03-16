type JavaScriptValue
    val::Ptr{Void}
end

macro NODE_FILE()
    path = joinpath(dirname(@__FILE__), "julia.node")
    println(path)
    println(pwd())
    return :($path)
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
