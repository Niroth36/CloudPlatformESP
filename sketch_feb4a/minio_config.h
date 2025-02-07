#ifndef MINIO_CONFIG_H
#define MINIO_CONFIG_H

// WiFi credentials
const char* ssid = "karpouzi";
const char* password = "123456789";

// MinIO Service URL (Kubernetes ClusterIP or NodePort)
#define MINIO_SERVICE "http://192.168.207.104:30090"
#define MINIO_BUCKET "sensor-data"  // Bucket name

// MinIO Access Credentials
#define MINIO_ACCESS_KEY "FTSG3tLVGXolqcmaj10A"
#define MINIO_SECRET_KEY "okorm0U5AXTRFbLi3zkLWhNyM5D9ZTItZCjdr8TM"

#endif
