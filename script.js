// Initialize Firebase
const firebaseConfig = {
    apiKey: "AIzaSyAwPc6mM5NLmqnujzYyPcNvax3XNepL5tc",
    authDomain: "trialultrasonic.firebaseapp.com",
    databaseURL: "https://trialultrasonic-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "trialultrasonic",
    storageBucket: "trialultrasonic.appspot.com",
    messagingSenderId: "158712235273",
    appId: "1:158712235273:web:258c6c474b5d292fca3fd8",
    measurementId: "G-YJRPB7S5WP"
  };
  
// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference to the database
var database = firebase.database();

// Fetch real sensor data for the "Real" sensors
var realDistanceRef = database.ref('Real/Ultrasonic/Distance');
var realCO2LevelRef = database.ref('Real/MQ135/CO2Level');
var realForceRef = database.ref('Real/ForceSensor/Force');
var realTempref = database.ref('Real/Temperature/Temp')

realDistanceRef.on('value', function(snapshot) {
    var distance = snapshot.val();
    document.getElementById('realDistance').innerHTML = distance + " cm";
}, function(error) {
    console.error('Error fetching real distance data:', error);
});

realCO2LevelRef.on('value', function(snapshot) {
    var co2Level = snapshot.val();
    document.getElementById('realCO2Level').innerHTML = co2Level + " ppm";
}, function(error) {
    console.error('Error fetching real CO2 level data:', error);
});

realForceRef.on('value', function(snapshot) {
    var sensorValueForce = snapshot.val();
    document.getElementById('realForce').innerHTML = sensorValueForce + " N";
}, function(error) {
    console.error('Error fetching force sensor data:', error);
});

realTempref.on('value', function(snapshot) {
    var temperature = snapshot.val();
    document.getElementById('realTemp').innerHTML = temperature + " C";
}, function(error) {
    console.error('Error fetching force sensor data:', error);
});


// Fetch dummy sensor data for the other five boxes
for (let i = 1; i <= 5; i++) {
    (function(index) {
        const dummyDistanceRef = database.ref('Dummy' + index + '/Ultrasonic/Distance');
        const dummyCO2LevelRef = database.ref('Dummy' + index + '/MQ135/CO2Level');
        const dummyForceRef = database.ref('Dummy' + index + '/ForceSensor/Force');
        const dummyTempRef = database.ref('Dummy' + index + '/Temperature/Temp');

        dummyDistanceRef.on('value', function(snapshot) {
            const dummyDistance = snapshot.val();
            document.getElementById('dummyDistance' + index).innerHTML = dummyDistance + " cm";
        }, function(error) {
            console.error('Error fetching dummy distance data:', error);
        });

        dummyCO2LevelRef.on('value', function(snapshot) {
            const dummyCO2Level = snapshot.val();
            document.getElementById('dummyCO2Level' + index).innerHTML = dummyCO2Level + " ppm";
        }, function(error) {
            console.error('Error fetching dummy CO2 level data:', error);
        });

        dummyForceRef.on('value', function(snapshot) {
            const dummyForce = snapshot.val();
            document.getElementById('dummyForce' + index).innerHTML = dummyForce + " N";
        }, function(error) {
            console.error('Error fetching dummy force data:', error);
        });
        dummyTempRef.on('value', function(snapshot) {
            const dummyTemp = snapshot.val();
            document.getElementById('dummyTemp' + index).innerHTML = dummyTemp + " C";
        }, function(error) {
            console.error('Error fetching dummy force data:', error);
        });
    })(i);
}

// Function to open the map page
function openMapPage() {
    window.open('map.html', '_blank');
}


// Function to filter helmet boxes by CO2 level
function filterByCO2(co2Level) {
    const helmetBoxes = document.querySelectorAll('.worker-box');
    helmetBoxes.forEach(box => {
        const co2Span = box.querySelector('.co2-level');
        if (co2Span) {
            const co2Value = parseInt(co2Span.innerText);
            if (co2Value >= co2Level) {
                box.style.display = 'block';
            } else {
                box.style.display = 'none';
            }
        }
    });
}

// Function to filter helmet boxes by heart rate
function filterByHeartRate(heartRate) {
    const helmetBoxes = document.querySelectorAll('.worker-box');
    helmetBoxes.forEach(box => {
        const heartRateSpan = box.querySelector('.heart-rate');
        if (heartRateSpan) {
            const heartRateValue = parseInt(heartRateSpan.innerText);
            if (heartRateValue <= heartRate) {
                box.style.display = 'block';
            } else {
                box.style.display = 'none';
            }
        }
    });
}

// Event listener for CO2 level dropdown
document.querySelectorAll('.dropdown-content')[0].addEventListener('click', function(event) {
    const selectedCO2 = parseInt(event.target.innerText);
    if (!isNaN(selectedCO2)) {
        filterByCO2(selectedCO2);
    }
});

// Event listener for heart rate dropdown
document.querySelectorAll('.dropdown-content')[1].addEventListener('click', function(event) {
    const selectedHeartRate = parseInt(event.target.innerText);
    if (!isNaN(selectedHeartRate)) {
        filterByHeartRate(selectedHeartRate);
    }
});
