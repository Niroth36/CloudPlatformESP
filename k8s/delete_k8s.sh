#!/bin/bash

set -e  # Exit if any command fails

K8S_DIR="$(dirname "$0")"

echo "🚀 Deleting Kubernetes resources..."

delete_files() {
    local pattern=$1
    local description=$2
    local files=$(find "$K8S_DIR" -type f -name "$pattern" 2>/dev/null)

    if [ -z "$files" ]; then
        echo "⚠️ No $pattern files found, skipping..."
    else
        echo "🗑️ Deleting $description..."
        microk8s kubectl delete $(echo "$files" | awk '{print "-f", $1}') --ignore-not-found
    fi
}

# Check argument
case "$1" in
    all|"")  # Default is "all" if no argument is provided
        # delete_files "*-service.yml" "Services"
        delete_files "*-ingress.yml" "Ingresses"
        delete_files "*-deployment.yml" "Deployments"
        delete_files "*-statefulset.yml" "StatefulSets"
        delete_files "*-pvc.yml" "Persistent Volume Claims"
        delete_files "*-pv.yml" "Persistent Volumes"
        ;;
    svc)
        delete_files "*-service.yml" "Services"
        ;;
    ingr)
        delete_files "*-ingress.yml" "Ingresses"
        ;;
    deploy)
        delete_files "*-deployment.yml" "Deployments"
        ;;
    statefulset)
        delete_files "*-statefulset.yml" "StatefulSets"
        ;;
    pvc)
        delete_files "*-pvc.yml" "Persistent Volume Claims"
        ;;
    pv)
        delete_files "*-pv.yml" "Persistent Volumes"
        ;;
    *)
        echo "❌ Invalid argument: $1"
        echo "Usage: $0 {all|svc|ingr|deploy|statefulset|pvc|pv}"
        exit 1
        ;;
esac

echo "✅ All requested Kubernetes resources deleted successfully!"
