#include <libplatform/libplatform.h>

#include <j2.h>

#include <gtest/gtest.h>

class V8Environment : public ::testing::Environment {
  class Allocator : public v8::ArrayBuffer::Allocator {
  public:
    void *Allocate(size_t length) { return new char[length]; }

    void *AllocateUninitialized(size_t length) { return new char[length]; }

    virtual void Free(void *data, size_t length) {
      delete[] static_cast<char *>(data);
    }
  };

  const char *path;
  v8::Platform *m_platform;
  v8::Isolate *m_isolate;

public:
  V8Environment(const char *path) : path(path) {}

  v8::Isolate *GetIsolate() { return m_isolate; }

  void SetUp() {
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

  void TearDown() {
    // ...

    m_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete m_platform;
    // delete create_params.array_buffer_allocator;
    // return 0;
  }
};

extern V8Environment *V8_ENVIRONMENT;

class JuliaEnvironment : public ::testing::Environment {
  void SetUp() {
    jl_init_with_image(
        "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia",
        "/Applications/Julia-0.5.app/Contents/Resources/julia/lib/julia/"
        "sys.dylib");
  }
};

class TestScope : public ::testing::Test {
  v8::Isolate::Scope isolate_scope;
  v8::HandleScope handle_scope;
  v8::Local<v8::Context> context;
  v8::Context::Scope context_scope;

public:
  TestScope()
      : isolate_scope(V8_ENVIRONMENT->GetIsolate()),
        handle_scope(V8_ENVIRONMENT->GetIsolate()),
        context(v8::Context::New(V8_ENVIRONMENT->GetIsolate())),
        context_scope(context) {}

  v8::Isolate *GetIsolate() { return V8_ENVIRONMENT->GetIsolate(); }

  void SetUp() {}

  v8::Local<v8::Value> JavaScriptEval(const char *src) {
    v8::Isolate *isolate = V8_ENVIRONMENT->GetIsolate();
    // Create a string containing the JavaScript source code.

    v8::Local<v8::Script> script =
        v8::Script::Compile(context, v8::String::NewFromUtf8(isolate, src))
            .ToLocalChecked();
    return script->Run(context).ToLocalChecked();
  }

  jl_value_t *JuliaEval(const char *src) { return jl_eval_string(src); }

  std::string Stringify(v8::Local<v8::Value> value) {
    if (value.IsEmpty()) {
      return std::string();
    }

    v8::Local<v8::Object> json =
        GetIsolate()
            ->GetCurrentContext()
            ->Global()
            ->Get(v8::String::NewFromUtf8(GetIsolate(), "JSON"))
            ->ToObject();
    v8::Local<v8::Function> stringify =
        json->Get(v8::String::NewFromUtf8(GetIsolate(), "stringify"))
            .As<v8::Function>();

    v8::String::Utf8Value str(stringify->Call(json, 1, &value));
    return *str;
  }
};

inline ::testing::AssertionResult AssertJuliaEquals(const char *m_expr,
                                                    const char *n_expr,
                                                    jl_value_t *lhs,
                                                    jl_value_t *rhs) {

  jl_value_t *f = jl_get_function(jl_base_module, "==");
  jl_value_t *res = jl_call2(f, lhs, rhs);

  if (jl_unbox_bool(res)) {
    return ::testing::AssertionSuccess();
  }

  return ::testing::AssertionFailure();
}

inline ::testing::AssertionResult AssertJuliaNotEquals(const char *m_expr,
                                                       const char *n_expr,
                                                       jl_value_t *lhs,
                                                       jl_value_t *rhs) {

  jl_value_t *f = jl_get_function(jl_base_module, "==");
  jl_value_t *res = jl_call2(f, lhs, rhs);

  if (!jl_unbox_bool(res)) {
    return ::testing::AssertionSuccess();
  }

  return ::testing::AssertionFailure();
}

#define EXPECT_JL_STRICT_EQ(A, B) EXPECT_PRED_FORMAT2(AssertJuliaEquals, A, B);

#define EXPECT_JL_STRICT_NE(A, B)                                              \
  EXPECT_PRED_FORMAT2(AssertJuliaNotEquals, A, B);

#define EXPECT_JULIA_JAVASCRIPT_EQ(LHS, RHS)                                   \
  EXPECT_JL_STRICT_EQ(JuliaEval(LHS),                                          \
                      j2::FromJavaScriptValue(JavaScriptEval(RHS)));

#define EXPECT_JULIA_JAVASCRIPT_NE(LHS, RHS)                                   \
  EXPECT_JL_STRICT_NE(JuliaEval(LHS),                                          \
                      j2::FromJavaScriptValue(JavaScriptEval(RHS)));

inline ::testing::AssertionResult
AssertJavaScriptStrictEquals(const char *m_expr, const char *n_expr,
                             v8::Local<v8::Value> lhs,
                             v8::Local<v8::Value> rhs) {
  if (lhs->StrictEquals(rhs)) {
    return ::testing::AssertionSuccess();
  }

  return ::testing::AssertionFailure();
}

inline ::testing::AssertionResult
AssertJavaScriptNotStrictEquals(const char *m_expr, const char *n_expr,
                                v8::Local<v8::Value> lhs,
                                v8::Local<v8::Value> rhs) {
  return !AssertJavaScriptStrictEquals(m_expr, n_expr, lhs, rhs);
}

#define EXPECT_JS_STRICT_EQ(A, B)                                              \
  EXPECT_PRED_FORMAT2(AssertJavaScriptStrictEquals, A, B);

#define EXPECT_JS_STRICT_NE(A, B)                                              \
  EXPECT_PRED_FORMAT2(AssertJavaScriptNotStrictEquals, A, B);
