//////////////////////////////////////////////
// rfesler@gmail.com, 2025
//
// PROJECT: MQTT Display for Vechile.
//
// DESCRIPTION:  ESP01S connecting to a MQTT Server via WiFi.
// Data being Published is incoming Solar, Shunt Load, and Altitude.
// Vechicle is utilized as a full-time living space. RPI 5
// running Rasparian, Mosquitto, and Node-Red. Display (SDD1306)
// updated via I2C.
// 
// TO-DO:
// * Publish to MQTT when device is online
// * Some type of sleep or low-power mode
// * Dynamic publication to device for user selected data
// * or, rotory encoder to select data?
// * 
// REVISIONS:
// v
// v1.00 - 02FEB2025, In-Service
// v0.40 - 04OCT2024, Scrolling, addition subscriptions
// v0.30 - 10NOV2024, WiFi/MQTT stablilty
// v0.20 - 23SEP2024, Recieving MQTT data
// v0.10 - 22SEP2024, Concept
/////////////////////////////////////////////////
