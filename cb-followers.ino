/*************************************(Chaturbate Details)**************************************/

void getCBFollowerData(){
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Connecting to server..."));
  if (!modelClient.connect("chaturbate.com", 443)) {
    Serial.println(F("Failed to connect to server"));
    return;
  }
  Serial.println(F("Connected to server"));

  Serial.println(F("Sending request..."));
  // Send HTTP request
  modelClient.println("GET /statsapi/?username=" + cbusername + "&token=" + cbtoken + " HTTP/1.0");
  modelClient.println(F("Host: chaturbate.com"));
  modelClient.println(F("User-Agent: Arduino"));
  modelClient.println(F("Connection: close"));
  modelClient.println();

  
  //Check the returning code                                                                  
    char status[32] = {0};
  modelClient.readBytesUntil('\r', status, sizeof(status));
  // should be "HTTP/1.0 200 OK"
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected HTTP status"));
    Serial.println(status);
    return;
  }

    Serial.println(F("Receive response..."));

    // Skip response headers
    modelClient.find("\r\n\r\n");

      // CB v5
      const size_t bufferSize = JSON_OBJECT_SIZE(12) + 600;
      
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(modelClient);
      // Parameters

      numFollowers = root["num_followers"];
      tokenBalance = root["token_balance"];

      resultPayout = tokenBalance * 0.05; // comment out this line if you're not a model on Chaturbate

      String specialC = String(char('~' + 25));
      
      cbFollowerString = "You have " + String(numFollowers) + " " + specialC+ "B Followers   ";
      cbFollowerString += "Your balance is: " + String(tokenBalance) + " tokens    ";
      cbFollowerString += "Converted: $" + String(resultPayout) + " this period                "; // Comment out this line if you're not a model on Chaturbate
      
    }
    modelClient.stop();  //Leh
}
