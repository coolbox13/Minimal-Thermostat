#!/usr/bin/env python3
"""
MQTT Discovery Test Script - Minimal Working Example
Based on verified working Home Assistant climate discovery example

Tests against real broker at 192.168.178.32

Usage:
    python3 test_mqtt_discovery_minimal.py

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
NODE_ID = "test_thermostat_minimal"
SW_VERSION = "1.0-minimal"

def create_minimal_climate_discovery():
    """
    Minimal climate discovery that MUST work
    Based on verified working example
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{NODE_ID}/config"

    payload = {
        "name": "Test Thermostat Minimal",
        "uniq_id": f"{NODE_ID}_001",
        "cmd_t": f"test/thermostat/set",
        "temp_cmd_t": f"test/thermostat/target_temperature/set",
        "temp_stat_t": f"test/thermostat/target_temperature",
        "curr_temp_t": f"test/thermostat/current_temperature",
        "modes": ["off", "heat"],
        "mode_cmd_t": f"test/thermostat/mode/set",
        "mode_stat_t": f"test/thermostat/mode",
        "min_temp": 15,
        "max_temp": 25,
        "temp_unit": "C"
    }

    return topic, payload

def create_full_climate_discovery():
    """
    Full featured climate discovery with all options
    Based on verified working example
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/livingroom_thermostat/config"

    payload = {
        "name": "Living Room Thermostat",
        "uniq_id": "livingroom_thermostat_001",
        "dev": {
            "ids": ["livingroom_thermostat_device"],
            "name": "Living Room HVAC",
            "mf": "DIY",
            "mdl": "ESP32-Thermo-2025",
            "sw": "1.0.0"
        },
        "cmd_t": "home/livingroom/thermostat/set",
        "mode_cmd_t": "home/livingroom/thermostat/mode/set",
        "mode_stat_t": "home/livingroom/thermostat/mode",
        "temp_cmd_t": "home/livingroom/thermostat/target_temperature/set",
        "temp_stat_t": "home/livingroom/thermostat/target_temperature",
        "curr_temp_t": "home/livingroom/thermostat/current_temperature",
        "avty_t": "home/livingroom/thermostat/availability",
        "pl_avail": "online",
        "pl_not_avail": "offline",

        "min_temp": 15,
        "max_temp": 25,
        "temp_step": 0.5,
        "temp_unit": "C",

        "modes": ["off", "heat", "cool", "auto"],
        "preset_modes": ["eco", "comfort", "boost"],

        "preset_mode_cmd_t": "home/livingroom/thermostat/preset/set",
        "preset_mode_stat_t": "home/livingroom/thermostat/preset",

        "fan_modes": ["auto", "low", "medium", "high"],
        "fan_mode_cmd_t": "home/livingroom/thermostat/fan_mode/set",
        "fan_mode_stat_t": "home/livingroom/thermostat/fan_mode",

        "qos": 0,
        "ret": True
    }

    return topic, payload

def publish_minimal_states(client):
    """Publish minimal state values"""
    print("\n=== Publishing Minimal States ===")

    states = [
        ("test/thermostat/mode", "heat"),
        ("test/thermostat/target_temperature", "22.0"),
        ("test/thermostat/current_temperature", "21.3"),
    ]

    for topic, value in states:
        result = client.publish(topic, value, qos=0, retain=True)
        status = "✓" if result.rc == 0 else "✗"
        print(f"{status} {topic} = {value}")
        time.sleep(0.1)

    print("=== Minimal States Published ===\n")

def publish_full_states(client):
    """Publish full state values"""
    print("\n=== Publishing Full States ===")

    states = [
        ("home/livingroom/thermostat/current_temperature", "21.3"),
        ("home/livingroom/thermostat/target_temperature", "22.0"),
        ("home/livingroom/thermostat/mode", "heat"),
        ("home/livingroom/thermostat/preset", "comfort"),
        ("home/livingroom/thermostat/fan_mode", "medium"),
        ("home/livingroom/thermostat/availability", "online"),
    ]

    for topic, value in states:
        result = client.publish(topic, value, qos=0, retain=True)
        status = "✓" if result.rc == 0 else "✗"
        print(f"{status} {topic} = {value}")
        time.sleep(0.1)

    print("=== Full States Published ===\n")

def on_connect(client, userdata, flags, rc):
    """Callback when connected to MQTT broker"""
    if rc == 0:
        print(f"✓ Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"✗ Connection failed with code {rc}")

def test_discovery():
    """Test discovery messages"""
    print("=" * 70)
    print("MQTT Climate Discovery Test - Verified Working Examples")
    print("=" * 70)
    print(f"Broker: {MQTT_BROKER}:{MQTT_PORT}")
    print("=" * 70)

    # Create MQTT client
    client = mqtt.Client(client_id=f"test_discovery_{int(time.time())}")
    client.on_connect = on_connect

    if MQTT_USERNAME and MQTT_PASSWORD:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    try:
        # Connect to broker
        print(f"\nConnecting to {MQTT_BROKER}:{MQTT_PORT}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        time.sleep(1)

        # Test 1: Minimal Climate Discovery (MUST WORK)
        print("\n" + "=" * 70)
        print("TEST 1: MINIMAL CLIMATE ENTITY (bare minimum)")
        print("=" * 70)

        publish_minimal_states(client)

        topic, payload = create_minimal_climate_discovery()
        payload_json = json.dumps(payload, indent=2)

        print(f"Topic: {topic}")
        print(f"Payload size: {len(payload_json)} bytes")
        print(f"\nPayload:\n{payload_json}\n")

        result = client.publish(topic, payload_json, qos=0, retain=True)
        if result.rc == 0:
            print("✓ Minimal climate discovery published successfully")
        else:
            print(f"✗ Publish failed with code {result.rc}")

        time.sleep(2)

        # Test 2: Full Featured Climate Discovery
        print("\n" + "=" * 70)
        print("TEST 2: FULL FEATURED CLIMATE ENTITY")
        print("=" * 70)

        publish_full_states(client)

        topic, payload = create_full_climate_discovery()
        payload_json = json.dumps(payload, indent=2)

        print(f"Topic: {topic}")
        print(f"Payload size: {len(payload_json)} bytes")
        print(f"\nPayload:\n{payload_json}\n")

        result = client.publish(topic, payload_json, qos=0, retain=True)
        if result.rc == 0:
            print("✓ Full climate discovery published successfully")
        else:
            print(f"✗ Publish failed with code {result.rc}")

        print("\n" + "=" * 70)
        print("VERIFICATION STEPS:")
        print("=" * 70)
        print("1. Check Home Assistant → Settings → Devices & Services → MQTT")
        print("2. Should see TWO new climate entities:")
        print("   a) 'Test Thermostat Minimal' (basic off/heat)")
        print("   b) 'Living Room Thermostat' (full featured)")
        print("3. Try controlling both entities in HA")
        print("4. If minimal works but full doesn't, there's a config issue")
        print("5. If neither works, MQTT discovery might be disabled in HA")
        print("=" * 70)

        # Keep connection alive
        print("\nKeeping connection alive for 10 seconds...")
        time.sleep(10)

        client.loop_stop()
        client.disconnect()

        print("\n✓ Test completed successfully")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_discovery()
