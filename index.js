const express = require("express");
const SerialPort = require("serialport");
const app = express();

// Builds a message builder that can handle a message with this format:
//
// 0xab
// 2 byte length (little Endian)
// (message body)
//
function createMsgBuilder(eventCb) {

  // Start of message sequence
  const START_OF_MESSAGE = 0xab;

  return {
    // This is where data accumulates while we are trying to build a line
    buffer: null,
    processBuffer: function(inputBuffer) {
      // Append buffer onto whatever we have already from previous requests
      if (!this.buffer) {
        this.buffer = Buffer.from(inputBuffer);
      } else {
        this.buffer = Buffer.concat([this.buffer,inputBuffer]);
      }
      while (this.buffer) {
        // Hunt for a start of message sequence
        var idx = this.buffer.indexOf(START_OF_MESSAGE);
        // Strip anything before the start of message sequence (assumed to be garbage)
        if (idx == -1) {
          this.buffer = null;
          break;
        } else if (idx > 0) {
          this.buffer = this.buffer.slice(idx);
        }
        if (this.buffer.length > 3) {
          // At this point we have a buffer that starts with the start of message
          // marker.  So we read the length word and find out if the entire
          // message is in the buffer.
          var msgLength = this.buffer.readUInt16LE(1);
          // Find out if we have enough in the buffer for an entire message
          if (this.buffer.length >= msgLength) {
            // If enough data is available then splice it out and pass it to
            // the callback
            eventCb(Buffer.from(this.buffer.slice(3,msgLength)));
            // Strip that part off the buffer
            if (this.buffer.length == msgLength) {
              this.buffer = null;
            } else {
              this.buffer = this.buffer.slice(msgLength);
            }
          } else {
            break;
          }
        } else {
          break;
        }
      }
    }
  };
}


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

var mode0Data = {
  sampleLst: 0,
  sampleMin: 0,
  sampleMax: 0,
  sampleAvg: 0,
  sampleStd: 0,
  fftBlockSize: 0,
  fftMaxMag: 0,
  fftMaxMagBucket: 0,
  fftData: null
};

// Temporary (test data generator)

setInterval(function() {

    // Create some random data
    var t0 = new Buffer([
      0xab,
      84,
      0,
      0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      64, 0, 0, 0, 0, 0,
      0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ])
    ;
    msgBuilder.processBuffer(t0);

  },2000);

const msgBuilder = createMsgBuilder((msg) => {
  // NOTE: The start flag and the length have been stripped off at this point
  var mode = msg.readUInt8(0);
  if (mode == 0) {
    mode0Data.sampleLst = msg.readUInt16LE(1);
    mode0Data.sampleMin = msg.readUInt16LE(3);
    mode0Data.sampleMax = msg.readUInt16LE(5);
    mode0Data.sampleAvg = msg.readUInt16LE(7);
    mode0Data.sampleStd = msg.readUInt16LE(9);
    mode0Data.fftBlockSize = msg.readUInt16LE(11);
    mode0Data.fftMaxMag = msg.readUInt16LE(13);
    mode0Data.fftMaxMagBucket = msg.readUInt16LE(15);
    var fftData = [];
    for (var i = 0; i < mode0Data.fftBlockSize / 2; i++) {
      fftData.push(msg.readUInt16LE(17 + i * 2));
    }
    mode0Data.fftData = fftData;
  }
});

var port = new SerialPort("COM3",
  {
    baudRate: 38400
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
port.on("data",(b) => { msgBuilder.processBuffer(b); });

app.get("/data",function(req,res) {
  res.type("json");
  res.send(mode0Data.fftData);
});

app.use(express.static('static'));

app.listen(3000, function () {
  console.log('Example app listening on port 3000!')
});

// ============================================================================
// Test stuff
/*
console.log("Testing ...");

var pr = createMsgBuilder(function(msg) {
  console.log("Received:",msg,msg.length);
});

// Test message (length 3 + header = 6)
var t0 = new Buffer([ 0xab, 0x06, 0x00, 0x01, 0x02, 0x03 ]);
pr.processBuffer(t0);

// This is a test of another message that has garbage at the beginning, and
// split into two parts
var t1 = new Buffer([ 0xff, 0xff, 0xab, 0x06 ]);
var t2 = new Buffer([ 0x00, 0x04, 0x05, 0x03 ]);
pr.processBuffer(t1);
pr.processBuffer(t2);
*/
