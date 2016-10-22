#include "heatline.h"
#include "nan.h"
#include "v8-profiler.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace heatline {

using namespace v8;

Nan::Persistent<v8::Function> CpuProfileWrap::constructor_;
Nan::Persistent<v8::Function> CpuProfileNodeWrap::constructor_;


CpuProfilerWrap::CpuProfilerWrap(Isolate* isolate) {
  profiler_ = isolate->GetCpuProfiler();
}


CpuProfilerWrap::~CpuProfilerWrap() {
  // TODO(indutny): Dispose `profiler_` on newer node.js versions
  profiler_ = nullptr;
}


NAN_METHOD(CpuProfilerWrap::New) {
  Nan::HandleScope();

  CpuProfilerWrap* p = new CpuProfilerWrap(info.GetIsolate());
  p->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}


NAN_METHOD(CpuProfilerWrap::SetSamplingInterval) {
  Nan::HandleScope();

  if (info.Length() < 1 || !info[0]->IsNumber())
    return Nan::ThrowError("setSamplingInterval(index)");

  CpuProfilerWrap* p = ObjectWrap::Unwrap<CpuProfilerWrap>(info.Holder());

  p->profiler_->SetSamplingInterval(
      Nan::To<Uint32>(info[0]).ToLocalChecked()->Value());
}


NAN_METHOD(CpuProfilerWrap::Start) {
  Nan::HandleScope();

  CpuProfilerWrap* p = ObjectWrap::Unwrap<CpuProfilerWrap>(info.Holder());

  p->profiler_->StartProfiling(Nan::New<String>("heatline").ToLocalChecked(),
                               true);
}


NAN_METHOD(CpuProfilerWrap::Stop) {
  Nan::HandleScope();

  CpuProfilerWrap* p = ObjectWrap::Unwrap<CpuProfilerWrap>(info.Holder());

  CpuProfile* res =
      p->profiler_->StopProfiling(Nan::New<String>("heatline").ToLocalChecked());
  CpuProfileWrap* wrap = new CpuProfileWrap(res);

  Local<Function> cons = Nan::New(CpuProfileWrap::constructor_);
  Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();

  wrap->Wrap(instance);

  info.GetReturnValue().Set(instance);
}


void CpuProfilerWrap::Init(Local<Object> target) {
  Nan::HandleScope();

  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  t->SetClassName(Nan::New("CpuProfiler").ToLocalChecked());
  t->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(t, "setSamplingInterval", SetSamplingInterval);
  Nan::SetPrototypeMethod(t, "start", Start);
  Nan::SetPrototypeMethod(t, "stop", Stop);

  Nan::Set(target, Nan::New("CpuProfiler").ToLocalChecked(),
           Nan::GetFunction(t).ToLocalChecked());
}


NAN_METHOD(CpuProfileWrap::New) {
  // No-op, instantiated only in in C++
}


NAN_METHOD(CpuProfileWrap::GetTop) {
  Nan::HandleScope();

  CpuProfileWrap* p = ObjectWrap::Unwrap<CpuProfileWrap>(info.Holder());
  info.GetReturnValue().Set(p->GetTop());
}


Local<Value> CpuProfileWrap::GetTop() {
  const CpuProfileNode* node = profile_->GetTopDownRoot();

  CpuProfileNodeWrap* wrap = new CpuProfileNodeWrap(node);

  Local<Function> cons = Nan::New(CpuProfileNodeWrap::constructor_);
  Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();

  wrap->Wrap(instance);

  return instance;
}


void CpuProfileWrap::Init(Local<Object> target) {
  Nan::HandleScope();

  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  t->SetClassName(Nan::New("CpuProfile").ToLocalChecked());
  t->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(t, "getTop", GetTop);

  constructor_.Reset(Nan::GetFunction(t).ToLocalChecked());
}


NAN_METHOD(CpuProfileNodeWrap::New) {
  // No-op, instantiated only in in C++
}


template <CpuProfileNodeWrap::What what>
NAN_METHOD(CpuProfileNodeWrap::Get) {
  CpuProfileNodeWrap* node = ObjectWrap::Unwrap<CpuProfileNodeWrap>(
      info.Holder());

  info.GetReturnValue().Set(node->Get<what>());
}


template <CpuProfileNodeWrap::What what>
Local<Value> CpuProfileNodeWrap::Get() {
  Nan::HandleScope();

  switch (what) {
    case What::kName:
      return node_->GetFunctionName();
    case What::kResourceName:
      return node_->GetScriptResourceName();
    case What::kLine:
      return Nan::New<Number>(node_->GetLineNumber());
    case What::kColumn:
      return Nan::New<Number>(node_->GetColumnNumber());
    case What::kChildren:
      {
        Local<Array> res = Nan::New<Array>();

        Local<Function> cons = Nan::New(constructor_);

        int count = node_->GetChildrenCount();
        for (int i = 0; i < count; i++) {
          CpuProfileNodeWrap* wrap = new CpuProfileNodeWrap(node_->GetChild(i));
          Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();
          wrap->Wrap(instance);

          res->Set(i, instance);
        }

        return res;
      }
    case What::kHitCount:
      return Nan::New<Number>(node_->GetHitCount());
    case What::kHitLines:
      {
        Local<Array> res = Nan::New<Array>();
        int count = node_->GetHitLineCount();

        CpuProfileNode::LineTick* entries = new CpuProfileNode::LineTick[count];
        node_->GetLineTicks(entries, count);

        for (int i = 0; i < count; i++) {
          res->Set(i * 2, Nan::New<Number>(entries[i].line));
          res->Set(i * 2 + 1, Nan::New<Number>(entries[i].hit_count));
        }
        delete[] entries;

        return res;
      }
  }
}


void CpuProfileNodeWrap::Init(Local<Object> target) {
  Nan::HandleScope();

  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  t->SetClassName(Nan::New("CpuProfileNode").ToLocalChecked());
  t->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(t, "getName", Get<What::kName>);
  Nan::SetPrototypeMethod(t, "getResourceName", Get<What::kResourceName>);
  Nan::SetPrototypeMethod(t, "getLine", Get<What::kLine>);
  Nan::SetPrototypeMethod(t, "getColumn", Get<What::kColumn>);
  Nan::SetPrototypeMethod(t, "getChildren", Get<What::kChildren>);
  Nan::SetPrototypeMethod(t, "getHitCount", Get<What::kHitCount>);
  Nan::SetPrototypeMethod(t, "getHitLines", Get<What::kHitLines>);

  constructor_.Reset(Nan::GetFunction(t).ToLocalChecked());
}


static NAN_MODULE_INIT(Init) {
  CpuProfilerWrap::Init(target);
  CpuProfileWrap::Init(target);
  CpuProfileNodeWrap::Init(target);
}

} // namespace heatline

NODE_MODULE(heatline, heatline::Init);
