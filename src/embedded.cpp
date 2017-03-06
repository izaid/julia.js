#include <node.h>

#include <j2.h>

void Eval(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> code = info[0]->ToObject();
  v8::String::Utf8Value s(code);
  if (s.length() == 0) {
    // ... name is not a string
  }

  jl_value_t *value = jl_eval_string(*s);

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, value));
}

void init(v8::Local<v8::Object> exports) {
  jl_init_with_image(
      "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia",
      "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia/"
      "sys.dylib");

  NODE_SET_METHOD(exports, "eval", Eval);
}

NODE_MODULE(julia, init)
