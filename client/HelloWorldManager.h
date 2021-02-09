#ifndef CLIENT_HELLOWORLDMANAGER_H
#define CLIENT_HELLOWORLDMANAGER_H

#include "helloworld.grpc.pb.h"

using namespace grpc;
using namespace helloworld;
using namespace std;

class HelloWorldManager
{
public:
    HelloWorldManager();
    ~HelloWorldManager();

    void wait();
private:
    shared_ptr<Channel> m_channel;
    unique_ptr<thread> m_thread;
    unique_ptr<HelloWorldService::Stub> m_stub;

    bool m_alive = true;
    ClientContext* m_context;

    void StreamWorker();

    // Avoid having to install roots.pem on the system itself, just hardcode it for this example

};


#endif //CLIENT_HELLOWORLDMANAGER_H
