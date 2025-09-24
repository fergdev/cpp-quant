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

resource "kubernetes_service" "app" {
  metadata {
    name      = "quant-engine"
    namespace = var.namespace
    labels    = local.labels
  }
  spec {
    selector = local.labels
    port {
      name        = "http"
      port        = 80
      target_port = "http"
    }
    type = "ClusterIP"
  }
}
resource "kubernetes_ingress_v1" "app" {
  metadata {
    name      = "quant-engine"
    namespace = var.namespace
    annotations = {
      "kubernetes.io/ingress.class"                    = "nginx"
      "cert-manager.io/cluster-issuer"                 = "letsencrypt"
      "nginx.ingress.kubernetes.io/proxy-read-timeout" = "3600"
      "nginx.ingress.kubernetes.io/proxy-send-timeout" = "3600"
    }
  }

  spec {
    tls {
      hosts       = [var.ingress_host]
      secret_name = "cpp-quant-tls"
    }

    rule {
      host = var.ingress_host
      http {
        path {
          path      = "/ws"
          path_type = "Prefix"
          backend {
            service {
              name = kubernetes_service.app.metadata[0].name # backend service
              port { name = "http" }
            }
          }
        }

        path {
          path      = "/"
          path_type = "Prefix"
          backend {
            service {
              name = kubernetes_service.web_ui.metadata[0].name
              port { name = "http" }
            }
          }
        }
      }
    }
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
