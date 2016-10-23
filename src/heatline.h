#ifndef SRC_HEATLINE_H_
#define SRC_HEATLINE_H_

#include "nan.h"
#include "v8-profiler.h"

namespace heatline {

class CpuProfilerWrap : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> target);

 protected:
  CpuProfilerWrap(v8::Isolate* isolate);
  ~CpuProfilerWrap();

  static NAN_METHOD(New);
  static NAN_METHOD(SetSamplingInterval);
  static NAN_METHOD(Start);
  static NAN_METHOD(Stop);

  v8::CpuProfiler* profiler_;
};

class CpuProfileWrap : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> target);

 protected:
  CpuProfileWrap(v8::CpuProfile* profile) : profile_(profile) {
  }

  ~CpuProfileWrap() {
    profile_->Delete();
    profile_ = nullptr;
  }

  v8::Local<v8::Value> GetTop();

  static NAN_METHOD(New);
  static NAN_METHOD(GetTop);

  v8::CpuProfile* profile_;

  static Nan::Persistent<v8::Function> constructor_;

  friend class CpuProfilerWrap;
};

class CpuProfileNodeWrap : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> target);

 protected:
  CpuProfileNodeWrap(const v8::CpuProfileNode* node) : node_(node) {
  }

  ~CpuProfileNodeWrap() {}

  enum What {
    kName,
    kResourceName,
    kLine,
    kColumn,
    kChildren,
    kHitCount,
    kCallUid,
    kHitLines,
  };

  static NAN_METHOD(New);

  template <What what>
  static NAN_METHOD(Get);

  template <What what>
  v8::Local<v8::Value> Get();

  const v8::CpuProfileNode* node_;

  static Nan::Persistent<v8::Function> constructor_;

  friend class CpuProfileWrap;
};

}  // namespace heatline

#endif  // SRC_HEATLINE_H_
