#include <WiFi.h>
#include <WebServer.h>

// --- Pin Definitions ---
const int in1 = 25; // Motor A
const int in2 = 26; // Motor A
const int in3 = 32; // Motor B
const int in4 = 33; // Motor B
const int ena = 4;  // PWM Speed Control (Enable A)
const int enb = 5;  // PWM Speed Control (Enable B - optional for 4WD)

// --- PWM Settings ---
const int freq = 2000;
const int res = 8; // 8-bit resolution (0-255)
const int chA = 0;
const int chB = 1;

WebServer server(80);

// --- HTML & Joystick Interface ---
String getHTML() {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>";
  s += "<style>body{background:#222; color:white; text-align:center; touch-action:none; font-family:sans-serif;}";
  s += "#zone{width:250px; height:250px; background:#444; border-radius:50%; margin:50px auto; position:relative; border:4px solid cyan;}";
  s += "#stick{width:80px; height:80px; background:cyan; border-radius:50%; position:absolute; top:85px; left:85px; transition: 0.1s;}</style>";
  s += "</head><body>";
  s += "<h1>ESP32 RC CAR</h1><div id='zone'><div id='stick'></div></div>";
  
  // JavaScript for Joystick Logic
  s += "<script>";
  s += "var zone=document.getElementById('zone'), stick=document.getElementById('stick');";
  s += "var x=0, y=0, dragging=false;";
  s += "zone.addEventListener('touchstart', start); zone.addEventListener('touchmove', move); zone.addEventListener('touchend', end);";
  s += "function start(e){ dragging=true; }";
  s += "function end(){ dragging=false; stick.style.left='85px'; stick.style.top='85px'; send(0,0); }";
  s += "function move(e){ if(!dragging) return; var rect=zone.getBoundingClientRect();";
  s += "var touch=e.touches[0]; x=touch.clientX-rect.left-125; y=touch.clientY-rect.top-125;";
  s += "var limit=80; var dist=Math.sqrt(x*x+y*y); if(dist>limit){ x*=limit/dist; y*=limit/dist; }";
  s += "stick.style.left=(x+85)+'px'; stick.style.top=(y+85)+'px';";
  s += "send(Math.round(x), Math.round(-y)); }"; // Flip Y for forward
  s += "function send(jx, jy){ fetch('/joy?x='+jx+'&y='+jy); }";
  s += "</script></body></html>";
  return s;
}

// --- Motor Control Logic ---
void moveMotors(int x, int y) {
  // Differential drive mapping (simplified)
  int leftSpeed = y + x;
  int rightSpeed = y - x;

  // Constrain to 0-255 range
  leftSpeed = constrain(leftSpeed * 3, -255, 255); 
  rightSpeed = constrain(rightSpeed * 3, -255, 255);

  // Left Motor
  if (leftSpeed >= 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    ledcWrite(chA, leftSpeed);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    ledcWrite(chA, abs(leftSpeed));
  }

  // Right Motor
  if (rightSpeed >= 0) {
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
    ledcWrite(chB, rightSpeed);
  } else {
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
    ledcWrite(chB, abs(rightSpeed));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  
  // PWM Setup
  ledcSetup(chA, freq, res);
  ledcSetup(chB, freq, res);
  ledcAttachPin(ena, chA);
  ledcAttachPin(enb, chB);

  // Access Point Setup
  WiFi.softAP("ESP32-RC-Car", "12345678");
  Serial.println("AP IP address: " + WiFi.softAPIP().toString());

  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/joy", []() {
    int x = server.arg("x").toInt();
    int y = server.arg("y").toInt();
    moveMotors(x, y);
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
