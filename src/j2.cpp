#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>

#include <node.h>
#include <node_buffer.h>

#include <j2.h>

static void ValueOfCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  static jl_value_t *getindex = jl_get_function(jl_main_module, "getindex");
  assert(getindex != nullptr);

  static jl_value_t *shared = jl_get_function(j2::js_module, "SHARED");
  assert(shared != nullptr);

  v8::Isolate *isolate = info.GetIsolate();

  uintptr_t id = reinterpret_cast<uintptr_t>(
      info.This()->GetInternalField(0).As<v8::External>()->Value());
  jl_value_t *value = jl_call2(getindex, shared, jl_box_uint64(id));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, value, true));
}

static void ImportGet(v8::Local<v8::Name> name,
                      const v8::PropertyCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);

  uintptr_t id =
      reinterpret_cast<uintptr_t>(wrapper.As<v8::External>()->Value());
  jl_value_t *object = j2::GetJuliaValue(id);

  v8::String::Utf8Value s(name);
  if (s.length() != 0) {
    jl_value_t *value = jl_get_field(object, *s);
    if (value != nullptr) {
      v8::ReturnValue<v8::Value> res = info.GetReturnValue();
      res.Set(j2::FromJuliaValue(isolate, value));
    }
  }
}

static void ImportEnumerator(const v8::PropertyCallbackInfo<v8::Array> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);

  uintptr_t id =
      reinterpret_cast<uintptr_t>(wrapper.As<v8::External>()->Value());
  jl_value_t *value = j2::GetJuliaValue(id);

  jl_datatype_t *type = (jl_datatype_t *)jl_typeof(value);

  size_t length = jl_field_count(type);

  v8::Local<v8::Array> properties = v8::Array::New(info.GetIsolate(), length);
  for (size_t i = 0; i < length; ++i) {
    jl_sym_t *name = jl_field_name(type, i);
    properties->Set(
        v8::Number::New(isolate, i),
        v8::String::NewFromUtf8(
            isolate, jl_symbol_name(reinterpret_cast<jl_sym_t *>(name))));
  }

  info.GetReturnValue().Set(properties);
}

static void JuliaConstruct(const v8::FunctionCallbackInfo<v8::Value> &info) {
  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::External> external = info.Data().As<v8::External>();
  jl_value_t *value = static_cast<jl_value_t *>(external->Value());

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (size_t i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));
  if (j2::TranslateJuliaException(isolate)) {
    return;
  }
  //  printf("value = %i\n", jl_unbox_bool(u));

  //  info.This() = j2::FromJuliaValue(isolate, u).As<v8::Object>();

  //  info.This() = j2::NewPersistent<v8::Value>(isolate, u).As<v8::Object>();

  info.This()->SetInternalField(
      0, v8::External::New(isolate, reinterpret_cast<void *>(jl_object_id(u))));
  j2::NewPersistent<v8::Value>(isolate, u);
}

void JuliaCall2(const v8::FunctionCallbackInfo<v8::Value> &info) {
  static jl_value_t *getindex = jl_get_function(jl_main_module, "getindex");
  assert(getindex != nullptr);

  static jl_value_t *shared = jl_get_function(j2::js_module, "SHARED");
  assert(shared != nullptr);

  v8::Isolate *isolate = info.GetIsolate();

  v8::Local<v8::Value> wrapper = info.This()->GetInternalField(0);
  uintptr_t id =
      reinterpret_cast<uintptr_t>(wrapper.As<v8::External>()->Value());

  jl_value_t *object = jl_call2(getindex, shared, jl_box_uint64(id));

  jl_svec_t *args = jl_alloc_svec(info.Length());
  for (size_t i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(object, jl_svec_data(args), jl_svec_len(args));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, u));
}

namespace j2 {

template <>
v8::Local<v8::Value> New<v8::Value>(v8::Isolate *isolate, jl_value_t *value) {
  v8::Local<v8::FunctionTemplate> t =
      NewPersistent<v8::FunctionTemplate>(isolate, jl_typeof(value));

  v8::Local<v8::Object> res = t->InstanceTemplate()->NewInstance();
  res->SetInternalField(
      0, v8::External::New(isolate,
                           reinterpret_cast<void *>(jl_object_id(value))));

  return res;
}

template <>
v8::Local<v8::FunctionTemplate> New<v8::FunctionTemplate>(v8::Isolate *isolate,
                                                          jl_value_t *value) {
  JL_GC_PUSH1(&value);

  v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(
      isolate, JuliaConstruct, v8::External::New(isolate, value));
  constructor->SetClassName(v8::String::NewFromUtf8(isolate, "JuliaValue"));

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

  JL_GC_POP();
  return constructor;
}

} // namespace j2

template <typename T>
v8::Local<T> j2::NewPersistent(v8::Isolate *isolate, jl_value_t *value) {
  static std::map<uintptr_t, v8::UniquePersistent<T>> PersistentValues;

  // ...
  JL_GC_PUSH1(&value);

  uintptr_t id = jl_object_id(value);

  auto it = PersistentValues.find(id);
  if (it != PersistentValues.end()) {
    JL_GC_POP();
    return it->second.Get(isolate);
  }

  v8::Local<T> res = New<T>(isolate, value);

  auto p = PersistentValues.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(id),
                                    std::forward_as_tuple(isolate, res));
  if (!p.second) {
    printf("COLLISION!\n");
  }

  auto jt = p.first;
  jt->second.SetWeak(reinterpret_cast<void *>(id),
                     [](const v8::WeakCallbackInfo<void> &data) -> void {
                       uintptr_t id =
                           reinterpret_cast<uintptr_t>(data.GetParameter());
                       PopJuliaValue(data.GetIsolate(), id);
                       PersistentValues.erase(id);
                     },
                     v8::WeakCallbackType::kParameter);

  PushJuliaValue(isolate, id, value);

  JL_GC_POP();
  return res;
}

size_t j2::SizeOfJuliaValue(jl_value_t *value) {
  static jl_value_t *size = jl_get_function(jl_main_module, "sizeof");
  assert(size != nullptr);

  if (jl_is_datatype(value)) {
    value = jl_typeof(value);
  }

  return jl_unbox_int64(jl_call1(size, value));
}

jl_value_t *j2::GetJuliaValue(uintptr_t id) {
  static jl_value_t *getindex = jl_get_function(jl_main_module, "getindex");
  assert(getindex != nullptr);

  static jl_value_t *shared = jl_get_function(js_module, "SHARED");
  assert(shared != nullptr);

  jl_value_t *value = jl_call2(getindex, shared, jl_box_uint64(id));

  return value;
}

void j2::PushJuliaValue(v8::Isolate *isolate, uintptr_t id, jl_value_t *value) {
  static jl_value_t *setindex = jl_get_function(jl_main_module, "setindex!");
  assert(setindex != nullptr);

  static jl_value_t *shared = jl_get_function(js_module, "SHARED");
  assert(shared != nullptr);

  JL_GC_PUSH1(&value);

  jl_call3(setindex, shared, value, jl_box_uint64(id));
  isolate->AdjustAmountOfExternalAllocatedMemory(SizeOfJuliaValue(value));

  jl_value_t *e = jl_exception_occurred();
  if (e != nullptr) {
    isolate->TerminateExecution();
  }

  JL_GC_POP();
}

void j2::PopJuliaValue(v8::Isolate *isolate, uintptr_t id) {
  static jl_value_t *pop = jl_get_function(jl_main_module, "pop!");
  assert(pop != nullptr);

  static jl_value_t *shared = jl_get_function(js_module, "SHARED");
  assert(shared != nullptr);

  jl_value_t *value = jl_call2(pop, shared, jl_box_uint64(id));
  isolate->AdjustAmountOfExternalAllocatedMemory(SizeOfJuliaValue(value));

  jl_value_t *e = jl_exception_occurred();
  if (e != nullptr) {
    isolate->TerminateExecution();
  }
}

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
  return constructor
      ->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 3, args)
      .ToLocalChecked();
}

bool j2::TranslateJuliaException(v8::Isolate *isolate) {
  static jl_module_t *js_module = reinterpret_cast<jl_module_t *>(
      jl_get_global(jl_main_module, jl_symbol("JavaScript")));
  assert(js_module != nullptr);

  static jl_value_t *catch_message =
      jl_get_function(js_module, "catch_message");
  assert(catch_message != nullptr);

  jl_value_t *e = jl_exception_occurred();
  if (e == nullptr) {
    return false;
  }

  jl_value_t *s;
  {
    JL_GC_PUSH1(&catch_message);
    s = jl_call1(catch_message, e);
    JL_GC_POP();
  }

  JL_GC_PUSH1(&s);

  isolate->ThrowException(v8::Exception::Error(
      v8::String::NewFromUtf8(isolate, jl_string_data(s))));

  JL_GC_POP();

  return true;
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

/*
jl_value_t *UnboxJuliaArrayType(v8::Isolate *isolate,
                                v8::Local<v8::Object> value) {
  jl_datatype_t *eltype = UnboxJuliaArrayElementType(isolate, value);
  JL_GC_PUSH1(&eltype);

  jl_value_t *res = jl_apply_array_type(eltype, 1);
  JL_GC_POP();

  return res;
}
*/

/*
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
*/

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
  return jl_box_float64(
      value->ToNumber(v8::Isolate::GetCurrent()->GetCurrentContext())
          .ToLocalChecked()
          ->Value());
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

jl_value_t *UnboxJuliaValue(v8::Isolate *isolate,
                            v8::Local<v8::Value> js_value) {
  v8::Local<v8::Value> js_external =
      js_value.As<v8::Object>()->GetInternalField(0);

  return j2::GetJuliaValue(
      reinterpret_cast<uintptr_t>(js_external.As<v8::External>()->Value()));
}

jl_value_t *j2::FromJavaScriptValue(v8::Isolate *isolate,
                                    v8::Local<v8::Value> value) {
  static jl_value_t *js_value_type = jl_get_function(js_module, "Value");
  assert(js_value_type != nullptr);

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

  if (value->IsObject()) {
    v8::Local<v8::String> js_name =
        value.As<v8::Object>()->GetConstructorName();
    v8::String::Utf8Value s(js_name);

    if (strcmp(*s, "JuliaValue") == 0) {
      return UnboxJuliaValue(isolate, value);
    }
  }

  // if (value->IsObject()) {
  //    return FromJavaScriptObject(isolate, value);
  //  }

  //  if (value->IsArray()) {
  //  return FromJavaScriptArray(value);
  //  }

  //  if (value->IsObject()) {
  //    return FromJavaScriptObject(isolate, value);
  //}

  jl_value_t *v = jl_call1(
      js_value_type,
      jl_box_voidpointer(new v8::Persistent<v8::Value>(isolate, value)));
  if (TranslateJuliaException(isolate)) {
    return jl_nothing;
  }

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
  static const std::map<jl_datatype_t *, const char *> types{
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
  for (size_t i = 0; i < jl_svec_len(args); ++i) {
    jl_value_t *value = j2::FromJavaScriptValue(isolate, info[i]);

    jl_svec_data(args)[i] = value;
  }

  jl_value_t *u = jl_call(value, jl_svec_data(args), jl_svec_len(args));

  v8::ReturnValue<v8::Value> res = info.GetReturnValue();
  res.Set(j2::FromJuliaValue(isolate, u));
}

v8::Local<v8::Value> j2::FromJuliaFunction(v8::Isolate *isolate,
                                           jl_value_t *value) {
  return v8::Function::New(isolate, JuliaCall,
                           v8::External::New(isolate, value));
}

v8::Local<v8::Value> j2::FromJuliaType(v8::Isolate *isolate,
                                       jl_value_t *value) {
  return NewPersistent<v8::FunctionTemplate>(isolate, value)->GetFunction();
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

v8::Local<v8::Value> j2::FromJuliaJavaScriptValue(v8::Isolate *isolate,
                                                  jl_value_t *value) {
  return v8::Null(isolate);
}

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

jl_module_t *j2::js_module;

v8::Local<v8::Value> j2::FromJuliaValue(v8::Isolate *isolate, jl_value_t *value,
                                        bool cast) {
  static jl_value_t *js_value_type = jl_get_function(js_module, "Value");
  assert(js_value_type != nullptr);

  JL_GC_PUSH1(&value);

  if (jl_is_bool(value)) {
    JL_GC_POP();

    return FromJuliaBool(isolate, value);
  }

  if (jl_is_int64(value)) {
    JL_GC_POP();

    return FromJuliaInt64(isolate, value);
  }

  if (jl_is_float64(value)) {
    JL_GC_POP();

    return FromJuliaFloat64(isolate, value);
  }

  if (jl_is_string(value)) {
    JL_GC_POP();

    return FromJuliaString(isolate, value);
  }

  if (jl_is_nothing(value)) {
    JL_GC_POP();

    return FromJuliaNothing(isolate, value);
  }

  if (jl_subtype(value, reinterpret_cast<jl_value_t *>(jl_function_type), 1)) {
    JL_GC_POP();

    return FromJuliaFunction(isolate, value);
  }

  if (jl_is_datatype(value)) {
    JL_GC_POP();

    return FromJuliaType(isolate, value);
  }

  if (jl_is_module(value)) {
    JL_GC_POP();
    return FromJuliaModule(isolate, value);
  }

  if (jl_subtype(value, js_value_type, 1)) {
    JL_GC_POP();
    v8::Persistent<v8::Value> *val =
        (v8::Persistent<v8::Value> *)jl_unbox_voidpointer(
            jl_get_nth_field(value, 0));
    return val->Get(isolate);
  }

  if (cast) {
    if (jl_is_int32(value)) {
      JL_GC_POP();

      return FromJuliaInt32(isolate, value);
    }

    if (jl_is_float32(value)) {
      JL_GC_POP();

      return FromJuliaFloat32(isolate, value);
    }

    if (jl_is_tuple(value)) {
      JL_GC_POP();

      return FromJuliaTuple(isolate, value);
    }

    if (jl_is_array(value)) {
      JL_GC_POP();

      return FromJuliaArray(isolate, value);
    }

    jl_value_t *type = jl_typeof(value);
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    for (size_t i = 0; i < jl_field_count(type); ++i) {
      obj->Set(v8::String::NewFromUtf8(isolate,
                                       jl_symbol_name(jl_field_name(type, i))),
               FromJuliaValue(isolate, jl_get_nth_field(value, i), true));
    }

    JL_GC_POP();

    return obj;
  }

  JL_GC_POP();
  return NewPersistent<v8::Value>(isolate, value);
}

v8::Local<v8::Value> UnboxJavaScriptValue(v8::Isolate *isolate,
                                          jl_value_t *value) {
  jl_value_t *ptr = jl_get_nth_field(value, 0);

  v8::Persistent<v8::Value> &persistent =
      *static_cast<v8::Persistent<v8::Value> *>(jl_unbox_voidpointer(ptr));
  return persistent.Get(isolate);
}

jl_value_t *ToJuliaArray(jl_value_t *jl_value) {
  static const std::map<std::string, jl_datatype_t *> jl_eltypes{
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

void j2::Eval(const v8::FunctionCallbackInfo<v8::Value> &info) {
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

void j2::Require(const v8::FunctionCallbackInfo<v8::Value> &info) {
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

  return j2::FromJavaScriptValue(
      isolate, script->Run(isolate->GetCurrentContext()).ToLocalChecked());
}
