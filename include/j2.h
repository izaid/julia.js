#include <julia.h>
#include <v8.h>

extern "C" void jl_v8_init(const char *path);
extern "C" void jl_v8_destroy();

extern "C" jl_value_t *jl_v8_eval(const char *src);

namespace j2 {

void Init(v8::Isolate *isolate);

v8::Local<v8::Value> FromJuliaArray(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaBool(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaFloat64(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaString(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaNothing(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaValue(v8::Isolate *isolate, jl_value_t *value);

jl_value_t *FromJavaScriptArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptBoolean(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNull(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNumber(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptString(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptTypedArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptValue(v8::Local<v8::Value> value);

} // namespace j2
