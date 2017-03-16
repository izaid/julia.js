#include <sstream>

#include <node.h>

#include <j2.h>

static void Eval(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> code = info[0]->ToObject();
  v8::String::Utf8Value s(code);
  if (s.length() == 0) {
    // ... name is not a string
  }

  jl_value_t *value = jl_eval_string(*s);

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, value, true));
}

static void Require(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> code = info[0]->ToObject();
  v8::String::Utf8Value s(code);
  if (s.length() == 0) {
    // ... name is not a string
  }

  jl_function_t *require = jl_get_function(jl_base_module, "require");
  jl_call1(require, (jl_value_t *)jl_symbol(*s));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaModule(isolate, jl_eval_string(*s)));
}

extern "C" jl_value_t *JSEval(const char *src) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  v8::Local<v8::Script> script =
      v8::Script::Compile(isolate->GetCurrentContext(),
                          v8::String::NewFromUtf8(isolate, src))
          .ToLocalChecked();

  return j2::FromJavaScriptValue2(
      isolate, script->Run(isolate->GetCurrentContext()).ToLocalChecked());
}

// julia -e "println(joinpath(dirname(JULIA_HOME), \"share\", \"julia\",
// \"julia-config.jl\"))" for OS X

char *read_bytes(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = static_cast<char *>(malloc(length));
  if (buffer == NULL) {
    return NULL;
  }

  fread(buffer, 1, length, file);

  fclose(file);

  return buffer;
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  v8::Isolate *isolate = module->GetIsolate();

  NODE_SET_METHOD(exports, "eval", Eval);
  NODE_SET_METHOD(exports, "require", Require);

  jl_init_with_image(JULIA_INIT_DIR, JULIA_INIT_DIR "/julia/sys.dylib");

  v8::String::Utf8Value filename(
      module->Get(v8::String::NewFromUtf8(isolate, "filename")));
  if (filename.length() == 0) {
    // ... name is not a string
  }

  char src_filename[PATH_MAX];
  strncpy(src_filename, *filename, filename.length() - strlen("julia.node"));
  src_filename[filename.length() - strlen("julia.node")] = '\0';
  strcat(src_filename, "js.jl");

  const char *buffer = read_bytes(src_filename);
  if (buffer == NULL) {
    static const char *message_format = "could not open \"%s\"";

    char message[strlen(message_format) + strlen(src_filename)];
    sprintf(message, message_format, src_filename);

    isolate->ThrowException(
        v8::Exception::Error(v8::String::NewFromUtf8(isolate, message)));
  } else {
    jl_eval_string(buffer);
    j2::TranslateJuliaException(isolate);
  }
}

NODE_MODULE(julia, Init)
