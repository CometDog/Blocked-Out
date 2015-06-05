var MessageQueue=function(){var RETRY_MAX=5;var queue=[];var sending=false;var timer=null;return{reset:reset,sendAppMessage:sendAppMessage,size:size};function reset(){queue=[];sending=false}function sendAppMessage(message,ack,nack){if(!isValidMessage(message)){return false}queue.push({message:message,ack:ack||null,nack:nack||null,attempts:0});setTimeout(function(){sendNextMessage()},1);return true}function size(){return queue.length}function isValidMessage(message){if(message!==Object(message)){return false}var keys=Object.keys(message);if(!keys.length){return false}for(var k=0;k<keys.length;k+=1){var validKey=/^[0-9a-zA-Z-_]*$/.test(keys[k]);if(!validKey){return false}var value=message[keys[k]];if(!validValue(value)){return false}}return true;function validValue(value){switch(typeof value){case"string":return true;case"number":return true;case"object":if(toString.call(value)=="[object Array]"){return true}}return false}}function sendNextMessage(){if(sending){return}var message=queue.shift();if(!message){return}message.attempts+=1;sending=true;Pebble.sendAppMessage(message.message,ack,nack);timer=setTimeout(function(){timeout()},1e3);function ack(){clearTimeout(timer);setTimeout(function(){sending=false;sendNextMessage()},200);if(message.ack){message.ack.apply(null,arguments)}}function nack(){clearTimeout(timer);if(message.attempts<RETRY_MAX){queue.unshift(message);setTimeout(function(){sending=false;sendNextMessage()},200*message.attempts)}else{if(message.nack){message.nack.apply(null,arguments)}}}function timeout(){setTimeout(function(){sending=false;sendNextMessage()},1e3);if(message.ack){message.ack.apply(null,arguments)}}}}();var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = "http://api.openweathermap.org/data/2.5/forecast?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + "&cnt=1";

  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
      
      var country = json.city.country;
      var cof;
      if (country == "US" || country == "BS" || country == "BZ" || country == "KY" || country == "PW" || country == "AR") {
        cof = 1;
      }
      else {
        cof = 0;
      }

      var temperature1 = Math.round(json.list[0].main.temp - 273.15);
      var temperature2 = Math.round(json.list[1].main.temp - 273.15);
      var temperature3 = Math.round(json.list[2].main.temp - 273.15);
      var temperature4 = Math.round(json.list[3].main.temp - 273.15);
      var temperature5 = Math.round(json.list[4].main.temp - 273.15);
      
      var conditions1 = json.list[1].weather[0].id;
      var conditions2 = json.list[2].weather[0].id;
      var conditions3 = json.list[3].weather[0].id;
      var conditions4 = json.list[4].weather[0].id;
      
      var humidity = json.list[0].main.humidity;
      
      var dictionary = {
        "COUNTRY": cof,
        "TEMPERATURE_1": temperature1,
        "TEMPERATURE_2": temperature2,
        "TEMPERATURE_3": temperature3,
        "TEMPERATURE_4": temperature4,
        "TEMPERATURE_5": temperature5,
        "CONDITIONS_1": conditions1,
        "CONDITIONS_2": conditions2,
        "CONDITIONS_3": conditions3,
        "CONDITIONS_4": conditions4,
        "HUMIDITY": humidity
      };

      MessageQueue.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 5000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    getWeather();
  }
);