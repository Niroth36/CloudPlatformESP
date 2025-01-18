import time
import json
import random
from minio import Minio
from datetime import datetime
from io import BytesIO  # Import BytesIO to handle data as file-like object

# MinIO server connection settings
minio_client = Minio(
    "localhost:9000",
    access_key="admin",
    secret_key="password",
    secure=False  # Set to True if using HTTPS
)

# Create a bucket if it doesn't exist
bucket_name = "sensor-data"
if not minio_client.bucket_exists(bucket_name):
    minio_client.make_bucket(bucket_name)
    print(f"Bucket '{bucket_name}' created.")


# Function to generate mock sensor data
def generate_sensor_data():
    temperature = round(random.uniform(18, 28), 2)  # Simulate temperature in Celsius
    humidity = round(random.uniform(40, 70), 2)  # Simulate humidity in %
    timestamp = datetime.utcnow().isoformat() + "Z"

    return {
        "temperature": temperature,
        "humidity": humidity,
        "timestamp": timestamp
    }


# Upload simulated data to MinIO periodically
upload_interval = 10  # Upload every 10 seconds

print("Simulating ESP device and uploading data to MinIO...")
while True:
    # Generate data
    data = generate_sensor_data()

    # Save data as JSON file content
    data_json = json.dumps(data)
    filename = f"sensor_data_{datetime.utcnow().strftime('%Y%m%d%H%M%S')}.json"

    # Convert JSON data to a file-like object using BytesIO
    data_bytes = BytesIO(data_json.encode('utf-8'))

    # Upload to MinIO
    minio_client.put_object(
        bucket_name,
        filename,
        data=data_bytes,
        length=len(data_json),
        content_type="application/json"
    )

    print(f"Uploaded: {filename} -> {data}")

    # Wait before sending next data point
    time.sleep(upload_interval)
