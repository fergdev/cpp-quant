
variable "do_token" {
  description = "DigitalOcean API token"
  type        = string
  sensitive   = true
}

variable "cluster_name" {
  description = "Existing DOKS cluster name"
  type        = string
  default     = "quant-dev"
}

variable "namespace" {
  description = "Kubernetes namespace for cpp-quant"
  type        = string
  default     = "cppq"
}

variable "cluster_issuer" {
  description = "The name of the cert-manager ClusterIssuer to use for TLS (e.g. letsencrypt-prod)"
  type        = string
  default     = "letsencrypt-prod"
}

variable "image_registry" {
  description = "Registry endpoint"
  type        = string
  default     = "registry.digitalocean.com/quant-registry"
}

variable "image_name" {
  description = "App image name"
  type        = string
  default     = "quant_engine"
}

variable "image_tag" {
  description = "Tag to deploy (override in CI)"
  type        = string
  default     = "latest"
}

variable "ingress_host" {
  description = "Public hostname for the app (e.g., quant.yourdomain.com)"
  type        = string
}

variable "replicas" {
  description = "Deployment replicas"
  type        = number
  default     = 2
}

variable "letsencrypt_email" {
  description = "Email for ACME (cert-manager)"
  type        = string
}

variable "registry_host" {
  description = "Registry host (DigitalOcean Container Registry endpoint)"
  type        = string
  default     = "registry.digitalocean.com"
}

variable "monitoring_namespace" {
  description = "Monitoring namespace"
  type        = string
  default     = "monitoring"
}

variable "grafana_host" {
  description = "FQDN for Grafana (via Ingress)"
  type        = string
  default     = "grafana.fergdev.io"
}

variable "grafana_admin_password" {
  description = "Grafana admin password"
  type        = string
  sensitive   = true
}

