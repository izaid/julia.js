module JavaScript
    type Value
        val::Ptr{Void}
    end

    macro NODE_FILE()
        path = joinpath(dirname(@__FILE__), "julia.node")
        return :($path)
    end

    const SHARED = Dict()

    function catch_message(e)
        sprint(showerror, e, catch_backtrace())
    end

    function convert(::Type{Array}, x::Value)
        ccall((:ToJuliaArray, @NODE_FILE), Any, (Any,), x)
    end
end

function js(src)
    ccall((:JSEval, @JavaScript.NODE_FILE), Any, (Cstring,), src)
end

macro js_str(src)
    js(src)
end
