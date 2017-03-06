#include <libplatform/libplatform.h>

#include <j2.h>

// class Array {
// int dims[2];
//};

static v8::Platform *m_platform;
static v8::Isolate *m_isolate;

void jl_v8_init(const char *path) {
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
  v8::Isolate::Scope isolate_scope(m_isolate);
  v8::HandleScope handle_scope(m_isolate);
  v8::Local<v8::Context> context(v8::Context::New(m_isolate));
  v8::Context::Scope context_scope(context);

  v8::Local<v8::Script> script =
      v8::Script::Compile(context, v8::String::NewFromUtf8(m_isolate, src))
          .ToLocalChecked();

  return j2::FromJavaScriptValue(script->Run(context).ToLocalChecked());
}

jl_value_t *j2::FromJavaScriptArray(v8::Local<v8::Value> value) {
  return jl_nothing;
}

jl_value_t *j2::FromJavaScriptBoolean(v8::Local<v8::Value> value) {
  return jl_box_bool(value->ToBoolean()->Value());
}

jl_value_t *j2::FromJavaScriptString(v8::Local<v8::Value> value) {
  v8::String::Utf8Value s(value);
  return jl_pchar_to_string(*s, s.length());
}

jl_value_t *j2::FromJavaScriptNull(v8::Local<v8::Value> value) {
  return jl_nothing;
}

jl_value_t *j2::FromJavaScriptNumber(v8::Local<v8::Value> value) {
  return jl_box_float64(value->ToNumber()->Value());
}

jl_value_t *j2::FromJavaScriptValue(v8::Local<v8::Value> value) {
  if (value->IsBoolean()) {
    return FromJavaScriptBoolean(value);
  }

  if (value->IsNumber()) {
    return FromJavaScriptNumber(value);
  }

  if (value->IsString()) {
    return FromJavaScriptString(value);
  }

  if (value->IsNull()) {
    return FromJavaScriptNull(value);
  }

  if (value->IsArray()) {
    return FromJavaScriptArray(value);
  }

  return jl_nothing;
}

v8::Local<v8::Value> j2::FromJuliaBool(v8::Isolate *isolate,
                                       jl_value_t *value) {
  return v8::Boolean::New(isolate, jl_unbox_bool(value));
}

v8::Local<v8::Value> j2::FromJuliaFloat64(v8::Isolate *isolate,
                                          jl_value_t *value) {
  return v8::Number::New(isolate, jl_unbox_float64(value));
}

v8::Local<v8::Value> j2::FromJuliaNothing(v8::Isolate *isolate,
                                          jl_value_t *value) {
  return v8::Null(isolate);
}

v8::Local<v8::Value> j2::FromJuliaString(v8::Isolate *isolate,
                                         jl_value_t *value) {
  return v8::String::NewFromUtf8(isolate, jl_string_data(value));
}

v8::Local<v8::Value> j2::FromJuliaValue(v8::Isolate *isolate,
                                        jl_value_t *value) {
  if (jl_is_bool(value)) {
    return FromJuliaBool(isolate, value);
  }

  if (jl_is_float64(value)) {
    return FromJuliaFloat64(isolate, value);
  }

  if (jl_is_nothing(value)) {
    return FromJuliaNothing(isolate, value);
  }

  if (jl_is_string(value)) {
    return FromJuliaString(isolate, value);
  }

  return v8::Undefined(isolate);
}
