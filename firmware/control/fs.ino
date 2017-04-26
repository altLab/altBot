extern ESP8266WebServer server;

File UploadFile;
String fileName;


void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if (!path.startsWith("/")) path = "/" + path;
  Serial.println("handleFileCreate: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  //  path = String();
}

void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if (!path.startsWith("/")) path = "/" + path;
  Serial.println("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  //  path = String();
}

// An empty ESP8266 Flash ROM must be formatted before using it, actual a problem
void handleFormat() {
  Serial.println("Format SPIFFS");
  if (SPIFFS.format()) {
    if (!SPIFFS.begin()) {
      Serial.println("Format SPIFFS failed");
    }
  } else {
    Serial.println("Format SPIFFS failed");
  }
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS failed, needs formatting");
  } else {
    Serial.println("SPIFFS mounted");
  }
}

void handleFileList() {
  //  if (!server.hasArg("dir")) {
  //    server.send(500, "text/plain", "BAD ARGS");
  //    return;
  //  }

  String path = !server.hasArg("dir") ? "/" : server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  //if(filename.endsWith(".src")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    size_t contentLength = file.size();

    file.close();
    return true;
  }
  return false;
}

void handleUpload() {
  const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", serverIndex);
}

void handleUploadRequest() {
  if (server.uri() != "/update") return;
  // get file
  HTTPUpload& upload = server.upload();
  Serial.print("status: ");
  Serial.println( (int)upload.status);  // need 2 commands to work!
  if (upload.status == UPLOAD_FILE_START) {
    fileName = upload.filename;
    Serial.println("Upload Name: " + fileName);
    String path = "/" + fileName;
    UploadFile = SPIFFS.open(path, "w");
    // already existing file will be overwritten!
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (UploadFile)
      UploadFile.write(upload.buf, upload.currentSize);
    Serial.println(fileName + " size: " + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.print("Upload Size: ");
    Serial.println(upload.totalSize);  // need 2 commands to work!
    if (UploadFile)
      UploadFile.close();
  }
  yield();
}

void handleUpdate() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  // ESP.restart();  // not needed
  Serial.println("dir SPIFFS");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
  }
}

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

void spiffs_init() {
  // show the SPIFFS contents, name and size
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS failed, needs formatting");
  } else {
    Serial.println("dir SPIFFS");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.println();
    FSInfo fs_info;
    if (!SPIFFS.info(fs_info)) {
      Serial.println("fs_info failed");
    } else {
      Serial.printf("Total: %u\nUsed: %u\nBlock: %u\nPage: %u\nMax open files: %u\nMax path len: %u\n",
                    fs_info.totalBytes,
                    fs_info.usedBytes,
                    fs_info.blockSize,
                    fs_info.pageSize,
                    fs_info.maxOpenFiles,
                    fs_info.maxPathLength
                   );
    }
  }
}


