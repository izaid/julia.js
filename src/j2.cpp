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
  static std::map<intptr_t, const char *> types{
      {(intptr_t) jl_float32_type, "Float32Array"}, {(intptr_t) jl_float64_type, "Float64Array"}};

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
                        [](char *d, void *value) {
                          // ...
                          //                          printf("DEALLOCATING\n");
                        },
                        value)
          .ToLocalChecked();
  res->Set(
      v8::String::NewFromUtf8(isolate, "data"),
      NewTypedArray(isolate,
                    types[reinterpret_cast<intptr_t>(jl_array_eltype(value))],
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

void JuliaCall(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::External> external = info.Data().As<v8::External>();
  jl_value_t *value = static_cast<jl_value_t *>(external->Value());

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (int i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, u));
}

void JuliaCall2(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *object =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (int i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(object, jl_svec_data(args), jl_svec_len(args));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, u));
}

v8::Local<v8::Value> j2::FromJuliaFunction(v8::Isolate *isolate,
                                           jl_value_t *value) {
  return v8::Function::New(isolate, JuliaCall,
                           v8::External::New(isolate, value));
}

void JuliaConstruct(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::External> external = info.Data().As<v8::External>();
  jl_value_t *value = static_cast<jl_value_t *>(external->Value());

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (int i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));
  //  printf("value = %i\n", jl_unbox_bool(u));

  //  info.This() = j2::FromJuliaValue(isolate, u).As<v8::Object>();

  info.This()->SetInternalField(0, v8::External::New(isolate, u));
}

void ImportGet(v8::Local<v8::Name> name,
               const v8::PropertyCallbackInfo<v8::Value> &info) {
  printf("getting...\n");
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *object =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  v8::String::Utf8Value s(name);
  if (s.length() != 0) {
    jl_value_t *value = jl_get_field(object, *s);
    if (value != nullptr) {
      v8::ReturnValue<v8::Value> res = info.GetReturnValue();
      res.Set(j2::FromJuliaValue(isolate, value));
    }
  }
}

void ImportEnumerator(const v8::PropertyCallbackInfo<v8::Array> &info) {
  printf("enumerating...\n");
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *value =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  jl_datatype_t *type = (jl_datatype_t *)jl_typeof(value);

  size_t length = jl_field_count(type);
  printf("field_count = %i\n", length);

  v8::Local<v8::Array> properties = v8::Array::New(info.GetIsolate(), length);
  for (int i = 0; i < length; ++i) {
    jl_sym_t *name = jl_field_name(type, i);
    properties->Set(
        v8::Number::New(isolate, i),
        v8::String::NewFromUtf8(
            isolate, jl_symbol_name(reinterpret_cast<jl_sym_t *>(name))));
  }

  info.GetReturnValue().Set(properties);
}

v8::Local<v8::Value> j2::FromJuliaType(v8::Isolate *isolate,
                                       jl_value_t *value) {
  if (reinterpret_cast<jl_datatype_t *>(value) == jl_bool_type) {
    return isolate->GetCurrentContext()->Global()->Get(
        v8::String::NewFromUtf8(isolate, "Boolean"));
  }

  v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(
      isolate, JuliaConstruct, v8::External::New(isolate, value));
  constructor->SetClassName(v8::String::NewFromUtf8(
      isolate,
      jl_symbol_name(reinterpret_cast<jl_datatype_t *>(value)->name->name)));

  v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();
  instance->SetInternalFieldCount(1);
  instance->SetCallAsFunctionHandler(JuliaCall2);

  v8::NamedPropertyHandlerConfiguration handler;
  handler.getter = ImportGet;
  handler.enumerator = ImportEnumerator;
  instance->SetHandler(handler);

  return constructor->GetFunction();
}

void ModuleGet(v8::Local<v8::Name> name,
               const v8::PropertyCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *module =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  v8::String::Utf8Value s(name);
  if (s.length() != 0) {
    //    JL_GC_PUSH1(&module);

    jl_value_t *value = jl_get_global((jl_module_t *)module, jl_symbol(*s));
    if (value != nullptr) {
      //    JL_GC_PUSH1(&value);

      v8::ReturnValue<v8::Value> res = info.GetReturnValue();
      res.Set(j2::FromJuliaValue(isolate, value));

      //      JL_GC_POP();
    }

    //    JL_GC_POP();
  }
}

JL_DLLEXPORT jl_value_t *jl_module_names(jl_module_t *m, int all,
                                         int imported) {
  jl_array_t *a = jl_alloc_array_1d(jl_array_symbol_type, 0);
  JL_GC_PUSH1(&a);
  size_t i;
  void **table = m->bindings.table;
  for (i = 1; i < m->bindings.size; i += 2) {
    if (table[i] != HT_NOTFOUND) {
      jl_binding_t *b = (jl_binding_t *)table[i];
      int hidden = jl_symbol_name(b->name)[0] == '#';
      if ((b->exportp || (imported && b->imported) ||
           ((b->owner == m) && (all || m == jl_main_module))) &&
          (all || (!b->deprecated && !hidden))) {
        jl_array_grow_end(a, 1);
        // XXX: change to jl_arrayset if array storage allocation for
        // Array{Symbols,1} changes:
        jl_array_ptr_set(a, jl_array_dim0(a) - 1, (jl_value_t *)b->name);
      }
    }
  }
  JL_GC_POP();
  return (jl_value_t *)a;
}

void ModuleEnumerator(const v8::PropertyCallbackInfo<v8::Array> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *module =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  jl_array_t *names =
      (jl_array_t *)jl_module_names((jl_module_t *)module, 0, 0);
  size_t length = jl_array_len(names);

  v8::Local<v8::Array> properties = v8::Array::New(info.GetIsolate(), length);
  for (size_t i = 0; i < length; ++i) {
    jl_value_t *v = jl_array_ptr_ref(names, i);
    if (jl_symbol_name(reinterpret_cast<jl_sym_t *>(v)) !=
        jl_symbol_name(((jl_module_t *)module)->name)) {
      properties->Set(
          v8::Number::New(isolate, i),
          v8::String::NewFromUtf8(
              isolate, jl_symbol_name(reinterpret_cast<jl_sym_t *>(v))));
    }
  }

  info.GetReturnValue().Set(properties);
}

v8::Local<v8::Value> j2::FromJuliaModule(v8::Isolate *isolate,
                                         jl_value_t *value) {
  v8::Local<v8::ObjectTemplate> instance = v8::ObjectTemplate::New(isolate);
  instance->SetInternalFieldCount(1);

  v8::NamedPropertyHandlerConfiguration handler;
  handler.getter = ModuleGet;
  handler.enumerator = ModuleEnumerator;
  instance->SetHandler(handler);

  v8::Local<v8::Object> module =
      instance->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
  module->SetInternalField(0, v8::External::New(isolate, value));

  return module;
}

v8::Local<v8::Value> j2::FromJuliaValue(v8::Isolate *isolate,
                                        jl_value_t *value) {
  if (value == nullptr) {
    return v8::Undefined(isolate);
  }

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

  if (jl_is_datatype(value)) {
    return FromJuliaType(isolate, value);
  }

  if (jl_is_module(value)) {
    return FromJuliaModule(isolate, value);
  }

  return v8::Undefined(isolate);
}