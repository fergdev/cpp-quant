output "ingress_hostname" {
  value       = var.ingress_host
  description = "DNS name serving the app over HTTPS"
}

output "image_ref" {
  value       = local.image_ref
  description = "Deployed image reference"
}