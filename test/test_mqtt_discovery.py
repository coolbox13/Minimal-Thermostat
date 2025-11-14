#!/usr/bin/env python3
"""
MQTT Discovery Test Script for Home Assistant
Tests MQTT discovery messages against real broker at 192.168.178.32

Usage:
    python3 test_mqtt_discovery.py

Requirements:
    pip3 install paho-mqtt
"""

import paho.mqtt.client as mqtt
import json
import time

# MQTT Configuration
MQTT_BROKER = "192.168.178.32"
MQTT_PORT = 1883
MQTT_USERNAME = None  # Set if authentication required
MQTT_PASSWORD = None  # Set if authentication required

# Discovery Configuration
HA_DISCOVERY_PREFIX = "homeassistant"
NODE_ID = "esp32_thermostat_test5"  # Different name for testing
SW_VERSION = "1.4-test5-no-presets"

def create_origin_info():
    """Create origin information for HA discovery"""
    return {
        "name": "ESP32-KNX-Thermostat",
        "sw_version": SW_VERSION,
        "support_url": "https://github.com/yourusername/ESP32-KNX-Thermostat"
    }

def create_device_info():
    """Create device information shared across all entities - using abbreviated keys"""
    return {
        "ids": [NODE_ID],  # Abbreviated: identifiers -> ids
        "name": "ESP32 KNX Thermostat TEST5 NO PRESETS",
        "mdl": "ESP32-KNX-Thermostat",  # Abbreviated: model -> mdl
        "mf": "DIY",  # Abbreviated: manufacturer -> mf
        "sw": SW_VERSION  # Abbreviated: sw_version -> sw
    }

def create_climate_discovery():
    """
    Create climate entity discovery message using ABBREVIATED field names
    Based on senior dev's working example
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{NODE_ID}/config"

    payload = {
        "name": "Thermostat",
        "uniq_id": f"{NODE_ID}_climate",  # Abbreviated: unique_id -> uniq_id
        "dev": create_device_info(),  # Abbreviated: device -> dev

        # Mode control (abbreviated keys)
        "mode_cmd_t": f"{NODE_ID}/mode/set",  # Abbreviated: mode_command_topic
        "mode_stat_t": f"{NODE_ID}/mode/state",  # Abbreviated: mode_state_topic
        "modes": ["off", "heat"],

        # Target temperature (abbreviated keys)
        "temp_cmd_t": f"{NODE_ID}/temperature/set",  # Abbreviated: temperature_command_topic
        "temp_stat_t": f"{NODE_ID}/temperature/setpoint",  # Abbreviated: temperature_state_topic
        "curr_temp_t": f"{NODE_ID}/temperature",  # Abbreviated: current_temperature_topic

        # Temperature settings
        "min_temp": 15,
        "max_temp": 30,
        "temp_step": 0.5,  # Note: temp_step not precision
        "temp_unit": "C",  # Abbreviated: temperature_unit -> temp_unit

        # NO PRESET MODES - they cause validation errors with abbreviated keys

        # Availability (abbreviated keys)
        "avty_t": f"{NODE_ID}/status",  # Abbreviated: availability_topic
        "pl_avail": "online",  # Abbreviated: payload_available
        "pl_not_avail": "offline",  # Abbreviated: payload_not_available

        # Optional: action topic
        "act_t": f"{NODE_ID}/action",  # Abbreviated: action_topic

        "qos": 0,
        "ret": True  # Retained
    }

    return topic, payload

def create_temperature_sensor_discovery():
    """Create temperature sensor discovery"""
    topic = f"{HA_DISCOVERY_PREFIX}/sensor/{NODE_ID}/temperature/config"

    payload = {
        "name": "Temperature",
        "unique_id": f"{NODE_ID}_temperature",
        "device": create_device_info(),
        "origin": create_origin_info(),
        "state_topic": f"{NODE_ID}/temperature",
        "unit_of_measurement": "°C",
        "device_class": "temperature",
        "state_class": "measurement",
        "availability_topic": f"{NODE_ID}/status"
    }

    return topic, payload

def create_humidity_sensor_discovery():
    """Create humidity sensor discovery"""
    topic = f"{HA_DISCOVERY_PREFIX}/sensor/{NODE_ID}/humidity/config"

    payload = {
        "name": "Humidity",
        "unique_id": f"{NODE_ID}_humidity",
        "device": create_device_info(),
        "origin": create_origin_info(),
        "state_topic": f"{NODE_ID}/humidity",
        "unit_of_measurement": "%",
        "device_class": "humidity",
        "state_class": "measurement",
        "availability_topic": f"{NODE_ID}/status"
    }

    return topic, payload

def create_valve_position_sensor_discovery():
    """Create valve position sensor discovery"""
    topic = f"{HA_DISCOVERY_PREFIX}/sensor/{NODE_ID}/valve_position/config"

    payload = {
        "name": "Valve Position",
        "unique_id": f"{NODE_ID}_valve_position",
        "device": create_device_info(),
        "origin": create_origin_info(),
        "state_topic": f"{NODE_ID}/valve/position",
        "unit_of_measurement": "%",
        "icon": "mdi:valve",
        "state_class": "measurement",
        "availability_topic": f"{NODE_ID}/status"
    }

    return topic, payload

def publish_initial_states(client):
    """Publish initial state values to populate the climate entity"""
    print("\n=== Publishing Initial States ===")

    states = [
        (f"{NODE_ID}/mode/state", "heat"),
        (f"{NODE_ID}/temperature/setpoint", "22.0"),
        (f"{NODE_ID}/preset/state", "none"),
        (f"{NODE_ID}/action", "idle"),
        (f"{NODE_ID}/status", "online"),
    ]

    for topic, value in states:
        result = client.publish(topic, value, qos=1, retain=True)
        status = "✓" if result.rc == 0 else "✗"
        print(f"{status} {topic} = {value}")
        time.sleep(0.1)

    print("=== Initial States Published ===\n")

def on_connect(client, userdata, flags, rc):
    """Callback when connected to MQTT broker"""
    if rc == 0:
        print(f"✓ Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"✗ Connection failed with code {rc}")

def on_publish(client, userdata, mid):
    """Callback when message is published"""
    pass  # Silent on publish

def test_discovery_messages():
    """Test all discovery messages"""
    print("=" * 60)
    print("MQTT Discovery Test Script for Home Assistant")
    print("=" * 60)
    print(f"Broker: {MQTT_BROKER}:{MQTT_PORT}")
    print(f"Node ID: {NODE_ID}")
    print(f"Version: {SW_VERSION}")
    print("=" * 60)

    # Create MQTT client
    client = mqtt.Client(client_id=f"test_discovery_{int(time.time())}")
    client.on_connect = on_connect
    client.on_publish = on_publish

    if MQTT_USERNAME and MQTT_PASSWORD:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    try:
        # Connect to broker
        print(f"\nConnecting to {MQTT_BROKER}:{MQTT_PORT}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        time.sleep(1)

        # Publish initial states BEFORE discovery
        publish_initial_states(client)

        # Test discovery messages
        print("\n=== Testing Discovery Messages ===\n")

        discoveries = [
            ("Climate Entity", create_climate_discovery),
            ("Temperature Sensor", create_temperature_sensor_discovery),
            ("Humidity Sensor", create_humidity_sensor_discovery),
            ("Valve Position Sensor", create_valve_position_sensor_discovery),
        ]

        for name, create_func in discoveries:
            topic, payload = create_func()
            payload_json = json.dumps(payload, indent=2)
            payload_size = len(payload_json)

            print(f"--- {name} ---")
            print(f"Topic: {topic}")
            print(f"Payload size: {payload_size} bytes")

            if payload_size > 1024:
                print(f"⚠️  WARNING: Payload exceeds 1024 bytes (needs 1536+ byte buffer)")

            # Publish discovery message
            result = client.publish(topic, payload_json, qos=1, retain=True)

            if result.rc == 0:
                print(f"✓ Published successfully")
            else:
                print(f"✗ Publish failed with code {result.rc}")

            print(f"\nPayload preview:")
            print(payload_json[:500])
            if len(payload_json) > 500:
                print(f"... ({len(payload_json) - 500} more bytes)")
            print()

            time.sleep(0.5)

        print("\n=== All Discovery Messages Published ===")
        print("\nNext steps:")
        print("1. Check Home Assistant → Settings → Devices & Services → MQTT")
        print("2. Look for 'ESP32 KNX Thermostat' device")
        print("3. Verify climate entity appears with preset mode support")
        print("4. Check that all sensors are discovered")

        # Keep connection alive for a bit
        print("\nKeeping connection alive for 5 seconds...")
        time.sleep(5)

        client.loop_stop()
        client.disconnect()

        print("\n✓ Test completed successfully")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_discovery_messages()
