Proof of Concept showing that AWS ALB does not properly forward/handling HTTP2 keep alive packets to keep the connection open with gRPC

Client & server are both included in respective directories. Setup server on a EC2 instance, create AWS ALB w/ gRPC enabled, and then fix the {REPLACE_ME} in the C++ code (found in HelloWorldManager.cpp) with the AWS ALB network address. Compile, & run client...and wait for a minute to watch the AWS ALB drop the socket connection ever 60 seconds even though keep alives are there and should keep the connection open.
