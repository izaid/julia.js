#include <julia.h>

#include <v8.h>

typedef jl_value_t *(*from_javascript_t)(v8::Isolate *isolate,
                                         v8::Local<v8::Value> value);

namespace j2 {

extern v8::Persistent<v8::FunctionTemplate> array_descriptor;

void Inject(v8::Local<v8::Object> exports);

v8::Local<v8::Value> FromJuliaArray(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaBool(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaInt32(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaInt64(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaFloat32(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaFloat64(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaFunction(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaString(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaModule(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaNothing(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaType(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaValue(v8::Isolate *isolate, jl_value_t *value);

jl_value_t *FromJavaScriptArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptJuliaArrayDescriptor(v8::Isolate *isolate,
                                               v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptJuliaConvert(v8::Isolate *isolate,
                                       v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptBoolean(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNull(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNumber(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptString(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptTypedArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptObject(v8::Isolate *isolate,
                                 v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptValue(v8::Isolate *isolate,
                                v8::Local<v8::Value> value);

} // namespace j2
