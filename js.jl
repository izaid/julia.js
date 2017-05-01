module JavaScript
#    macro NODE_FILE()
#        path = joinpath(dirname(@__FILE__), "julia.node")
#        return :($path)
#    end

#    type JavaScriptValue
#        val::Ptr{Void}
#    end

    const SHARED = Dict()

    function catch_message(e)
        "dummy message"
#        sprint(showerror, e, catch_backtrace())
    end

#    function convert(::Type{Array}, x::JavaScriptValue)
#        ccall((:ToJuliaArray, @NODE_FILE), Any, (Any,), x)
#    end
end

#function js(src)
#    ccall((:JSEval, @JavaScript.NODE_FILE), Any, (Cstring,), src)
#end

#macro js_str(src)
#    js(src)
#end
