#!/usr/bin/env python3
"""
MQTT Discovery Test Script - Current ESP32 Implementation
Tests exact copy of current ESP32 discovery + variations with presets

Usage:
    python3 test/test_mqtt_discovery_current.py

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
NODE_ID_BASE = "esp32_thermostat_test"
SW_VERSION = "1.4"

def create_device_info(node_id):
    """Create device information - EXACT match of ESP32 code"""
    return {
        "ids": [node_id],
        "name": "ESP32 KNX Thermostat",
        "mf": "DIY",
        "mdl": "ESP32-KNX-Thermostat",
        "sw": SW_VERSION
    }

def test_current_esp32_exact(node_id):
    """
    EXACT copy of current ESP32 implementation (no presets)
    This is what's currently in src/home_assistant.cpp
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{node_id}/config"

    device_info = create_device_info(node_id)

    payload = {
        "name": "Thermostat EXACT",
        "uniq_id": f"{node_id}_climate",
        "dev": device_info,
        # Mode control (abbreviated)
        "mode_cmd_t": f"{node_id}/mode/set",
        "mode_stat_t": f"{node_id}/mode/state",
        "modes": ["off", "heat"],
        # Temperature control (abbreviated)
        "temp_cmd_t": f"{node_id}/temperature/set",
        "temp_stat_t": f"{node_id}/temperature/setpoint",
        "curr_temp_t": f"{node_id}/temperature",
        "min_temp": 15,
        "max_temp": 30,
        "temp_step": 0.5,
        "temp_unit": "C",
        # Availability (abbreviated)
        "avty_t": f"{node_id}/status",
        "pl_avail": "online",
        "pl_not_avail": "offline",
        # Action (abbreviated)
        "act_t": f"{node_id}/action",
        "qos": 0,
        "ret": True
    }

    return topic, payload, "Current ESP32 (exact, no presets)"

def test_with_presets_abbreviated(node_id):
    """
    Add preset modes WITH abbreviated keys
    Let's see what HA complains about
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{node_id}/config"

    device_info = create_device_info(node_id)

    payload = {
        "name": "Thermostat PRESET_ABBREV",
        "uniq_id": f"{node_id}_climate",
        "dev": device_info,
        "mode_cmd_t": f"{node_id}/mode/set",
        "mode_stat_t": f"{node_id}/mode/state",
        "modes": ["off", "heat"],
        "temp_cmd_t": f"{node_id}/temperature/set",
        "temp_stat_t": f"{node_id}/temperature/setpoint",
        "curr_temp_t": f"{node_id}/temperature",
        "min_temp": 15,
        "max_temp": 30,
        "temp_step": 0.5,
        "temp_unit": "C",
        # PRESETS WITH ABBREVIATED KEYS
        "pr_mode_cmd_t": f"{node_id}/preset/set",  # Abbreviated: preset_mode_command_topic
        "pr_mode_stat_t": f"{node_id}/preset/state",  # Abbreviated: preset_mode_state_topic
        "pr_modes": ["eco", "comfort", "away", "sleep", "boost"],  # Abbreviated: preset_modes
        # Availability
        "avty_t": f"{node_id}/status",
        "pl_avail": "online",
        "pl_not_avail": "offline",
        "act_t": f"{node_id}/action",
        "qos": 0,
        "ret": True
    }

    return topic, payload, "With presets (abbreviated keys)"

def test_with_presets_full_keys(node_id):
    """
    Add preset modes WITH FULL keys (not abbreviated)
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{node_id}/config"

    device_info = create_device_info(node_id)

    payload = {
        "name": "Thermostat PRESET_FULL",
        "uniq_id": f"{node_id}_climate",
        "dev": device_info,
        # FULL KEYS for mode
        "mode_command_topic": f"{node_id}/mode/set",
        "mode_state_topic": f"{node_id}/mode/state",
        "modes": ["off", "heat"],
        # FULL KEYS for temperature
        "temperature_command_topic": f"{node_id}/temperature/set",
        "temperature_state_topic": f"{node_id}/temperature/setpoint",
        "current_temperature_topic": f"{node_id}/temperature",
        "min_temp": 15,
        "max_temp": 30,
        "temp_step": 0.5,
        "temperature_unit": "C",
        # FULL KEYS for presets
        "preset_mode_command_topic": f"{node_id}/preset/set",
        "preset_mode_state_topic": f"{node_id}/preset/state",
        "preset_modes": ["eco", "comfort", "away", "sleep", "boost"],
        # FULL KEYS for availability
        "availability_topic": f"{node_id}/status",
        "payload_available": "online",
        "payload_not_available": "offline",
        "action_topic": f"{node_id}/action",
        "qos": 0,
        "retain": True
    }

    return topic, payload, "With presets (full keys)"

def test_with_presets_mixed(node_id):
    """
    Mix abbreviated keys for basic + full keys for presets
    """
    topic = f"{HA_DISCOVERY_PREFIX}/climate/{node_id}/config"

    device_info = create_device_info(node_id)

    payload = {
        "name": "Thermostat PRESET_MIXED",
        "uniq_id": f"{node_id}_climate",
        "dev": device_info,
        # ABBREVIATED for basic fields
        "mode_cmd_t": f"{node_id}/mode/set",
        "mode_stat_t": f"{node_id}/mode/state",
        "modes": ["off", "heat"],
        "temp_cmd_t": f"{node_id}/temperature/set",
        "temp_stat_t": f"{node_id}/temperature/setpoint",
        "curr_temp_t": f"{node_id}/temperature",
        "min_temp": 15,
        "max_temp": 30,
        "temp_step": 0.5,
        "temp_unit": "C",
        # FULL KEYS for presets
        "preset_mode_command_topic": f"{node_id}/preset/set",
        "preset_mode_state_topic": f"{node_id}/preset/state",
        "preset_modes": ["eco", "comfort", "away", "sleep", "boost"],
        # ABBREVIATED for availability
        "avty_t": f"{node_id}/status",
        "pl_avail": "online",
        "pl_not_avail": "offline",
        "act_t": f"{node_id}/action",
        "qos": 0,
        "ret": True
    }

    return topic, payload, "With presets (mixed keys)"

def publish_initial_states(client, node_id):
    """Publish initial state values to populate the climate entity"""
    print(f"\n=== Publishing Initial States for {node_id} ===")

    states = [
        (f"{node_id}/mode/state", "heat"),
        (f"{node_id}/temperature/setpoint", "22.0"),
        (f"{node_id}/temperature", "20.0"),
        (f"{node_id}/preset/state", "comfort"),
        (f"{node_id}/action", "idle"),
        (f"{node_id}/status", "online"),
    ]

    for topic, value in states:
        result = client.publish(topic, value, qos=1, retain=True)
        status = "✓" if result.rc == 0 else "✗"
        print(f"{status} {topic} = {value}")
        time.sleep(0.05)

    print("=== Initial States Published ===\n")

def on_connect(client, userdata, flags, rc):
    """Callback when connected to MQTT broker"""
    if rc == 0:
        print(f"✓ Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"✗ Connection failed with code {rc}")

def test_all_variations():
    """Test all variations"""
    print("=" * 80)
    print("MQTT Discovery Test - Current ESP32 Implementation + Preset Variations")
    print("=" * 80)
    print(f"Broker: {MQTT_BROKER}:{MQTT_PORT}")
    print(f"Version: {SW_VERSION}")
    print("=" * 80)

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

        # Test variations with numbered IDs
        tests = [
            (f"{NODE_ID_BASE}1", test_current_esp32_exact),
            (f"{NODE_ID_BASE}2", test_with_presets_abbreviated),
            (f"{NODE_ID_BASE}3", test_with_presets_full_keys),
            (f"{NODE_ID_BASE}4", test_with_presets_mixed),
        ]

        for node_id, test_func in tests:
            print(f"\n{'='*80}")
            topic, payload, description = test_func(node_id)
            payload_json = json.dumps(payload, indent=2)
            payload_size = len(payload_json)

            print(f"TEST: {description}")
            print(f"Node ID: {node_id}")
            print(f"Topic: {topic}")
            print(f"Payload size: {payload_size} bytes")

            if payload_size > 1536:
                print(f"⚠️  WARNING: Payload exceeds 1536 bytes")

            # Publish initial states BEFORE discovery
            publish_initial_states(client, node_id)

            # Publish discovery message
            result = client.publish(topic, payload_json, qos=1, retain=True)

            if result.rc == 0:
                print(f"✓ Discovery message published successfully")
            else:
                print(f"✗ Publish failed with code {result.rc}")

            print(f"\nPayload preview:")
            print(payload_json[:400])
            if len(payload_json) > 400:
                print(f"... ({len(payload_json) - 400} more bytes)")

            time.sleep(1)

        print("\n" + "="*80)
        print("All test variations published!")
        print("="*80)
        print("\n📋 Next steps:")
        print("1. Enable HA MQTT debug logging:")
        print("   logger:")
        print("     logs:")
        print("       homeassistant.components.mqtt: debug")
        print("2. Restart Home Assistant")
        print("3. Check Settings → Devices & Services → MQTT")
        print("4. Check HA logs for validation errors")
        print("5. Report which test variation works!")

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
    test_all_variations()
