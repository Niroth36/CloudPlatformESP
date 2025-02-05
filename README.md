# IoT Weather Data Visualization with ThingsBoard

## General Project Information
This project collects weather data (temperature and humidity) from an ESP8266 IoT device and visualizes it using ThingsBoard. The system is robust due to the use of persistent volumes (PVs) that ensure data durability even in case of failures.

### Key Features:
- **Robust Data Storage:** Uses persistent volumes (PVs) for reliable data storage.
- **Real-time Data Processing:** Event-driven data flow through Minio, RabbitMQ, Node-RED, and ThingsBoard.
- **Cloud-Native Architecture:** Leveraging Kubernetes (microk8s) for scalable and manageable deployment.

### Main Technologies:
- **Kubernetes (microk8s)** for container orchestration and management.
- **kubectl** for interacting with the Kubernetes cluster.
- **Minio** for object storage and event triggering.
- **RabbitMQ** for handling messaging between services.
- **Node-RED** for workflow automation and data processing.
- **ThingsBoard** for visualizing IoT data.
- **Grafana** for monitoring system health through Prometheus metrics.
- **Keycloak** for securing and managing user access.

## Installation Instructions

### Prerequisites:
- **MicroK8s:** A lightweight Kubernetes distribution, required for deploying the system.
- **kubectl:** Command-line tool for interacting with the Kubernetes cluster.

### Installation Steps:
#### Install MicroK8s:
```bash
sudo snap install microk8s --classic
```
#### Enable Add-ons:
Enable required add-ons like Prometheus, Grafana, etc.:
```bash
microk8s enable dns storage prometheus grafana
```
#### Clone the Repository:
```bash
git clone https://github.com/Niroth36/CloudPlatformESP.git
cd CloudPlatformESP
```
#### Run the Master Script:
The `start-yamls.sh` script will deploy all necessary services (Minio, RabbitMQ, Node-RED, ThingsBoard, Grafana, etc.) to your Kubernetes cluster:
```bash
./start-yamls.sh
```
This script will:
- Set up all Kubernetes resources (Deployments, Services, Persistent Volumes).
- Deploy and configure all services needed to process and visualize the data.

## Configuration

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

### Grafana Setup:
#### Update `grafana-deployment.yml`:
```bash
kubectl apply -f grafana-deployment.yml
kubectl delete pod -n cloud-platform -l app=grafana
```
#### Update `grafana-service.yml`:
```bash
kubectl apply -f grafana-service.yml
```

## Keycloak Configuration
### Create the `cloud-platform` Realm in Keycloak
1. Open Keycloak:
   - URL: [http://192.168.1.95:30008](http://192.168.1.95:30008)
   - Login as Admin:
     - **Username:** admin
     - **Password:** adminpassword
2. Create a New Realm:
   - Click the dropdown menu (top-left).
   - Click **Create Realm**.
   - **Realm Name:** cloud-platform
   - Click **Create**.

### Create the `grafana-client` for OAuth
1. Go to **Clients** → Click **Create Client**.
   - **Client ID:** grafana-client
   - **Client Type:** OpenID Connect.
2. Click **Next**.
3. Enable OAuth2 Client Authentication:
   - **Client Authentication:** ON (Grafana needs a client secret).
   - **Standard Flow:** ✅ Enabled (Required for login via OAuth2).
   - **Direct Access Grants:** ❌ Disabled.
   - Click **Save**.

### Configure the Client for Grafana
1. Open the **grafana-client** settings.
   - **Root URL:** `http://192.168.1.95:30000`
   - **Valid Redirect URIs:** `http://192.168.1.95:30000/login/generic_oauth`
   - **Web Origins:** `http://192.168.1.95:30000`
2. Click **Save**.

### Get the Client Secret
1. Go to the **Credentials** tab.
2. Copy the **Client Secret**.

### Update Grafana with the New Keycloak Details
```bash
kubectl apply -f grafana-deployment.yml
kubectl delete pod -n cloud-platform -l app=grafana
```
**Grafana is accessible at:** [http://192.168.1.95:30000](http://192.168.1.95:30000)

## Usage Instructions

### Send Data to Minio:
```bash
python upload_to_minio.py
```

### Check Data in Minio:
- Access Minio's web interface: `http://<minio-ip>:9000`.

### Monitor RabbitMQ:
- Access RabbitMQ: `http://<rabbitmq-ip>:15672`.

### Check Data in Node-RED:
- Access Node-RED: `http://<nodered-ip>:1880`.

### Visualize Data in ThingsBoard:
- Access ThingsBoard: `http://<thingsboard-ip>:8080`.

## Port Forwarding
```bash
kubectl port-forward -n cloud-platform svc/grafana 3000:80
```

## Test Accounts & Credentials
| Service      | Username  | Password  |
|-------------|----------|----------|
| Minio       | admin    | password |
| RabbitMQ    | admin    | password |
| Node-RED    | No auth  | N/A      |
| ThingsBoard | tenant@thingsboard.org | tenant |
| Grafana     | admin    | password |
| Keycloak    | admin    | password |

## Contributing
Fork the repository, create a new branch, and submit a pull request.

## License
This project is licensed under the MIT License.

