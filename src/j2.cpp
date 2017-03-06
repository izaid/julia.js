#include <j2.h>

#include <node.h>
#include <node_buffer.h>

v8::Local<v8::Object> NewTypedArray(v8::Isolate *isolate, const char *name,
                                    v8::Local<v8::Value> buffer,
                                    size_t byte_offset, size_t length) {
  v8::Local<v8::Function> constructor =
      isolate->GetCurrentContext()
          ->Global()
          ->Get(v8::String::NewFromUtf8(isolate, name))
          .As<v8::Function>();

  v8::Local<v8::Value> args[3] = {buffer,
                                  v8::Integer::New(isolate, byte_offset),
                                  v8::Integer::New(isolate, length)};
  return constructor->NewInstance(3, args);
}

static void
ArrayDescriptorConstructor(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  info.This()->Set(v8::String::NewFromUtf8(isolate, "dims"), info[0]);
  info.This()->Set(v8::String::NewFromUtf8(isolate, "data"), info[1]);
}

#include <map>

static v8::Local<v8::Object> NewArrayDescriptor(v8::Isolate *isolate,
                                                jl_value_t *value) {
  static std::map<jl_datatype_t *, const char *> types{
      {jl_float32_type, "Float32Array"}};

  v8::Local<v8::ObjectTemplate> instance =
      j2::array_descriptor.Get(isolate)->InstanceTemplate();
  v8::Local<v8::Object> res = instance->NewInstance();

  int32_t ndims = jl_array_ndims(value);
  v8::Local<v8::Array> dims = v8::Array::New(isolate, ndims);
  for (int32_t i = 0; i < ndims; ++i) {
    dims->Set(i, v8::Number::New(isolate, jl_array_size(value, i)));
  }
  res->Set(v8::String::NewFromUtf8(isolate, "dims"), dims);

  v8::Local<v8::Object> buffer =
      node::Buffer::New(isolate, static_cast<char *>(jl_array_data(value)),
                        jl_array_len(value) *
                            reinterpret_cast<jl_array_t *>(value)->elsize,
                        [](char *d, void *value) {}, value)
          .ToLocalChecked();
  res->Set(
      v8::String::NewFromUtf8(isolate, "data"),
      NewTypedArray(isolate,
                    types[static_cast<jl_datatype_t *>(jl_array_eltype(value))],
                    buffer->Get(v8::String::NewFromUtf8(isolate, "buffer")), 0,
                    jl_array_len(value)));

  return res;
}

v8::Persistent<v8::FunctionTemplate> j2::array_descriptor;

void j2::Init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> f =
      v8::FunctionTemplate::New(isolate, ArrayDescriptorConstructor);
  f->SetClassName(v8::String::NewFromUtf8(isolate, "ArrayDescriptor"));

  f->InstanceTemplate()->Set(v8::String::NewFromUtf8(isolate, "dims"),
                             v8::Null(isolate));

  array_descriptor.Reset(isolate, f);
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

v8::Local<v8::Value> j2::FromJuliaInt32(v8::Isolate *isolate,
                                        jl_value_t *value) {
  return v8::Number::New(isolate, jl_unbox_int32(value));
}

v8::Local<v8::Value> j2::FromJuliaInt64(v8::Isolate *isolate,
                                        jl_value_t *value) {
  return v8::Number::New(isolate, jl_unbox_int64(value));
}

v8::Local<v8::Value> j2::FromJuliaFloat32(v8::Isolate *isolate,
                                          jl_value_t *value) {
  return v8::Number::New(isolate, jl_unbox_float32(value));
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

v8::Local<v8::Value> j2::FromJuliaArray(v8::Isolate *isolate,
                                        jl_value_t *value) {
  return NewArrayDescriptor(isolate, value);
}

/*
void JuliaCall(const v8::FunctionCallbackInfo<v8::Value> &info) {
  printf("JuliaCall (start)\n");

  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::External> external = info.Data().As<v8::External>();
  jl_value_t *value = static_cast<jl_value_t *>(external->Value());

  // ...
  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (int i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = UnboxJuliaValue(isolate, info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u =
      jl_call((jl_function_t *)value, jl_svec_data(args), jl_svec_len(args));
  if (TranslateJuliaException(isolate) != 0) {
    return;
  }

  // SetCallAsFunctionHandler
  // call(::Type{A},x,y,z)

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(NewFromJuliaValue(isolate, u));

  printf("JuliaCall (stop)\n");
}
*/

v8::Local<v8::Value> j2::FromJuliaFunction(v8::Isolate *isolate,
                                           jl_value_t *value) {
  return v8::Function::New(
      isolate,
      [](const v8::FunctionCallbackInfo<v8::Value> &info) {
        v8::Isolate *isolate = info.GetIsolate();

        v8::Local<v8::External> external = info.Data().As<v8::External>();
        jl_value_t *value = static_cast<jl_value_t *>(external->Value());

        jl_svec_t *args = jl_alloc_svec(info.Length());
        for (int i = 0; i < jl_svec_len(args); ++i) {
          jl_value_t *value = FromJavaScriptValue(info[i]);

          jl_svec_data(args)[i] = value;
        }

        jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));

        v8::ReturnValue<v8::Value> res = info.GetReturnValue();
        res.Set(FromJuliaValue(isolate, u));
      },
      v8::External::New(isolate, value));
}

v8::Local<v8::Value> j2::FromJuliaValue(v8::Isolate *isolate,
                                        jl_value_t *value) {
  if (jl_is_bool(value)) {
    return FromJuliaBool(isolate, value);
  }

  if (jl_is_int32(value)) {
    return FromJuliaInt32(isolate, value);
  }

  if (jl_is_int64(value)) {
    return FromJuliaInt64(isolate, value);
  }

  if (jl_is_float32(value)) {
    return FromJuliaFloat32(isolate, value);
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

  if (jl_is_array(value)) {
    return FromJuliaArray(isolate, value);
  }

  if (jl_subtype(value, reinterpret_cast<jl_value_t *>(jl_function_type),
                 true)) {
    return FromJuliaFunction(isolate, value);
  }

  return v8::Undefined(isolate);
}
