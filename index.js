const express = require("express");

const app = express();

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
})
