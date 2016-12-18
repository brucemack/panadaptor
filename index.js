const express = require("express");
const SerialPort = require("serialport");
const app = express();

function createLineBuilder(eventCb,MAX_BUFFER_SIZE = 1024) {
  return {
    // This is where data accumulates while we are trying to build a line
    buffer: Buffer.alloc(MAX_BUFFER_SIZE),
    bufferPtr: 0,
    processBuffer: function(buffer) {
      for (var i = 0; i < buffer.length; i++) {
        var b = buffer[i];
        if (b == 10 || b == 13) {
          if (this.bufferPtr > 0) {
            // Fire the callback using a string created from the accumulated
            // data.
            eventCb(this.buffer.toString("ascii",0,this.bufferPtr));
            // Clear the accumulator
            this.bufferPtr = 0;
          }
        } else {
          if (this.bufferPtr < MAX_BUFFER_SIZE) {
            this.buffer.writeInt8(b,this.bufferPtr++);
          }
        }
      }
    }
  };
}

var spectrumData = [];

// Temporary
/*
setInterval(function() {
    // Create some random data
    var result = "SPECTRUM";
    for (var i = 0; i < 32; i++) {
      //var d = Math.trunc(Math.random() * 100);
      var d = (i / 32) * 100;
      result += ",";
      result += d;
    }
    lineProcessor(result);
  },500);
*/
function lineProcessor(line) {

  //console.log("LINE: " + line);

  // Parse
  var tokens = line.split(',');
  if (tokens[0] == "GENERAL") {
    tokens.splice(0,1);
  } else if (tokens[0] == "SPECTRUM") {
    tokens.splice(0,1);
    spectrumData = tokens;
  }
}

const lineBuilder = createLineBuilder(lineProcessor);

var port = new SerialPort("COM3",
  {
    baudRate: 9600
  },
  function(err) {
    if (err) {
      console.log("OPEN FAILURE: " + err);
      return;
    }
    console.log("Opened");
  }
);

// Connect the serial port receiver callback to the line builder
port.on("data",function(b) {
  //console.log(b);
  lineBuilder.processBuffer(b);
});

app.get("/data",function(req,res) {
  res.type("json");
  res.send(spectrumData);
});

app.use(express.static('static'));

app.listen(3000, function () {
  console.log('Example app listening on port 3000!')
});
