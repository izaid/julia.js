module JavaScript
    macro __NODE_FILE__()
        path = joinpath(dirname(@__FILE__), "julia.node")
        return :($path)
    end

    type Value
        ptr::Ptr{Void}

        function Value(ptr)
            res = new(ptr)
            finalizer(res, (x::Value) -> ccall((:j2_delete_persistent_value, @__NODE_FILE__), Void, (Ptr{Void},), x.ptr))
            res
        end
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
