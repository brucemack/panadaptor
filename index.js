const express = require("express");
const SerialPort = require("serialport");
const Readline = SerialPort.parsers.Readline;
const parser = new Readline();
const app = express();

var port = SerialPort("COM3",{
    baudRate: 9600
  },function(err) {
    if (err) {
      console.log("OPEN FAILURE: " + err);
      return;
    }
    console.log("Opened");
  });

port.pipe(parser);

port.on("data",function(data) {
      consloe.log(data);
});

app.get("/data",function(req,res) {
  res.type("json");
  // Create some random data
  var data = [];
  for (var i = 0; i < 32; i++) {
    data.push(Math.trunc(Math.random() * 100));
  }
  res.send(data);
});

app.use(express.static('static'));

app.listen(3000, function () {
  console.log('Example app listening on port 3000!')
});
