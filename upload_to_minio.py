import re
from minio import Minio
from minio.error import S3Error
import json

# Path to the minio_config.h file
CONFIG_FILE = "minio_config.h"

# Function to parse the minio_config.h file
def parse_minio_config(file_path):
    config = {}
    try:
        with open(file_path, "r") as f:
            for line in f:
                # Match lines like: #define NAME "value"
                match = re.match(r'#define\s+(\w+)\s+"([^"]+)"', line)
                if match:
                    key, value = match.groups()
                    config[key] = value
    except FileNotFoundError:
        print(f"Configuration file {file_path} not found.")
        exit(1)
    return config

# Parse the MinIO configuration
config = parse_minio_config(CONFIG_FILE)

# Extract the required values
MINIO_SERVER = config.get("MINIO_SERVER")
MINIO_BUCKET = config.get("MINIO_BUCKET")
MINIO_ACCESS_KEY = config.get("MINIO_ACCESS_KEY")
MINIO_SECRET_KEY = config.get("MINIO_SECRET_KEY")

# Validate configuration
if not all([MINIO_SERVER, MINIO_BUCKET, MINIO_ACCESS_KEY, MINIO_SECRET_KEY]):
    print("Error: Missing configuration values in minio_config.h.")
    exit(1)

# Create a MinIO client
client = Minio(
    MINIO_SERVER,
    access_key=MINIO_ACCESS_KEY,
    secret_key=MINIO_SECRET_KEY,
    secure=False  # Set to True if using HTTPS
)

# Create bucket if it doesn't exist
if not client.bucket_exists(MINIO_BUCKET):
    client.make_bucket(MINIO_BUCKET)
    print(f"Bucket '{MINIO_BUCKET}' created successfully.")
else:
    print(f"Bucket '{MINIO_BUCKET}' already exists.")

# Mock JSON data
mock_data = {
    "temperature": 22.5,
    "humidity": 55.0,
    "timestamp": "2025-01-12T14:30:00Z"
}

# Save mock data to a file
FILE_NAME = "sensor_data.json"
with open(FILE_NAME, "w") as json_file:
    json.dump(mock_data, json_file)
print(f"Mock JSON data saved to {FILE_NAME}")

# Upload JSON file to MinIO
try:
    client.fput_object(
        MINIO_BUCKET,
        FILE_NAME,  # Object name in MinIO
        FILE_NAME   # Local file path
    )
    print(f"File '{FILE_NAME}' uploaded successfully to bucket '{MINIO_BUCKET}'.")
except S3Error as e:
    print("Error occurred:", e)