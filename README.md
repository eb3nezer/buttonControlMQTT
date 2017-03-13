# buttonControlMQTT
Arduino code for sending MQTT updates from pushing buttons for home automation control

This code watches for presses of 4 momentary pushbuttons. It sends updates to an MQTT broker based on "short" or "long" presses of these buttons.

The buttons should be connected so that they connect to GND when pressed.

This code can be modified to handle more buttons. For each button, define the pin, a unique bit mask, and a topic.

## configuration

These things will need to be edited in the code to match your environment.

### RED1

Set this to the digital pin that the first red button is connected to.

### RED1_TOPIC

Append this to the topic to send button press messages for the first red button.

### RED2

Set this to the digital pin that the second red button is connected to.

### RED2_TOPIC

Append this to the topic to send button press messages for the second red button.

### BLACK1

Set this to the digital pin that the first black button is connected to.

### BLACK1_TOPIC

Append this to the topic to send button press messages for the first black button.

### BLACK2

Set this to the digital pin that the second black button is connected to.

### BLACK2_TOPIC

Append this to the topic to send button press messages for the second black button.

### SHORT_PRESS

The minimum number of ms that a button must be held down for, before it is a short press. This should be greater than 0 to prevent "bounce".

If a button is released after this amount of time, but less than LONG_PRESS, a short press message is sent.

### LONG_PRESS

The minimum number of ms that a button must be held down for, before it is a long press. 

Once a button is held down for this amount of time, a long press message is sent immediately.

### mqttServer

The IP address of your MQTT broker.

### mqttUser

The user name for your MQTT broker.

### mqttPassword

The password for your MQTT broker.

### MQTT_DEVICE_NAME

The device name to identify as when connecting to MQTT.

### BUTTON_UPDATE_TOPIC_BASE

Use this as the topic prefix for button updates. The topic for each button will be appended to this to generate the final topic for the button.

If the topic base is "myhome/buttons/" and the topic for the first red button is "r1", then the final topic for this button is "myhome/buttons/r1". The payload will be "short" for a short press of this button, and "long" for a long press of this button.
