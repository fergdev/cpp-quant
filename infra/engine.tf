locals {
  image_ref = "${var.image_registry}/${var.image_name}:${var.image_tag}"
  labels    = { app = "quant-engine" }
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

          env {
            name  = "CPPQ_WS_PORT"
            value = "8080"
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

resource "kubernetes_horizontal_pod_autoscaler_v2" "app" {
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
    min_replicas = var.replicas
    max_replicas = 10
    metric {
      type = "Resource"
      resource {
        name = "cpu"
        target {
          type                = "Utilization"
          average_utilization = 60
        }
      }
    }
  }
}
