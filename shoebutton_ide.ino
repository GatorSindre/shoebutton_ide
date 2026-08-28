#include <WiFi.h>
#include <WebServer.h>

// CD Laptop
//              c:\Users\olsen\OneDrive - Troms Fylkeskommune\Skrivebord2\GitHub\shoebutton_ide


// =========================
// Wi-Fi
// =========================

const char* ssid = "Nettverksnavn";
const char* password = "35961574";

WebServer server(80);

// =========================
// FSR / Morse settings
// =========================

const int FSR_PIN = 3;
const int PRESS_THRESHOLD = 1000;

// How long a press must be to count as a dash.
// Below this = dot
const unsigned long DOT_DASH_TIME = 300;

// If held for a whole second it becomes a special char to activate secondary mode
const unsigned long SPECIAL_CHAR_TIME = 1000;

// How long the FSR must be released before
// the current Morse character is translated.
const unsigned long CHARACTER_PAUSE = 700;

// How often we read the FSR
const unsigned long READ_INTERVAL = 50;


String morseCode = "";
String fullMessage = "";

bool wasPressed = false;

unsigned long pressStartTime = 0;
unsigned long releaseStartTime = 0;
unsigned long lastReadTime = 0;
bool specialActive = false;


char translateMorse(String code) {

  if (code == ".-") return 'A';
  if (code == "-...") return 'B';
  if (code == "-.-.") return 'C';
  if (code == "-..") return 'D';
  if (code == ".") return 'E';
  if (code == "..-.") return 'F';
  if (code == "--.") return 'G';
  if (code == "....") return 'H';
  if (code == "..") return 'I';
  if (code == ".---") return 'J';
  if (code == "-.-") return 'K';
  if (code == ".-..") return 'L';
  if (code == "--") return 'M';
  if (code == "-.") return 'N';
  if (code == "---") return 'O';
  if (code == ".--.") return 'P';
  if (code == "--.-") return 'Q';
  if (code == ".-.") return 'R';
  if (code == "...") return 'S';
  if (code == "-") return 'T';
  if (code == "..-") return 'U';
  if (code == "...-") return 'V';
  if (code == ".--") return 'W';
  if (code == "-..-") return 'X';
  if (code == "-.--") return 'Y';
  if (code == "--..") return 'Z';

  // Numbers
  if (code == "-----") return '0';
  if (code == ".----") return '1';
  if (code == "..---") return '2';
  if (code == "...--") return '3';
  if (code == "....-") return '4';
  if (code == ".....") return '5';
  if (code == "-....") return '6';
  if (code == "--...") return '7';
  if (code == "---..") return '8';
  if (code == "----.") return '9';

  return '?';
}

void runCMD() {

  String cmdMessage = fullMessage;
  fullMessage = "";

  if (cmdMessage == "wf") {
    // TEST FUNCTION FOR NOW
    fullMessage += "wifi";
  }
  else if (cmdMessage == "kb") {
    // TEST FUNCTION FOR NOW
    fullMessage += "keyboard";
  }
}

void specialCMD(char letter) {
  switch (letter) {
    case 'E':
      // Enter function
      runCMD();
      break;
    case 'i':
      if (fullMessage.length() > 0) {
        fullMessage.remove(fullMessage.length() - 1);
      }
      break;
    case 'S':
      fullMessage = "";
      break;
    default:
      // Case default :
      break;
  }
}

void handleRoot() {
  String page = "<html><body>";
  page += "<h1>Morse Code</h1>";
  page += "<h2>" + fullMessage + "</h2>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Morse ready!");

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  unsigned long currentTime = millis();

  // Only read the sensor every READ_INTERVAL milliseconds
  if (currentTime - lastReadTime < READ_INTERVAL) {
    return;
  }

  lastReadTime = currentTime;

  int value = analogRead(FSR_PIN);

  bool pressed = value > PRESS_THRESHOLD;


  // Button just pressed
  if (pressed && !wasPressed) {

    pressStartTime = currentTime;

    Serial.println("PRESS");
  }


  // Button just released
  if (!pressed && wasPressed) {

    unsigned long pressDuration = currentTime - pressStartTime;

    Serial.print("HoldTime: ");
    Serial.print(pressDuration);
    Serial.println(" ms");

    if (pressDuration > SPECIAL_CHAR_TIME) {

      morseCode += "_";

      Serial.println("SPECIAL");


    } else if (pressDuration < DOT_DASH_TIME) {

      morseCode += ".";

      Serial.println("DOT");

    } else {

      morseCode += "-";

      Serial.println("DASH");
    }

    releaseStartTime = currentTime;
  }


  // Character finished
  if (!pressed && morseCode.length() > 0 && currentTime - releaseStartTime >= CHARACTER_PAUSE) {

    char letter = translateMorse(morseCode);

    if (letter == '_') {
      specialActive = true;
    }
    else if (specialActive == true) {
      specialCMD(letter);  
      specialActive = false;
    }
    else if (letter != '?') {
      // Add the decoded letter to the full message
      fullMessage += letter;
    }


    Serial.print("Morse: ");
    Serial.print(morseCode);
    Serial.print(" = ");
    Serial.println(letter);
    Serial.println(fullMessage);

    morseCode = "";
  }


  wasPressed = pressed;
}