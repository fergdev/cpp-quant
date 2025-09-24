resource "kubernetes_ingress_v1" "quant" {
  metadata {
    name      = "quant-ingress"
    namespace = var.namespace
    annotations = {
      "kubernetes.io/ingress.class"                    = "nginx"
      "cert-manager.io/cluster-issuer"                 = var.cluster_issuer
      "nginx.ingress.kubernetes.io/proxy-read-timeout" = "3600"
      "nginx.ingress.kubernetes.io/proxy-send-timeout" = "3600"
      "nginx.ingress.kubernetes.io/force-ssl-redirect" = "true"
    }
  }

  spec {
    tls {
      hosts       = [var.ingress_host]
      secret_name = "quant-cert"
    }

    rule {
      host = var.ingress_host
      http {
        path {
          path      = "/ws"
          path_type = "Prefix"
          backend {
            service {
              name = kubernetes_service.quant_engine.metadata[0].name
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

