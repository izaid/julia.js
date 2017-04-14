#include <sstream>

#include <node.h>

#include <j2.h>

// julia -e "println(joinpath(dirname(JULIA_HOME), \"share\", \"julia\",
// \"julia-config.jl\"))" for OS X

using namespace j2;

#include <dlfcn.h>


void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  dlopen(JULIA_INIT_DIR "/libjulia.so", RTLD_GLOBAL);

  v8::Isolate *isolate = module->GetIsolate();

  NODE_SET_METHOD(exports, "eval", j2::Eval);
  NODE_SET_METHOD(exports, "require", j2::Require);

  printf("path = %s\n", JULIA_INIT_DIR "/julia/sys.so");
  jl_init_with_image(JULIA_INIT_DIR, JULIA_INIT_DIR "/julia/sys.so");

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
