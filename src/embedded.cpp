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

#include <sstream>

void Import(v8::Local<v8::Name> name,
            const v8::PropertyCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::String::Utf8Value s(name);
  if (s.length() != 0) {
    std::stringstream ss;
    ss << "import " << *s;

    jl_eval_string(ss.str().c_str());

    v8::ReturnValue<v8::Value> res = info.GetReturnValue();
    res.Set(j2::FromJuliaModule(isolate, jl_eval_string(*s)));
  }
}

void Init(v8::Local<v8::Object> exports) {
  v8::Isolate *isolate = exports->GetIsolate();
  j2::Init(isolate);

  exports->Set(
      v8::String::NewFromUtf8(exports->GetIsolate(), "ArrayDescriptor"),
      j2::array_descriptor.Get(isolate)->GetFunction());
  NODE_SET_METHOD(exports, "eval", Eval);

  v8::Local<v8::ObjectTemplate> import_template =
      v8::ObjectTemplate::New(isolate);

  v8::NamedPropertyHandlerConfiguration handler;
  handler.getter = Import;
  import_template->SetHandler(handler);

  exports->Set(v8::String::NewFromUtf8(exports->GetIsolate(), "$"),
               import_template->NewInstance(isolate->GetCurrentContext())
                   .ToLocalChecked());

  jl_init_with_image(
      "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia",
      "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia/"
      "sys.dylib");
}

NODE_MODULE(julia, Init)
