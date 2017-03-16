#include <map>
#include <string>
#include <unordered_map>

#include <node.h>
#include <node_buffer.h>

#include <j2.h>

namespace j2 {

static std::map<std::string, v8::UniquePersistent<v8::FunctionTemplate>> types;

} // namespace j2

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

int j2::TranslateJuliaException(v8::Isolate *isolate) {
  jl_value_t *exception = jl_exception_occurred();
  if (exception != NULL) {
    JL_GC_PUSH1(&exception);

    const char *message = jl_typeof_str(exception);
    jl_value_t *message2 = jl_get_field(exception, "msg");

    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(isolate, jl_string_data(message2))));

    JL_GC_POP();

    return 1;
  }

  return 0;
}

jl_value_t *j2::FromJavaScriptArray(v8::Local<v8::Value> value) {
  return jl_nothing;
}

jl_value_t *UnboxJuliaArrayDims(v8::Isolate *isolate,
                                v8::Local<v8::Array> value) {
  uint32_t ndims =
      value->Get(v8::String::NewFromUtf8(isolate, "length"))->NumberValue();
  std::vector<uint64_t> dims(ndims);
  for (uint32_t i = 0; i < ndims; ++i) {
    dims[i] = value->Get(i)->NumberValue();
  }

  jl_value_t *type = jl_tupletype_fill(ndims, (jl_value_t *)jl_uint64_type);
  JL_GC_PUSH1(&type);

  jl_value_t *tuple = jl_new_bits(type, dims.data());
  JL_GC_POP();

  return tuple;
}

jl_datatype_t *UnboxJuliaArrayElementType(v8::Isolate *isolate,
                                          v8::Local<v8::Object> value) {
  if (value->IsFloat32Array()) {
    return jl_float32_type;
  }

  return NULL;
}

jl_value_t *UnboxJuliaArrayType(v8::Isolate *isolate,
                                v8::Local<v8::Object> value) {
  jl_datatype_t *eltype = UnboxJuliaArrayElementType(isolate, value);
  JL_GC_PUSH1(&eltype);

  jl_value_t *res = jl_apply_array_type(eltype, 1);
  JL_GC_POP();

  return res;
}

jl_value_t *j2::FromJavaScriptJuliaArrayDescriptor(v8::Isolate *isolate,
                                                   v8::Local<v8::Value> value) {
  v8::Local<v8::Object> data =
      value.As<v8::Object>()
          ->Get(v8::String::NewFromUtf8(isolate, "data"))
          .As<v8::Object>();

  jl_value_t *dims;
  jl_value_t *type;
  JL_GC_PUSH2(&type, &dims);

  dims = UnboxJuliaArrayDims(isolate,
                             value.As<v8::Object>()
                                 ->Get(v8::String::NewFromUtf8(isolate, "dims"))
                                 .As<v8::Array>());
  type = UnboxJuliaArrayType(isolate, data);
  v8::Local<v8::ArrayBuffer> buffer =
      data->Get(v8::String::NewFromUtf8(isolate, "buffer"))
          .As<v8::ArrayBuffer>();

  jl_array_t *res =
      jl_ptr_to_array(type, buffer->GetContents().Data(), dims, 0);
  JL_GC_POP();

  // register a finalizer on res that deletes a copy of the javascript object

  return (jl_value_t *)res;
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

jl_value_t *j2::FromJavaScriptObject(v8::Isolate *isolate,
                                     v8::Local<v8::Value> value) {

  v8::Local<v8::String> x = value.As<v8::Object>()->GetConstructorName();
  v8::String::Utf8Value s(x);
  printf("construct_name = %s\n", *s);

  /*
    if (value.As<v8::Object>()->GetConstructorName()->Equals(
            v8::String::NewFromUtf8(isolate, "Conversion"))) {
      printf("Convert success\n");
      return FromJavaScriptJuliaConvert(isolate, value);
    }
  */

  return jl_nothing;
}

jl_value_t *j2::FromJavaScriptValue(v8::Isolate *isolate,
                                    v8::Local<v8::Value> value) {
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

  //  if (value->IsArray()) {
  //  return FromJavaScriptArray(value);
  //  }

  //  if (value->IsObject()) {
  //    return FromJavaScriptObject(isolate, value);
  //}

  jl_value_t *func = jl_eval_string("JavaScriptValue");
  jl_value_t *v = jl_new_struct((jl_datatype_t *)func);
  TranslateJuliaException(isolate);

  jl_set_nth_field(
      v, 0, jl_box_voidpointer(new v8::Persistent<v8::Value>(isolate, value)));

  return v;
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

v8::Local<v8::Value> j2::FromJuliaTuple(v8::Isolate *isolate,
                                        jl_value_t *value) {
  size_t length = jl_field_count(jl_typeof(value));
  v8::Local<v8::Array> res = v8::Array::New(isolate, length);
  for (size_t i = 0; i < length; ++i) {
    res->Set(i, FromJuliaValue(isolate, jl_get_nth_field(value, i)));
  }

  return res;
}

v8::Local<v8::Value> j2::FromJuliaArray(v8::Isolate *isolate,
                                        jl_value_t *value) {
  static const std::unordered_map<jl_datatype_t *, const char *> types{
      {jl_uint8_type, "Uint8Array"},
      {jl_uint16_type, "Uint16Array"},
      {jl_float32_type, "Float32Array"},
      {jl_float64_type, "Float64Array"}};

  v8::Local<v8::Object> res = v8::Object::New(isolate);

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
  res->Set(v8::String::NewFromUtf8(isolate, "data"),
           NewTypedArray(
               isolate,
               types.at(static_cast<jl_datatype_t *>(jl_array_eltype(value))),
               buffer->Get(v8::String::NewFromUtf8(isolate, "buffer")), 0,
               jl_array_len(value)));

  return res;
}

void JuliaCall(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::External> external = info.Data().As<v8::External>();
  jl_value_t *value = static_cast<jl_value_t *>(external->Value());

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (int i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

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
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

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
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));
  //  printf("value = %i\n", jl_unbox_bool(u));

  //  info.This() = j2::FromJuliaValue(isolate, u).As<v8::Object>();

  info.This()->SetInternalField(0, v8::External::New(isolate, u));
}

void ImportGet(v8::Local<v8::Name> name,
               const v8::PropertyCallbackInfo<v8::Value> &info) {
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
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *value =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  jl_datatype_t *type = (jl_datatype_t *)jl_typeof(value);

  size_t length = jl_field_count(type);

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

  if (reinterpret_cast<jl_datatype_t *>(value) == jl_float64_type) {
    return isolate->GetCurrentContext()->Global()->Get(
        v8::String::NewFromUtf8(isolate, "Number"));
  }

  return NewJavaScriptType(isolate, reinterpret_cast<jl_datatype_t *>(value))
      ->GetFunction();
}

static void ValueOfCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();
  jl_value_t *value = static_cast<jl_value_t *>(
      info.This()->GetInternalField(0).As<v8::External>()->Value());

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, value, false));
}

v8::Local<v8::FunctionTemplate> j2::NewJavaScriptType(v8::Isolate *isolate,
                                                      jl_datatype_t *type) {
  v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(
      isolate, JuliaConstruct, v8::External::New(isolate, type));
  constructor->SetClassName(
      v8::String::NewFromUtf8(isolate, jl_symbol_name(type->name->name)));

  constructor->PrototypeTemplate()->Set(
      v8::String::NewFromUtf8(isolate, "valueOf"),
      v8::FunctionTemplate::New(isolate, ValueOfCallback));

  v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();
  instance->SetInternalFieldCount(1);
  instance->SetCallAsFunctionHandler(JuliaCall2);

  v8::NamedPropertyHandlerConfiguration handler;
  handler.getter = ImportGet;
  handler.enumerator = ImportEnumerator;
  instance->SetHandler(handler);

  return constructor;
}

namespace {

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

void ModuleGetter(v8::Local<v8::Name> name,
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

void ModuleEnumerator(const v8::PropertyCallbackInfo<v8::Array> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  jl_value_t *module =
      static_cast<jl_value_t *>(wrapper.As<v8::External>()->Value());

  jl_array_t *names =
      (jl_array_t *)jl_module_names((jl_module_t *)module, 0, 0);
  size_t length = jl_array_len(names);

  v8::Local<v8::Array> properties = v8::Array::New(isolate, length);
  for (size_t i = 0; i < length; ++i) {
    jl_value_t *v = jl_array_ptr_ref(names, i);
    if (jl_symbol_name(reinterpret_cast<jl_sym_t *>(v)) !=
        jl_symbol_name(reinterpret_cast<jl_module_t *>(module)->name)) {
      properties->Set(
          v8::Number::New(isolate, i),
          v8::String::NewFromUtf8(
              isolate, jl_symbol_name(reinterpret_cast<jl_sym_t *>(v))));
    }
  }

  info.GetReturnValue().Set(properties);
}

} // unnamed namespace

v8::Local<v8::Value> j2::FromJuliaModule(v8::Isolate *isolate,
                                         jl_value_t *value) {
  v8::Local<v8::ObjectTemplate> instance = v8::ObjectTemplate::New(isolate);
  instance->SetInternalFieldCount(1);

  v8::NamedPropertyHandlerConfiguration handler;
  handler.getter = ModuleGetter;
  handler.enumerator = ModuleEnumerator;
  instance->SetHandler(handler);

  v8::Local<v8::Object> module =
      instance->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
  module->SetInternalField(0, v8::External::New(isolate, value));

  return module;
}

v8::Local<v8::Value> j2::FromJuliaValue(v8::Isolate *isolate, jl_value_t *value,
                                        bool exact) {
  if (jl_is_bool(value)) {
    return FromJuliaBool(isolate, value);
  }

  if (jl_is_int64(value)) {
    return FromJuliaInt64(isolate, value);
  }

  if (jl_is_float64(value)) {
    return FromJuliaFloat64(isolate, value);
  }

  if (jl_is_string(value)) {
    return FromJuliaString(isolate, value);
  }

  if (jl_is_nothing(value)) {
    return FromJuliaNothing(isolate, value);
  }

  if (jl_is_datatype(value)) {
    return FromJuliaType(isolate, value);
  }

  if (jl_subtype(value, reinterpret_cast<jl_value_t *>(jl_function_type), 1)) {
    return FromJuliaFunction(isolate, value);
  }

  if (jl_is_module(value)) {
    return FromJuliaModule(isolate, value);
  }

  jl_value_t *datatype = jl_eval_string("JavaScriptValue");
  if (jl_subtype(value, datatype, 1)) {
    v8::Persistent<v8::Value> *val =
        (v8::Persistent<v8::Value> *)jl_unbox_voidpointer(
            jl_get_nth_field(value, 0));
    return val->Get(isolate);
  }

  if (!exact) {
    if (jl_is_int32(value)) {
      return FromJuliaInt32(isolate, value);
    }

    if (jl_is_float32(value)) {
      return FromJuliaFloat32(isolate, value);
    }

    if (jl_is_tuple(value)) {
      return FromJuliaTuple(isolate, value);
    }

    if (jl_is_array(value)) {
      return FromJuliaArray(isolate, value);
    }

    jl_value_t *type = jl_typeof(value);
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    for (size_t i = 0; i < jl_field_count(type); ++i) {
      obj->Set(v8::String::NewFromUtf8(isolate,
                                       jl_symbol_name(jl_field_name(type, i))),
               FromJuliaValue(isolate, jl_get_nth_field(value, i), false));
    }

    return obj;
  }

  v8::Local<v8::FunctionTemplate> t = NewJavaScriptType(
      isolate, reinterpret_cast<jl_datatype_t *>(jl_typeof(value)));

  v8::Local<v8::Object> inst = t->InstanceTemplate()->NewInstance();
  inst->SetInternalField(0, v8::External::New(isolate, value));

  return inst;
}

v8::Local<v8::Value> UnboxJavaScriptValue(v8::Isolate *isolate,
                                          jl_value_t *value) {
  jl_value_t *ptr = jl_get_nth_field(value, 0);

  v8::Persistent<v8::Value> &persistent =
      *static_cast<v8::Persistent<v8::Value> *>(jl_unbox_voidpointer(ptr));
  return persistent.Get(isolate);
}

jl_value_t *ToJuliaArray(jl_value_t *jl_value) {
  static const std::unordered_map<std::string, jl_datatype_t *> jl_eltypes{
      {"Uint8Array", jl_uint8_type},
      {"Uint16Array", jl_uint16_type},
      {"Float32Array", jl_float32_type},
      {"Float64Array", jl_float64_type}};

  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  v8::Local<v8::Value> js_value = UnboxJavaScriptValue(isolate, jl_value);
  v8::Local<v8::Value> js_dims =
      js_value.As<v8::Object>()->Get(v8::String::NewFromUtf8(isolate, "dims"));
  v8::Local<v8::Value> js_data =
      js_value.As<v8::Object>()->Get(v8::String::NewFromUtf8(isolate, "data"));
  v8::Local<v8::ArrayBuffer> js_buffer =
      js_data.As<v8::Object>()
          ->Get(v8::String::NewFromUtf8(isolate, "buffer"))
          .As<v8::ArrayBuffer>();

  v8::Local<v8::String> js_name =
      js_data.As<v8::Object>()->GetConstructorName();
  v8::String::Utf8Value s(js_name);

  uint32_t ndims = js_dims.As<v8::Array>()
                       ->Get(v8::String::NewFromUtf8(isolate, "length"))
                       ->NumberValue();

  std::vector<uint64_t> dims(ndims);
  for (uint32_t i = 0; i < ndims; ++i) {
    dims[i] = js_dims.As<v8::Array>()->Get(i)->NumberValue();
  }

  jl_value_t *jl_type = jl_tupletype_fill(ndims, (jl_value_t *)jl_uint64_type);
  jl_value_t *jl_dims = jl_new_bits(jl_type, dims.data());

  jl_array_t *res =
      jl_ptr_to_array(jl_apply_array_type(jl_eltypes.at(*s), ndims),
                      js_buffer->GetContents().Data(), jl_dims, 0);

  return (jl_value_t *)res;
}
