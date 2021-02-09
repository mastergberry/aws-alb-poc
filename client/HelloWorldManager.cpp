#include "HelloWorldManager.h"

#include <grpc++/grpc++.h>

HelloWorldManager::HelloWorldManager()
{
    // KeepAlive arguments to trigger HTTP2 pings and keep connection to server open
    ChannelArguments channelArguments;
    channelArguments.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 25000);
    channelArguments.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    channelArguments.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    channelArguments.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 25000);
    channelArguments.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
    channelArguments.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);

    // Replace this string with the target of the AWS ALB you are testing
    this->m_channel = CreateCustomChannel("{REPLACE_ME}", InsecureChannelCredentials(), channelArguments);

    if (this->m_channel != nullptr)
    {
        cout << "Starting worker thread" << endl;
        this->m_thread = make_unique<std::thread>(&HelloWorldManager::StreamWorker, this);
    }
    else
        cerr << "Failed to create channel" << endl;
}

HelloWorldManager::~HelloWorldManager()
{
    // Don't let the thread continue anymore
    this->m_alive = false;

    // Try to kill GRPC thread
    if (this->m_context != nullptr)
        this->m_context->TryCancel();

    // Join thread
    if (this->m_thread->joinable())
        this->m_thread->join();
}

void HelloWorldManager::wait()
{
    // Just chill here and show that the worker thread keeps disconnecting from AWS ALB
    while (this->m_alive)
        this_thread::sleep_for(chrono::milliseconds(100));
}

void HelloWorldManager::StreamWorker()
{
    HelloWorldData request, response;
    Status status;

    // Infinite loop to reconnect
    while (this->m_alive)
    {
        cout << "Connecting to AWS ALB for updates to be polled..." << endl;

        this->m_context = new (nothrow) ClientContext();
        if (this->m_context == nullptr)
        {
            cerr << "Something went wrong when creating client context" << endl;
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }

        // Create double stream
        shared_ptr<ClientReaderWriter<HelloWorldData, HelloWorldData>> stream(this->m_stub->HelloWorldStream(this->m_context));

        bool writeForever = true;

        // Send off our single request
        thread writer([stream, &request, &writeForever]() {
            // Force the data to flush
            grpc::WriteOptions writeOptions;
            writeOptions.set_write_through();

            if (!stream->Write(request, writeOptions))
                cerr << "Error writing data to stream" << endl;

            // Infinite loop because we never want to return in theory to keep the stream "open"
            while (writeForever)
                this_thread::sleep_for(chrono::milliseconds(100));
        });

        // Get our responses
        while (stream->Read(&response))
            cout << "Received data: " << response.message() << endl;

        cerr << "Error: Disconnected from remote server. Should not happen unless network was interrupted (or AWS ALB said bye bye)" << endl;

        // Tell writer to die now
        writeForever = false;
        writer.join();

        // Get the error from the stream
        status = stream->Finish();

        // Check we got a good response
        if (!status.ok())
            cerr << "GRPC Error " + to_string(status.error_code()) << " - " << status.error_message() << endl;

        // Clean memory
        delete this->m_context;
    }
}
