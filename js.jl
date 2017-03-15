type JavaScriptValue
    val::Ptr{Void}
end

function js(src)
    ccall((:JSEval, "julia.node"), Any, (Cstring,), src)
end

macro js_str(src)
    js(src)
end
