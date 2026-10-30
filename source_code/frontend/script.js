// Task 4 Dashboard: script.js (Modular Firebase SDK v10)
import { initializeApp } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-app.js";
import { getAuth, signInWithEmailAndPassword, signOut, onAuthStateChanged } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-auth.js";
import { getDatabase, ref, onValue, set, get } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-database.js";

// Firebase Project Configuration
const firebaseConfig = {
  apiKey: "YOUR_FIREBASE_API_KEY",
  authDomain: "YOUR_PROJECT_ID.firebaseapp.com",
  databaseURL: "YOUR_FIREBASE_DATABASE_URL",
  projectId: "YOUR_PROJECT_ID",
  storageBucket: "YOUR_PROJECT_ID.appspot.com",
  messagingSenderId: "YOUR_SENDER_ID",
  appId: "YOUR_APP_ID"
};

// Initialize Services
const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const db = getDatabase(app);

let currentBulbState = false;

// Auth State Observer
onAuthStateChanged(auth, (user) => {
  if (user) {
    document.getElementById("auth-section").style.display = "none";
    document.getElementById("telemetry-section").style.display = "block";
    initRealtimeStreams();
  } else {
    document.getElementById("auth-section").style.display = "block";
    document.getElementById("telemetry-section").style.display = "none";
  }
});

// User Sign In
window.handleLogin = () => {
  const email = document.getElementById("user-email").value;
  const pass = document.getElementById("user-password").value;
  const errBox = document.getElementById("auth-error");
  errBox.innerText = "";

  signInWithEmailAndPassword(auth, email, pass).catch((err) => {
    errBox.innerText = "Authentication Error: " + err.message;
  });
};

// User Sign Out
window.handleLogout = () => {
  signOut(auth);
};

// Bind Real-time RTDB Listeners to DOM
function initRealtimeStreams() {
  // Listen for Temperature changes
  onValue(ref(db, "sensors/temperature"), (snapshot) => {
    const val = snapshot.val();
    document.getElementById("val-temp").innerText = (val !== null) ? val.toFixed(1) + " Â°C" : "--.- Â°C";
  });

  // Listen for Humidity changes
  onValue(ref(db, "sensors/humidity"), (snapshot) => {
    const val = snapshot.val();
    document.getElementById("val-hum").innerText = (val !== null) ? val.toFixed(1) + " %" : "-- %";
  });

  // Listen for LDR Light changes
  onValue(ref(db, "sensors/ldr"), (snapshot) => {
    const val = snapshot.val();
    document.getElementById("val-ldr").innerText = (val !== null) ? val : "----";
  });

  // Listen for Bulb Relay state
  onValue(ref(db, "control/bulb"), (snapshot) => {
    currentBulbState = snapshot.val() === true;
    const badge = document.getElementById("bulb-state-label");
    if (currentBulbState) {
      badge.className = "state-badge badge-on";
      badge.innerText = "BULB IS ON";
    } else {
      badge.className = "state-badge badge-off";
      badge.innerText = "BULB IS OFF";
    }
  });
}

// Push Remote Actuator Toggle
window.toggleBulbState = () => {
  set(ref(db, "control/bulb"), !currentBulbState);
};