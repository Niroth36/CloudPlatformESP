#!/bin/bash

set -e  # Exit if any command fails

K8S_DIR="$(dirname "$0")"  # Set directory where script is located

echo "🚀 Applying Kubernetes configurations in order..."

apply_files() {
    local pattern=$1
    local files=$(find "$K8S_DIR" -type f -name "$pattern" 2>/dev/null)

    if [ -z "$files" ]; then
        echo "⚠️ No $pattern files found, skipping..."
    else
        echo "🔹 Applying $pattern..."
        # Fix: Pass all files with multiple `-f` flags
        kubectl apply $(echo "$files" | awk '{print "-f", $1}')
    fi
}

# Apply in order
apply_files "*-pv.yml"
apply_files "*-pvc.yml"
apply_files "*-deployment.yml"
apply_files "*-statefulset.yml"
apply_files "*-service.yml"

echo "✅ All Kubernetes resources applied successfully!"
