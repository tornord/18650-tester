#!/usr/bin/env python3
"""
Test script for the Label Generation API
"""

import requests
import json

def test_label_api():
    """Test the label generation API"""
    
    # API endpoint
    url = "http://localhost:5001/"
    
    # Test data
    test_data = {
        "capacity": 2.6,
        "internalResistance": 0.09
    }
    
    print("Testing Label Generation API...")
    print(f"URL: {url}")
    print(f"Data: {json.dumps(test_data, indent=2)}")
    print("-" * 50)
    
    try:
        # Make POST request
        response = requests.post(
            url,
            json=test_data,
            headers={'Content-Type': 'application/json'},
            timeout=30
        )
        
        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        
        if response.status_code == 200:
            print("✅ API test successful!")
        else:
            print("❌ API test failed!")
            
    except requests.exceptions.ConnectionError:
        print("❌ Connection failed! Make sure the API server is running.")
        print("Start the server with: python label_api.py")
    except Exception as e:
        print(f"❌ Error: {e}")

def test_health_endpoint():
    """Test the health check endpoint"""
    
    url = "http://localhost:5001/health"
    
    print("\nTesting health endpoint...")
    print(f"URL: {url}")
    
    try:
        response = requests.get(url, timeout=5)
        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        
        if response.status_code == 200:
            print("✅ Health check successful!")
        else:
            print("❌ Health check failed!")
            
    except requests.exceptions.ConnectionError:
        print("❌ Connection failed! Make sure the API server is running.")
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    print("Label Generation API Test")
    print("=" * 50)
    
    # Test health endpoint first
    test_health_endpoint()
    
    # Test main API endpoint
    test_label_api()
    
    print("\n" + "=" * 50)
    print("Test completed!")
