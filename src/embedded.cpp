#include <sstream>

#include <node.h>

#include <j2.h>

// julia -e "println(joinpath(dirname(JULIA_HOME), \"share\", \"julia\",
// \"julia-config.jl\"))" for OS X

using namespace j2;

void Convert(const v8::FunctionCallbackInfo<v8::Value> &info) {
  static jl_value_t *convert = jl_get_function(js_module, "convert");
  assert(convert != nullptr);

  v8::Isolate *isolate = info.GetIsolate();

  v8::String::Utf8Value s(info[0]);
  if (*s == NULL) {
    printf("NULL\n");
    isolate->ThrowException(
        v8::Exception::Error(v8::String::NewFromUtf8(isolate, "Error")));
    return;
  }

  jl_value_t *type = jl_eval_string(*s);
  if (TranslateJuliaException(isolate)) {
    return;
  }

  jl_value_t *u = jl_call2(convert, type, FromJavaScriptValue(isolate, info[1]));
  if (TranslateJuliaException(isolate)) {
    return;
  }

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(FromJuliaValue(isolate, u));
}

void Eval(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::String::Utf8Value s(info[0]);
  if (*s == NULL) {
    printf("NULL\n");
    isolate->ThrowException(
        v8::Exception::Error(v8::String::NewFromUtf8(isolate, "Error")));
    return;
  }

  jl_value_t *value = jl_eval_string(*s);
  if (TranslateJuliaException(isolate)) {
    return;
  }

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(FromJuliaValue(isolate, value));
}

void Require(const v8::FunctionCallbackInfo<v8::Value> &info) {
  static jl_function_t *require = jl_get_function(jl_base_module, "require");
  assert(require != nullptr);

  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> code = info[0]->ToObject();
  v8::String::Utf8Value s(code);
  if (s.length() == 0) {
    // ... name is not a string
  }

  jl_call1(require, (jl_value_t *)jl_symbol(*s));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(FromJuliaModule(isolate, jl_eval_string(*s)));
}

extern "C" jl_value_t *JSEval(const char *src) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  v8::Local<v8::Script> script =
      v8::Script::Compile(isolate->GetCurrentContext(),
                          v8::String::NewFromUtf8(isolate, src))
          .ToLocalChecked();

  return j2::FromJavaScriptValue(
      isolate, script->Run(isolate->GetCurrentContext()).ToLocalChecked());
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  v8::Isolate *isolate = module->GetIsolate();

  NODE_SET_METHOD(exports, "convert", Convert);
  NODE_SET_METHOD(exports, "eval", Eval);
  NODE_SET_METHOD(exports, "require", Require);

  jl_init(JULIA_INIT_DIR);

  v8::String::Utf8Value filename(
      module->Get(v8::String::NewFromUtf8(isolate, "filename")));
  if (filename.length() == 0) {
    // ... name is not a string
  }

  char src_filename[PATH_MAX];
  strncpy(src_filename, *filename, filename.length() - strlen("julia.node"));
  src_filename[filename.length() - strlen("julia.node")] = '\0';
  strcat(src_filename, "js.jl");

  jl_value_t *include = jl_get_function(jl_main_module, "include");
  if (include == NULL) {
    printf("NULL INCLUDE!\n");
  }

  jl_call1(include, jl_cstr_to_string(src_filename));
  j2::TranslateJuliaException(isolate);

  js_module = reinterpret_cast<jl_module_t *>(
      jl_get_global(jl_main_module, jl_symbol("JavaScript")));
  TranslateJuliaException(isolate);
}

NODE_MODULE(julia, Init)
