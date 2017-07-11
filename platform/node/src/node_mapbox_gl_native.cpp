#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <node.h>
#include <nan.h>
#pragma GCC diagnostic pop

#include <mbgl/util/run_loop.hpp>

#include "node_map.hpp"
#include "node_logging.hpp"

void RegisterModule(v8::Local<v8::Object> target, v8::Local<v8::Object> module) {
    // This has the effect of:
    //   a) Ensuring that the static local variable is initialized before any thread contention.
    //   b) unreffing an async handle, which otherwise would keep the default loop running.
    static mbgl::util::RunLoop nodeRunLoop;
    nodeRunLoop.stop();

    node_mbgl::NodeMap::Init(target);

    // Make the exported object inherit from EventEmitter
    v8::Local<v8::Function> require = Nan::Get(module,
        Nan::New("require").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

    v8::Local<v8::Value> eventsString = Nan::New("events").ToLocalChecked();
    v8::Local<v8::Object> events = Nan::To<v8::Object>(Nan::Call(require, module, 1, &eventsString).ToLocalChecked()).ToLocalChecked();

    v8::Local<v8::Object> EventEmitter = Nan::To<v8::Object>(
        Nan::Get(
            events,
            Nan::New("EventEmitter").ToLocalChecked()
        ).ToLocalChecked()
    ).ToLocalChecked();

    Nan::SetPrototype(target,
        Nan::Get(EventEmitter, Nan::New("prototype").ToLocalChecked()).ToLocalChecked());
    Nan::CallAsFunction(EventEmitter, target, 0, nullptr);

    mbgl::Log::setObserver(std::make_unique<node_mbgl::NodeLogObserver>(target));
}

NODE_MODULE(mapbox_gl_native, RegisterModule)
