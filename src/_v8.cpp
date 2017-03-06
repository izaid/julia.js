#include <libplatform/libplatform.h>

#include <j2.h>

#include <nan.h>

static v8::Platform *m_platform;
static v8::Isolate *m_isolate;

void jl_v8_init(const char *path) {
  printf("Initing v8\n");

  class Allocator : public v8::ArrayBuffer::Allocator {
  public:
    void *Allocate(size_t length) { return new char[length]; }

    void *AllocateUninitialized(size_t length) { return new char[length]; }

    virtual void Free(void *data, size_t length) {
      delete[] static_cast<char *>(data);
    }
  };

  v8::V8::InitializeICU(path);
  v8::V8::InitializeExternalStartupData(path);

  m_platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(m_platform);
  v8::V8::Initialize();

  v8::V8::InitializeExternalStartupData(path);

  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = new Allocator();
  m_isolate = v8::Isolate::New(create_params);
}

void jl_v8_destroy() {
  m_isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete m_platform;
}

jl_value_t *jl_v8_eval(const char *src) {
  Nan::HandleScope scope;

  Nan::MaybeLocal<Nan::BoundScript> script =
      Nan::CompileScript(v8::String::NewFromUtf8(m_isolate, src));
  if (script.IsEmpty()) {
    printf("BAD SCRIPT\n");
  }

  Nan::MaybeLocal<v8::Value> value = Nan::RunScript(script.ToLocalChecked());
  if (value.IsEmpty()) {
    printf("BAD VALUE\n");
  }

  return j2::FromJavaScriptValue(value.ToLocalChecked());

  /*
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context =  v8::Context::New(NULL); // =
    v8::Context::New(m_isolate);
    v8::Context::Scope context_scope(context);

    v8::Local<v8::Script> script =
        v8::Script::Compile(context, v8::String::NewFromUtf8(m_isolate, src))
            .ToLocalChecked();

  */
}
