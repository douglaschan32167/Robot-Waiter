import java.io.IOException;
import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;


public class OrderStreamReceiver {

	private static final int ORDER_SIZE = 200; //assume that orders can't get too large.
	private int port;
	private ServerSocket serverSocket;
	private Socket socket;
	
	public OrderStreamReceiver(int port) {
		this.port = port;
		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException ioe) {
			System.err.println("unable to create ServerSocket with port " + port);
			System.exit(1);
		}
	}
	public void receiveOrder() {
		while(true) {
			try {
				socket = serverSocket.accept();
				InputStream orderStream = socket.getInputStream();
				byte[] orderArray = new byte[ORDER_SIZE];
				orderStream.read(orderArray, 0, ORDER_SIZE);
				String order = new String(orderArray);
				System.out.println(order);
			} catch (IOException ioe) {
				System.err.println("Unable to accept");
				System.exit(1);
			}

			
		}
	}


}
