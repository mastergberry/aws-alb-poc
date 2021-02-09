import helloworld.HelloWorldData;
import helloworld.HelloWorldServiceGrpc;
import io.grpc.Server;
import io.grpc.netty.NettyServerBuilder;
import io.grpc.stub.StreamObserver;

import java.util.concurrent.TimeUnit;

public class TestServer {
	public static void main(String[] args) throws Exception {
		Server server = NettyServerBuilder.forPort(9876) // Replace this port with whatever port you want to test on
			.addService(new HelloWorldServiceGrpc.HelloWorldServiceImplBase() {
				@Override
				public StreamObserver<HelloWorldData> helloWorldStream(StreamObserver<HelloWorldData> responseObserver) {
					System.out.println("Stream opened");

					return new StreamObserver<HelloWorldData>() {
						@Override
						public void onNext(HelloWorldData request) {
							System.out.println("Received data: " + request.getMessage().toStringUtf8());
						}

						@Override
						public void onError(Throwable throwable) {
							System.out.println("Stream Error: " + throwable.getMessage());
						}

						@Override
						public void onCompleted() {
							System.out.println("Stream completed");
						}
					};
				}
			})
			.keepAliveTime(25, TimeUnit.SECONDS)
			.keepAliveTimeout(10, TimeUnit.SECONDS)
			.permitKeepAliveWithoutCalls(true)
			.permitKeepAliveTime(20, TimeUnit.SECONDS)
			.build()
			.start();

		Runtime.getRuntime().addShutdownHook(new Thread(server::shutdown));

		while (!Thread.currentThread().isInterrupted()) {
			try {
				Thread.sleep(1000000L);
			} catch (InterruptedException ignored) {

			}
		}
	}
}
