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

variable "registry_name" {
  description = "DigitalOcean Container Registry name"
  type        = string
  default     = "quant-registry"
}

variable "image_registry" {
  description = "Registry endpoint"
  type        = string
  default     = "registry.digitalocean.com/quant-registry"
}

variable "image_name" {
  description = "App image name"
  type        = string
  default     = "cpp-quant"
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
