#include <julia.h>
#include <v8.h>

namespace j2 {

extern jl_module_t *js_module;

/**
 * This is
 */

size_t SizeOfJuliaValue(jl_value_t *value);

/**
 *
 */
bool TranslateJuliaException(v8::Isolate *isolate);

v8::Local<v8::FunctionTemplate> TemplateFromJuliaType(v8::Isolate *isolate,
                                                      jl_value_t *value);

template <typename T> v8::Local<T> New(v8::Isolate *isolate, jl_value_t *value);
template <typename T>
v8::Local<T> NewPersistent(v8::Isolate *isolate, jl_value_t *value);

void PushJuliaValue(v8::Isolate *isolate, uintptr_t id, jl_value_t *value);
void PopJuliaValue(v8::Isolate *isolate, uintptr_t id);

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
v8::Local<v8::Value> FromJuliaTuple(v8::Isolate *isolate, jl_value_t *value);
v8::Local<v8::Value> FromJuliaValue(v8::Isolate *isolate, jl_value_t *value,
                                    bool cast = false);

jl_value_t *FromJavaScriptArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptBoolean(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNull(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptNumber(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptString(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptTypedArray(v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptObject(v8::Isolate *isolate,
                                 v8::Local<v8::Value> value);
jl_value_t *FromJavaScriptValue(v8::Isolate *isolate,
                                v8::Local<v8::Value> value);

void Eval(const v8::FunctionCallbackInfo<v8::Value> &info);
void Require(const v8::FunctionCallbackInfo<v8::Value> &info);

} // namespace j2
