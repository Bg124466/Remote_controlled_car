#include <WiFi.h>
#include <WebServer.h>

// Pins & PWM
cons int in1 = 25; const int in2 = 26;
const int in3 = 32; const int in4 = 33;
const int eep = 4;

WebServer server(80);

void moveMotors(int left, int right) {
  if (left >= 0) { ledcWrite(0, left); ledcWrite(1, 0); }
  else { ledcWrite(0, 0); ledcWrite(1, abs(left)); }
  if (right >= 0) { ledcWrite(2, right); ledcWrite(3, 0); }
  else { ledcWrite(2, 0); ledcWrite(3, abs(right)); }
}

String getHTML() {
  // HTML HII HAIHITAJI INTANETI
  String s = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>";
  s += "<style>body{background:#222; color:white; text-align:center; touch-action:none;} #zone{width:250px; height:250px; background:#444; border-radius:50%; margin:50px auto; position:relative;} #stick{width:80px; height:80px; background:cyan; border-radius:50%; position:absolute; top:85px; left:85px;}</style></head>";
  s += "<body><h1>ESP32 OFFLINE JOYSTICK</h1><div id='zone'><div id='stick'></div></div>";
  s += "<script>var z=document.getElementById('zone'), s=document.getElementById('stick'), sx=85, sy=85, active=false;";
  s += "function drive(x,y){ fetch('/d?x='+x+'&y='+y); }";
  s += "z.addEventListener('touchstart',function(e){active=true;});";
  s += "z.addEventListener('touchmove',function(e){if(!active)return; var r=z.getBoundingClientRect(), tx=e.touches[0].clientX-r.left-40, ty=e.touches[0].clientY-r.top-40;";
  s += "s.style.left=tx+'px'; s.style.top=ty+'px'; drive(Math.round(tx-85), Math.round(85-ty));});";
  s += "z.addEventListener('touchend',function(){active=false; s.style.left='85px'; s.style.top='85px'; drive(0,0);});</script></body></html>";
  return s;
}

void setup() {
  Serial.begin(115200);
  pinMode(eep, OUTPUT); digitalWrite(eep, HIGH);
  ledcSetup(0, 5000, 8); ledcAttachPin(in1, 0);
  ledcSetup(1, 5000, 8); ledcAttachPin(in2, 1);
  ledcSetup(2, 5000, 8); ledcAttachPin(in3, 2);
  ledcSetup(3, 5000, 8); ledcAttachPin(in4, 3);
  WiFi.softAP("ESP32_RC_Car", "12345678");
  server.on("/", [](){ server.send(200, "text/html", getHTML()); });
  server.on("/d", [](){ 
    int x = server.arg("x").toInt() * 2;
    int y = server.arg("y").toInt() * 2;
    moveMotors(constrain(y+x,-255,255), constrain(y-x,-255,255));
    server.send(200);
  });
  server.begin();
}
void loop() { server.handleClient(); }
