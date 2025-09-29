locals {
  image_ref = "${var.image_registry}/${var.image_name}:${var.image_tag}"
  labels    = { app = "quant-engine" }
}

locals {
  engine_env = [
    { name = "ENGINE_LOG_LEVEL", value = "info" },
    { name = "ENGINE_LOG_FORMAT", value = "json" },
  ]
}
resource "kubernetes_deployment" "app" {
  metadata {
    name      = "quant-engine"
    namespace = var.namespace
    labels    = local.labels
  }

  spec {
    replicas = var.replicas
    selector { match_labels = local.labels }
    template {
      metadata { labels = local.labels }
      spec {
        image_pull_secrets { name = kubernetes_secret.docr_pull.metadata[0].name }
        container {
          name  = "app"
          image = local.image_ref

          port {
            name           = "http"
            container_port = 8080
          }
          dynamic "env" {
            for_each = concat(local.engine_env, [
              { name = "CPPQ_WS_PORT", value = "8080" },
              { name = "OTEL_SERVICE_NAME", value = "quant-engine" },
              { name = "OTEL_TRACES_EXPORTER", value = "otlp" },
              { name = "OTEL_EXPORTER_OTLP_ENDPOINT", value = "http://otelcol.monitoring.svc.cluster.local:4317" },
              { name = "OTEL_RESOURCE_ATTRIBUTES", value = "deployment.environment=dev" },
            ])
            content {
              name  = env.value.name
              value = env.value.value
            }
          }
          resources {
            requests = {
              cpu    = "250m"
              memory = "256Mi"
            }
            limits = {
              cpu    = "1"
              memory = "512Mi"
            }
          }

          readiness_probe {
            tcp_socket {
              port = "http"
            }
            initial_delay_seconds = 2
          }

          liveness_probe {
            tcp_socket {
              port = "http"
            }
            initial_delay_seconds = 10
          }
        }
      }
    }
  }
}

resource "kubernetes_service" "quant_engine" {
  metadata {
    name      = "quant-engine"
    namespace = var.namespace
    labels    = { app = "quant-engine" }
  }
  spec {
    selector = { app = "quant-engine" }
    port {
      name        = "http"
      port        = 80
      target_port = "http"
    }
    type = "ClusterIP"
  }
}

# resource "kubernetes_horizontal_pod_autoscaler_v2" "app" {
#   metadata {
#     name      = "quant-engine"
#     namespace = var.namespace
#   }
#   spec {
#     scale_target_ref {
#       api_version = "apps/v1"
#       kind        = "Deployment"
#       name        = kubernetes_deployment.app.metadata[0].name
#     }
#     min_replicas = var.replicas
#     max_replicas = 10
#     metric {
#       type = "Resource"
#       resource {
#         name = "cpu"
#         target {
#           type                = "Utilization"
#           average_utilization = 60
#         }
#       }
#     }
#   }
# }
variable "enable_hpa" {
  type    = bool
  default = false
}
variable "hpa_min_replicas" {
  type    = number
  default = 1
}
variable "hpa_max_replicas" {
  type    = number
  default = 3
}
variable "hpa_target_cpu" {
  type    = number
  default = 60
}

resource "kubernetes_horizontal_pod_autoscaler_v2" "app" {
  count = var.enable_hpa ? 1 : 0

  metadata {
    name      = "quant-engine"
    namespace = var.namespace
  }
  spec {
    scale_target_ref {
      api_version = "apps/v1"
      kind        = "Deployment"
      name        = kubernetes_deployment.app.metadata[0].name
    }
    min_replicas = var.hpa_min_replicas
    max_replicas = var.hpa_max_replicas
    metric {
      type = "Resource"
      resource {
        name = "cpu"
        target {
          type                = "Utilization"
          average_utilization = var.hpa_target_cpu
        }
      }
    }
    behavior {
      scale_up {
        stabilization_window_seconds = 60
        policy {
          type           = "Percent"
          value          = 100
          period_seconds = 60
        }
      }
      scale_down {
        stabilization_window_seconds = 300
        policy {
          type           = "Percent"
          value          = 50
          period_seconds = 60
        }
      }
    }
  }
}
