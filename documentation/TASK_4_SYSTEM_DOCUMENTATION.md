# Task 4 Technical Implementation Report
## Firebase IoT Monitoring Dashboard

### Database Topology Analysis
1. **Live Authentic Database (`Apppp` Project):**
   - **URL:** `https://apppp-f359e-default-rtdb.asia-southeast1.firebasedatabase.app`
   - **Structure:**
     - `/current`: Holds the live edge telemetry and actuator state (`bulbState: false`, `humidity: 64`, `ldr: 750`, `mode: "AUTO"`, `temperature: 28.5`).
     - `/history`: Stores timestamped historical push records (e.g. push ID `-Nh1a` logging `bulbState: false`, `humidity: 60`, `ldr: 820`).
2. **Conceptual Schema Diagram:**
   - Visualized in `images/firebase_sensors_control_database_mockup.png` depicting theoretical `/sensors` and `/control` hierarchy.
3. **Audit Status:**
   - Conservatives maintained as **PARTIAL** on Criterion 16 strictly because the authentic database organizes telemetry under `/current` rather than separate top-level branches literally named `/sensors` and `/control`.
