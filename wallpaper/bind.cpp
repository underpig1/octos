#include <node.h>
#include <node_buffer.h>
#include <windows.h>
#include <string>
#include "./main.h"

using node::AddEnvironmentCleanupHook;
using v8::Array;
using v8::Boolean;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

unsigned char *windowHandleBuffer;

void AttachExport(const FunctionCallbackInfo<Value> &args)
{
    windowHandleBuffer = (unsigned char *)node::Buffer::Data(args[0]);
    Attach(windowHandleBuffer);
}

void DetachExport(const FunctionCallbackInfo<Value> &args)
{
    Detach(windowHandleBuffer);
}

void MousePosExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    POINT pos = MousePosition();
    v8::Local<v8::Array> array = v8::Array::New(isolate, 2);
    array->Set(context, 0, v8::Integer::New(isolate, (int)pos.x));
    array->Set(context, 1, v8::Integer::New(isolate, (int)pos.y));
    args.GetReturnValue().Set(array);
}

void LMousePressedExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    bool pressed = LMousePressed();
    args.GetReturnValue().Set(pressed);
}

void InForegroundExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    bool pressed = InForeground();
    args.GetReturnValue().Set(pressed);
}

void SetTaskbarExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    SetTaskbar(args[0]->BooleanValue(isolate));
}

void MMousePressedExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    bool pressed = MMousePressed();
    args.GetReturnValue().Set(pressed);
}

void RMousePressedExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    bool pressed = RMousePressed();
    args.GetReturnValue().Set(pressed);
}

void KeyboardExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    bool *keys = KeyboardState();
    v8::Local<v8::Array> array = v8::Array::New(isolate, 256);
    for (int i = 0; i <= 255; i++) array->Set(context, i, v8::Boolean::New(isolate, keys[i]));
    args.GetReturnValue().Set(array);
}

void SendMediaExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Maybe<uint32_t> maybe_uint = args[0]->Uint32Value(context);
    uint32_t i = maybe_uint.FromJust();
    SendMediaEvent((int)i);
}

void ForegroundExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    char* title = ForegroundWindow();
    v8::Local<v8::String> string = v8::String::NewFromUtf8(isolate, title, v8::NewStringType::kNormal).ToLocalChecked();
    args.GetReturnValue().Set(string);
}

void TrackTitleExport(const FunctionCallbackInfo<Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    std::string title = TrackTitle();
    v8::Local<v8::String> string = v8::String::NewFromUtf8(isolate, title.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    args.GetReturnValue().Set(string);
}

static void Cleanup(void *)
{
    SetTaskbar(true);
    Detach(windowHandleBuffer);
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "attach", AttachExport);
    NODE_SET_METHOD(exports, "detach", DetachExport);
    NODE_SET_METHOD(exports, "mposition", MousePosExport);
    NODE_SET_METHOD(exports, "lmpressed", LMousePressedExport);
    NODE_SET_METHOD(exports, "mmpressed", MMousePressedExport);
    NODE_SET_METHOD(exports, "rmpressed", RMousePressedExport);
    NODE_SET_METHOD(exports, "infg", InForegroundExport);
    NODE_SET_METHOD(exports, "settb", SetTaskbarExport);
    NODE_SET_METHOD(exports, "keyboard", KeyboardExport);
    NODE_SET_METHOD(exports, "sendmedia", SendMediaExport);
    NODE_SET_METHOD(exports, "title", ForegroundExport);
    NODE_SET_METHOD(exports, "ttitle", TrackTitleExport);
}

NODE_MODULE_INIT()
{
    Isolate *isolate = context->GetIsolate();
    AddEnvironmentCleanupHook(isolate, Cleanup, nullptr);
    Initialize(exports);
}