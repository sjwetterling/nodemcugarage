const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style type="text/css">
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 40px 40px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 2em; 
  width: 50%;
}
</style>
<body style="background-color: #ffffff ">
<center>
<div>
<h1>GARAGE DOOR</h1>
  <button class="button" onclick="send(1)">TOGGLE GARAGE</button>
</div>
 <br>
<div><h1>
  Garage is: <span id="door_val">WAITING FOR STATUS</span><br><br>
</h1>
</div>
<script>
function send(led_sts) 
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("state").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "door_control?state="+led_sts, true);
  xhttp.send();
}
setInterval(function() 
{
  getData();
}, 2000); 
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("door_val").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "doorread", true);
  xhttp.send();
}
</script>
</center>
</body>
</html>
)=====";