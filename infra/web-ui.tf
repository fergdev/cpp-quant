resource "kubernetes_deployment" "web_ui" {
  metadata {
    name      = "web-ui"
    namespace = var.namespace
    labels    = { app = "web-ui" }
  }

  spec {
    replicas = 1

    selector {
      match_labels = { app = "web-ui" }
    }

    template {
      metadata {
        labels = { app = "web-ui" }
      }

      spec {
        container {
          name  = "web-ui"
          image = "${var.image_registry}/cpp-quant:ui-${var.image_tag}"

          port {
            name           = "http"
            container_port = 80
          }

          resources {
            requests = { cpu = "100m", memory = "128Mi" }
            limits   = { cpu = "500m", memory = "256Mi" }
          }
        }

        image_pull_secrets {
          name = kubernetes_secret.docr_pull.metadata[0].name
        }
      }
    }
  }
}

resource "kubernetes_service" "web_ui" {
  metadata {
    name      = "web-ui"
    namespace = var.namespace
    labels    = { app = "web-ui" }
  }

  spec {
    selector = { app = "web-ui" }

    port {
      name        = "http"
      port        = 80
      target_port = "http"
    }

    type = "ClusterIP"
  }
}
