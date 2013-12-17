package com.example.ordertaker;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
 
public class MainActivity extends Activity {
 
    protected static final int RESULT_SPEECH = 1;
 
    private Button killButton;
    private Button sendOrderButton;
    private EditText ipText;
    private ImageButton btnSpeak;
    private TextView txtText;
    private String currentOrder = "";
    private String targetIp; 
    private int targetPort = 4321;
    
    OnClickListener forceQuitListener = new OnClickListener() {
    	public void onClick(View view) {
    		System.exit(0);
    	}
    };
    
    OnClickListener sendOrderListener = new OnClickListener() {
    	public void onClick(View view) {
    		Thread sendOrderThread = new Thread(sendOrderRunnable);
    		sendOrderThread.start();
    	}
    };
    
    public Runnable sendOrderRunnable = new Runnable() {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			sendOrder();
			
		}
    	
    };
    
    public void sendOrder() {
		try {
			targetIp = ipText.getText().toString();
			Socket socket = new Socket(targetIp, targetPort);
			OutputStream outputStream = socket.getOutputStream();
			String tableThenOrder = currentOrder;
			if (tableThenOrder.contains("table 1") || tableThenOrder.contains("table one")) {
				tableThenOrder = "4 " + tableThenOrder;
			} else if (tableThenOrder.contains("table 2") || (tableThenOrder.contains("table two"))) {
				tableThenOrder = "5 " + tableThenOrder;
			} else if (tableThenOrder.contains("table 3") || (tableThenOrder.contains("table three"))) {
				tableThenOrder = "6 " + tableThenOrder;
			} else if (tableThenOrder.contains("table 4") || (tableThenOrder.contains("table four"))) {
				tableThenOrder = "7 " + tableThenOrder;
			} else {
				System.out.println("You must include a table number");
				return;
			}
			outputStream.write(tableThenOrder.getBytes());
			outputStream.flush();
			outputStream.close();
			socket.close();
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
 
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
 
        txtText = (TextView) findViewById(R.id.txtText);
 
        btnSpeak = (ImageButton) findViewById(R.id.btnSpeak);
        ipText = (EditText) findViewById(R.id.ipText);
        killButton = (Button) findViewById(R.id.killButton);
        killButton.setOnClickListener(forceQuitListener);
        sendOrderButton = (Button) findViewById(R.id.sendOrderButton);
        sendOrderButton.setOnClickListener(sendOrderListener);
 
        btnSpeak.setOnClickListener(new View.OnClickListener() {
 
            @Override
            public void onClick(View v) {
 
                Intent intent = new Intent(
                        RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
 
                intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, "en-US");
 
                try {
                    startActivityForResult(intent, RESULT_SPEECH);
                    txtText.setText("");
                } catch (ActivityNotFoundException a) {
                    Toast t = Toast.makeText(getApplicationContext(),
                            "Oops! Your device doesn't support Speech to Text",
                            Toast.LENGTH_SHORT);
                    t.show();
                }
            }
        });
 
    }
 
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
 
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
 
        switch (requestCode) {
        	case RESULT_SPEECH: {
        		if (resultCode == RESULT_OK && null != data) {
 
        			ArrayList<String> text = data
        					.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);
 
        			currentOrder = text.get(0);
        			txtText.setText(text.get(0));
        		}
        		break;
        }
 
        }
    }
    
    
}