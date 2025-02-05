# IoT Weather Data Visualization with ThingsBoard

## General Project Information

This project collects weather data (temperature and humidity) from an ESP8266 IoT device and visualizes it using ThingsBoard. The system is robust due to the use of persistent volumes (PVs) that ensure data durability even in case of failures.

### Key Features:
- **Robust Data Storage**: Uses persistent volumes (PVs) for reliable data storage.
- **Real-time Data Processing**: Event-driven data flow through Minio, RabbitMQ, Node-RED, and ThingsBoard.
- **Cloud-Native Architecture**: Leveraging Kubernetes (microk8s) for scalable and manageable deployment.

### Main Technologies:
- **Kubernetes (microk8s)** for container orchestration and management.
- **kubectl** for interacting with the Kubernetes cluster.
- **Minio** for object storage and event triggering.
- **RabbitMQ** for handling messaging between services.
- **Node-RED** for workflow automation and data processing.
- **ThingsBoard** for visualizing IoT data.
- **Grafana** for monitoring system health through Prometheus metrics.
- **Keycloak** for securing and managing user access.

## 2. Installation Instructions

### Prerequisites:
- **MicroK8s**: A lightweight Kubernetes distribution, required for deploying the system.
- **kubectl**: Command-line tool for interacting with the Kubernetes cluster.

### Installation Steps:

1. **Install MicroK8s**:
   You can install MicroK8s on a Linux system using the following command:

   ```bash
   sudo snap install microk8s --classic
   ```

2. **Enable Add-ons**: Enable required add-ons like Prometheus, Grafana, etc.:

   ```bash
   microk8s enable dns storage prometheus grafana
   ```

3. **Clone the Repository**: Clone the repository containing the Kubernetes configuration files and scripts:

   ```bash
   git clone https://github.com/Niroth36/CloudPlatformESP.git
   cd CloudPlatformESP
   ```

4. **Run the Master Script**: The `start-yamls.sh` script will deploy all necessary services (Minio, RabbitMQ, Node-RED, ThingsBoard, Grafana, etc.) to your Kubernetes cluster:

   ```bash
   ./start-yamls.sh
   ```

   This script will:
   - Set up all Kubernetes resources (Deployments, Services, Persistent Volumes).
   - Deploy and configure all services needed to process and visualize the data.

## 3. Configuration

### Minio Setup:
- Create a bucket in Minio for storing IoT sensor data.
- Configure access keys (access and secret key) so the ESP8266 can send data to Minio.

### RabbitMQ Setup:
- Create an exchange to handle the events from Minio.
- Create a queue to capture the events triggered by Minio.

### Node-RED Setup:
- Create a flow in Node-RED to integrate the Minio and RabbitMQ services:
  - Capture events from RabbitMQ.
  - Retrieve data from Minio.
  - Process the data and send it to ThingsBoard.

### ThingsBoard Setup:
- Set up ThingsBoard to receive data from Node-RED via HTTP POST and visualize it in dashboards.

## 4. Usage Instructions

Once the system is set up and running, follow these steps to interact with the system:

### Send Data to Minio:
Use the script `upload_to_minio.py` to send weather data (temperature, humidity) from the ESP8266 device to Minio.

Example:

```bash
python upload_to_minio.py
```

### Check Data in Minio:
- Log into Minio's web interface (by default, available at `http://<minio-ip>:9000`) and verify that the data has been uploaded to the created bucket.

### Monitor RabbitMQ:
- Access RabbitMQ's web interface (`http://<rabbitmq-ip>:15672`) and check the queues to confirm that the events are being captured as expected.

### Check Data in Node-RED:
- Log into Node-RED's web interface (`http://<nodered-ip>:1880`) to verify that the flow is processing the events and sending the data to ThingsBoard.

### Visualize Data in ThingsBoard:
- Log into the ThingsBoard dashboard (`http://<thingsboard-ip>:8080`), and check the data visualizations (graphs, charts) for the weather data sent by the IoT device.

## 5. Test Accounts & Credentials

For default services, the following test credentials are used:

- **Minio**:
  - Username: admin
  - Password: password

- **RabbitMQ**:
  - Username: admin
  - Password: password

- **Node-RED**:
  - No authentication required by default.

- **ThingsBoard**:
  - Username: tenant@thingsboard.org
  - Password: tenant

- **Grafana**:
  - Username: admin
  - Password: password

- **Keycloak**:
  - Username: admin
  - Password: password

Make sure to change these credentials for production use or if additional security is required.

## 6. Troubleshooting & FAQs

### Common Issues:

- **Minio Bucket Not Found**:
  - Ensure that the bucket was created properly in Minio.
  - Verify the access credentials used by the ESP8266 in `upload_to_minio.py`.

- **RabbitMQ Events Not Captured**:
  - Check if the exchange and queue are properly configured in RabbitMQ.
  - Verify that Node-RED has the correct flow setup to capture and process RabbitMQ events.

- **No Data in ThingsBoard**:
  - Ensure that the Node-RED flow is correctly sending data to the ThingsBoard instance.
  - Verify that ThingsBoard has the correct device and data sources configured for visualization.

### Logs:
Logs for each service are available in the Kubernetes pods:

```bash
kubectl logs <pod-name>
```

For any other issues, please refer to the specific service documentation (Minio, RabbitMQ, Node-RED, ThingsBoard) or reach out for assistance.

## 7. Contributing

If you would like to contribute to this project, please fork the repository, create a new branch, and submit a pull request. Ensure that your code is well-documented, and all tests pass.

## 8. License

This project is licensed under the MIT License - see the LICENSE file for details.
