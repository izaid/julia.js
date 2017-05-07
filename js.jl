module JavaScript
    macro __NODE_FILE__()
        path = joinpath(dirname(@__FILE__), "julia.node")
        return :($path)
    end

    type Value
        ptr::Ptr{Void}

        function Value()
            res = new()
            finalizer(res, finalize_value)
            res
        end
    end

    # bitstype 8 ValueBits

    function finalize_value(x::Value)
        ccall((:j2_destroy_value, @__NODE_FILE__), Void, (Any,), x)
    end

    const SHARED = Dict()

    function catch_message(e)
        sprint(showerror, e, catch_backtrace())
    end

    function convert(::Type{Array}, x::Value)
        ccall((:j2_to_julia_array, @__NODE_FILE__), Any, (Any,), x)
    end
end

function js(src)
    ccall((:JSEval, @JavaScript.__NODE_FILE__), Any, (Cstring,), src)
end

macro js_str(src)
    js(src)
end
