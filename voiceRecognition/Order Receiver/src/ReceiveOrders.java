
public class ReceiveOrders {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		OrderStreamReceiver orderReceiver = new OrderStreamReceiver(4321);
		orderReceiver.receiveOrder();

	}

}
